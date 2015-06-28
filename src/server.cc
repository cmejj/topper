/*
 * Copyright © 2015 Nathan Rosenblum <flander@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <arpa/inet.h>
#include <string.h>

#include <exception>
#include <string>
#include <thread>
#include <unordered_map>

#include "wte/connection_listener.h"
#include "wte/event_base.h"
#include "wte/event_handler.h"
#include "wte/stream.h"

#include "http_parser.h"
#include "logging.h"
#include "query_string.h"
#include "resource.h"
#include "resource_matcher.h"
#include "response.h"
#include "request.h"
#include "server.h"

namespace topper {

namespace {

std::unordered_multimap<std::string, std::string> parseQueryParameters(
        const char *s, size_t len) {
    std::unordered_multimap<std::string, std::string> ret;
    QueryString qs(std::string(s, len));
    for (auto param : qs) {
        ret.insert(param);
    }
    return ret;
}

// State for consuming HTTP requests from http-parser and constructing
// the immutable Request object.
class RequestBuilder {
public:
    // Casting helper
    static RequestBuilder* builder(void *data);

    static int on_url(http_parser *parser, const char *at, size_t length) {
        RequestBuilder *b = builder(parser->data);
        b->url_.append(at, length);
        return 0;
    }

    static int on_header_field(http_parser *parser, const char *at,
            size_t length) {
        RequestBuilder *b = builder(parser->data);
        switch (b->hstate_) {
        case HeaderState::INIT:
            // Receiving the first header
            b->hname_.append(at, length);
            break;
        case HeaderState::VALUE:
            // New header received; save existing header
            VLOG(3) << "Header " << b->hname_ << " = " << b->hvalue_;
            b->headers_[b->hname_] = b->hvalue_;
            // Reset buffers
            b->hname_.clear();
            b->hvalue_.clear();
            // Save new name
            b->hname_.append(at, length);
            break;
        case HeaderState::FIELD:
            // Continuing existing name
            b->hname_.append(at, length);
            break;
        }

        b->hstate_ = HeaderState::FIELD;

        return 0;
    }

    static int on_header_value(http_parser *parser, const char *at,
            size_t length) {
        RequestBuilder *b = builder(parser->data);
        switch (b->hstate_) {
        case HeaderState::INIT:
            return 1;
        case HeaderState::VALUE:
        case HeaderState::FIELD:
            b->hvalue_.append(at, length);
        }

        b->hstate_ = HeaderState::VALUE;

        return 0;
    }

    static int on_body(http_parser *parser, const char *at, size_t length) {
        RequestBuilder *b = builder(parser->data);
        b->body_.append(at, length);
        return 0;
    }

    static HttpMethod convertMethod(int method) {
        switch(method) {
        case 0:
            return HttpMethod::DELETE;
        case 1:
            return HttpMethod::GET;
        case 3:
            return HttpMethod::POST;
        case 4:
            return HttpMethod::PUT;
        default:
            throw std::runtime_error("Invalid method");
        }
    }

    // Construct a request object (throws)
    Request build(int method) const {
        struct http_parser_url parser_url;
        // XXX return code checking?
        int rc = http_parser_parse_url(url_.c_str(), url_.size(),
            /*isconnect=*/ 0, &parser_url);
        if (rc != 0) {
            throw std::runtime_error("Error parsing url");
        }

        std::string path {"/"}; // Default to root
        if (parser_url.field_set & (1 << UF_PATH)) {
            path = std::string(&url_[parser_url.field_data[UF_PATH].off],
                parser_url.field_data[UF_PATH].len);
        }

        std::unordered_multimap<std::string, std::string> queryParams;
        if (parser_url.field_set & (1 << UF_QUERY)) {
            queryParams = parseQueryParameters(
                &url_[parser_url.field_data[UF_QUERY].off],
                parser_url.field_data[UF_QUERY].len);
        }

        std::unordered_multimap<std::string, std::string> postParams;
        if (convertMethod(method) == HttpMethod::POST) {
            // Same same; assuming application/x-www-form-urlencoded.
            // TODO: support multipart
            postParams = parseQueryParameters(body_.c_str(), body_.size());
        }

        return Request(path, body_, headers_, convertMethod(method),
            std::move(queryParams), std::move(postParams));
    }
private:
    // State for parsing headers. See documentation at
    // https://github.com/joyent/http-parser for discussion.
    enum class HeaderState {
        INIT,       // First call
        VALUE,      // Reading value
        FIELD,      // Reading vield
    };

    // State for parsing headers
    HeaderState hstate_ = HeaderState::INIT;
    std::string hname_; // Buffer for header name
    std::string hvalue_; // Buffer for header value

    std::string url_; // Buffer for url parsing
    std::string body_; // Buffer for body parsing

    // Completed headers
    std::unordered_map<std::string, std::string> headers_;
};

} // unnamed namespace

class ServerImpl {
public:
    ServerImpl(std::string const& ipaddr, short port)
        : ipaddr_(ipaddr), port_(port), listener_base_(wte::mkEventBase()),
          listener_(wte::mkConnectionListener(listener_base_,
            std::bind(&ServerImpl::acceptCb, this, std::placeholders::_1),
            std::bind(&ServerImpl::listenErrorCb, this,
                std::placeholders::_1))) { }

    ~ServerImpl() {
        if (started_) {
            stopAndWait();
        }
    }

    struct RequestContext;
    class WriteCallback final : public wte::Stream::WriteCallback {
    public:
        explicit WriteCallback(RequestContext *ctx) : ctx_(ctx) { }
        void complete(wte::Stream *stream) override;
        void error(std::runtime_error const& e) override;
    private:
        RequestContext *ctx_ = nullptr;
    };

    class ReadCallback final : public wte::Stream::ReadCallback {
    public:
        explicit ReadCallback(RequestContext *ctx) : ctx_(ctx) { }
        void available(wte::Buffer *buffer) override;
        void eof() override;
        void error(std::runtime_error const& e) override;
    private:
        RequestContext *ctx_ = nullptr;
    };

    // Context used for receiving a request
    struct RequestContext {
        RequestContext(ServerImpl *server, wte::EventBase *base, int sock)
                : server(server), wcb(this), rcb(this) {
            // TODO: dispatch to one of N bases
            stream = wte::wrapFd(base, sock);

            // http-parser config
            memset(&settings, 0, sizeof(http_parser_settings));
            settings.on_url = RequestBuilder::on_url;
            settings.on_header_field = RequestBuilder::on_header_field;
            settings.on_header_value = RequestBuilder::on_header_value;
            settings.on_body = RequestBuilder::on_body;
            settings.on_message_complete = message_complete;
            http_parser_init(&parser, HTTP_REQUEST);
            parser.data = this;
        }

        ~RequestContext() {
            stream->stopRead();
            stream->close();
            delete stream;
        }

        http_parser parser;
        http_parser_settings settings;
        RequestBuilder builder;
        ServerImpl *server;
        wte::Stream *stream;

        WriteCallback wcb;
        ReadCallback rcb;
    };

    void start(); // throws
    void stopAndWait(); // throws
    void wait();

    void registerResource(Resource *resource, detail::Methods const& methods) {
        matcher_.addResource(resource, methods);
    }

    ResourceMatcher const& matcher() const {
        return matcher_;
    }
private:
    static Response dispatch(Request const& req, Match const& handler) {
        switch (req.type()) {
        case HttpMethod::GET:
            return handler.methods.get(handler.parameters, req.uriInfo());
        case HttpMethod::PUT:
            return handler.methods.put(handler.parameters, req.uriInfo());
        case HttpMethod::POST:
            return handler.methods.post(handler.parameters, req.uriInfo());
        case HttpMethod::DELETE:
            return handler.methods.del(handler.parameters, req.uriInfo());
        }
    }

    // Choose a base for the request
    wte::EventBase* chooseBase();

    void acceptCb(int fd);
    void listenErrorCb(std::exception const& e);

    static Response get_response(http_parser *parser, RequestContext *ctx) {
        try {
            // Build the request object
            Request req = ctx->builder.build(parser->method);

            // Find a resouce that matches this requests's path
            auto match = ctx->server->matcher_.match(req.path());

            // Dispatch to the resource or return 404
            return match ? dispatch(req, match.get()) : Response::notFound();
        } catch (std::exception const& e) {
            return Response(HttpCode::INTERNAL_ERROR, MediaType::TEXT_PLAIN,
                e.what());
        }
    }

    static int message_complete(http_parser *parser) {
        auto ctx = reinterpret_cast<RequestContext*>(parser->data);

        // TODO: move this off of the event loop
        Response resp = get_response(parser, ctx);

        // XXX uhg.
        std::string resp_str = resp.to_string();
        ctx->stream->write(resp_str.c_str(), resp_str.size(), &ctx->wcb);
        return 0;
    }

    // Configuration
    const std::string ipaddr_;
    const short port_;

    // Runtime state
    bool started_ = false;
    bool shutdown_ = false;
    wte::EventBase *listener_base_ = nullptr;
    wte::ConnectionListener *listener_ = nullptr;

    // Request handlers
    std::vector<wte::EventBase*> bases_;
    std::vector<std::thread*> base_threads_;
    // TODO: worker pool for compute-intensive or blocking requests

    std::thread main_;

    // Resources
    ResourceMatcher matcher_;
};

RequestBuilder* RequestBuilder::builder(void *data) {
    auto ctx = reinterpret_cast<ServerImpl::RequestContext*>(data);
    return &ctx->builder;
}

namespace {
bool validateAddr(std::string const& ipaddr) {
    struct in_addr tmp;
    return inet_aton(ipaddr.c_str(), &tmp) == 1;
}
} // unnamed namespace

//
// ServerImpl impl
//

void ServerImpl::wait() {
    if (!started_) {
        return;
    }
    main_.join();
}

void ServerImpl::stopAndWait() {
    if (!started_) {
        throw std::logic_error("Server was not started");
    }

    if (shutdown_) {
        // Already shut down
        return;
    }

    listener_->stopAccepting();
    listener_base_->stop();

    for (wte::EventBase *base : bases_) {
        base->stop();
        delete base;
    }
    bases_.clear();

    for (std::thread *thread : base_threads_) {
        thread->join();
        delete thread;
    }
    base_threads_.clear();

    main_.join();
    shutdown_ = true;
}

void ServerImpl::start() {
    if (started_) {
        throw std::logic_error("Server has already been started");
    }

    // Bring up the worker bases
    const int kBases = 4;
    for (int i = 0; i < kBases; ++i) {
        wte::EventBase *base = wte::mkEventBase();
        std::thread *base_thread = new std::thread([base]() {
                base->loop(wte::EventBase::LoopMode::FOREVER);
            });
        bases_.push_back(base);
        base_threads_.push_back(base_thread);
    }

    listener_->bind(ipaddr_, port_);
    listener_->listen(128);
    listener_->startAccepting();

    // Ok we're off
    started_ = true;

    printf("Started server on %s port %hu\n", ipaddr_.c_str(),
        listener_->port());
    printf("The following resource paths are registered:\n\n");
    for (Resource *resource : matcher().resources()) {
        printf("    %s\n", resource->path().c_str());
    }
    printf("\n");

    main_ = std::thread([this]() {
            listener_base_->loop(wte::EventBase::LoopMode::FOREVER);
        });
}

void ServerImpl::listenErrorCb(std::exception const& e) {
    LOG(INFO) << e.what();
}

wte::EventBase* ServerImpl::chooseBase() {
    // Simply round-robin for the moment
    static std::atomic<int> iter(0);
    return bases_[++iter % bases_.size()];
}

void ServerImpl::acceptCb(int fd) {
    wte::EventBase *base = chooseBase();

    // Released on error or completion
    auto *ctx = new RequestContext(this, base, fd);

    // Queue asynchronous reading
    base->runOnEventLoop([ctx]() { ctx->stream->startRead(&ctx->rcb); });
}

void ServerImpl::WriteCallback::complete(wte::Stream *s) {
    DCHECK(ctx_->stream == s); // XXX this parameter is apparently silly
    // TODO: support keepalives
    delete ctx_;
}

void ServerImpl::WriteCallback::error(std::runtime_error const& e) {
    LOG(INFO) << "While writing: " << e.what();
    delete ctx_;
}

void ServerImpl::ReadCallback::error(std::runtime_error const& e) {
    LOG(INFO) << "While reading: " << e.what();
    delete ctx_;
}

void ServerImpl::ReadCallback::eof() {
    int rc = http_parser_execute(&ctx_->parser, &ctx_->settings, nullptr, 0);
    if (rc != 0) {
        LOG(INFO) << "Error on eof";
        delete ctx_;
    }
}

void ServerImpl::ReadCallback::available(wte::Buffer *buffer) {
    std::vector<wte::Extent> extents;
    size_t drain = 0;
    buffer->peek(-1, &extents);
    for (auto& extent : extents) {
        size_t parsed = http_parser_execute(&ctx_->parser, &ctx_->settings,
            extent.data, extent.size);
        if (parsed != extent.size) {
            LOG(INFO) << "Parsed " << parsed << " bytes of " << extent.size;
            delete ctx_;
            return;
        }
        drain += parsed;
    }
    buffer->drain(drain);
}

//
// Server impl
//

Server::Server(std::string const& ipaddr, short port) : internal_(nullptr) {
    if (!validateAddr(ipaddr)) {
        throw std::invalid_argument("Invalid address " + ipaddr);
    }
    internal_ = new ServerImpl(ipaddr, port);
}

Server::~Server() {
    delete internal_;
}

void Server::start() {
    internal_->start();
}

void Server::stopAndWait() {
    internal_->stopAndWait();
}

void Server::wait() {
    internal_->wait();
}

//
// Registration helper
//

namespace detail {

void doRegister(ServerImpl *server, Resource *resource,
        Methods const& methods) {
    server->registerResource(resource, methods);
}

} // detail namespace

} // topper namespace
