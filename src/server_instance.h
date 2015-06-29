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

#ifndef SRC_SERVER_INSTANCE_H_
#define SRC_SERVER_INSTANCE_H_

#include <functional>
#include <string>
#include <thread>

#include "wte/connection_listener.h"
#include "wte/event_base.h"
#include "wte/event_handler.h"
#include "wte/stream.h"

#include "http_parser.h"
#include "resource.h"
#include "resource_matcher.h"
#include "response.h"
#include "request.h"
#include "request_builder.h"

namespace topper {

class ServerInstance {
public:
    ServerInstance(std::string const& ipaddr, short port)
        : ipaddr_(ipaddr), port_(port) { }

    ~ServerInstance() {
        delete listener_;
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
        RequestContext(ServerInstance *server, wte::EventBase *base, int sock)
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
        ServerInstance *server;
        wte::Stream *stream;

        WriteCallback wcb;
        ReadCallback rcb;
    };

    void start(wte::EventBase *listener_base,
        std::vector<wte::EventBase*> const& handlers); // throws
    void stop();

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
    wte::ConnectionListener *listener_ = nullptr;

    // Request handlers. These may be shared.
    std::vector<wte::EventBase*> bases_;

    // Resources
    ResourceMatcher matcher_;
};

} // topper namespace

#endif // SRC_SERVER_INSTANCE_H_
