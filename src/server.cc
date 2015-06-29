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

#include "logging.h"
#include "server.h"
#include "server_impl.h"

namespace topper {

namespace {
bool validateAddr(std::string const& ipaddr) {
    struct in_addr tmp;
    return inet_aton(ipaddr.c_str(), &tmp) == 1;
}
} // unnamed namespace

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
