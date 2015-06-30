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

#include "ccmetrics/detail/define_once.h"
#include "ccmetrics/metric_registry.h"
#include "ccmetrics/timer.h"

#include "wte/event_base.h"

#include "logging.h"
#include "metrics_resource.h"
#include "server.h"
#include "server_instance.h"

namespace topper {

namespace {
bool validateAddr(std::string const& ipaddr) {
    struct in_addr tmp;
    return inet_aton(ipaddr.c_str(), &tmp) == 1;
}

class PingResource : public Resource {
public:
    PingResource() : Resource("/ping") { }
    Response get() const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN, "pong\n");
    }
};

struct AdminServer {
    AdminServer(std::string const& ipaddr, short port,
                ccmetrics::MetricRegistry *metrics)
            : server(ipaddr, port, metrics),
              server_metrics(metrics) {
        server.registerResource(&ping, detail::bindMethods(&ping));
        server.registerResource(&server_metrics,
            detail::bindMethods(&server_metrics));
    }
    ServerInstance server;
    PingResource ping;
    MetricsResource server_metrics;
};
} // unnamed namespace

class ServerImpl {
public:
    ServerImpl(std::string const& ip_addr, short port)
        : listener_base_(wte::mkEventBase()),
          application_(ip_addr, port, &metrics_) { }

    ~ServerImpl() {
        if (started_) {
            stopAndWait();
        }
        delete listener_base_;
    }

    void start();
    void stopAndWait();
    void wait();
    void startAdminServer(std::string const& ipaddr, short port);

    void registerResource(Resource *resource, detail::Methods const& methods) {
        application_.registerResource(resource, methods);
    }
private:
    bool started_ = false;
    bool shutdown_ = false;
    wte::EventBase *listener_base_ = nullptr;

    std::vector<wte::EventBase*> bases_;
    std::vector<std::thread*> base_threads_;
    // TODO: worker pool for compute-intensive or blocking requests

    std::thread main_;

    // Metrics
    ccmetrics::MetricRegistry metrics_;

    ServerInstance application_;
    AdminServer *admin_server_ = nullptr;
};

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

    // Bring up application server
    application_.start(listener_base_, bases_);

    // Ok we're off
    started_ = true;

    main_ = std::thread([this]() {
            listener_base_->loop(wte::EventBase::LoopMode::FOREVER);
        });
}

void ServerImpl::startAdminServer(std::string const& ipaddr, short port) {
    if (admin_server_) {
        throw std::logic_error("Admin server has already been started");
    }

    admin_server_ = new AdminServer(ipaddr, port, &metrics_);
    listener_base_->runOnEventLoop([this]() -> void {
            admin_server_->server.start(listener_base_, bases_);
        });
}

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

    listener_base_->runOnEventLoopAndWait([this]() -> void {
            application_.stop();
            if (admin_server_) {
                admin_server_->server.stop();
                delete admin_server_;
                admin_server_ = nullptr;
            }
        });

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

void Server::startAdminServer(std::string const& ipaddr, short port) {
    internal_->startAdminServer(ipaddr, port);
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
