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

#include "server_instance.h"

#include <atomic>

namespace topper {

void ServerInstance::stop() {
    if (!listener_) {
        return;
    }

    listener_->stopAccepting();

    delete listener_;
    listener_ = nullptr;
}

void ServerInstance::start(wte::EventBase *listener_base,
        std::vector<wte::EventBase*> const& handlers) {
    if (listener_) {
        throw std::logic_error("Server has already been started");
    }

    bases_ = handlers;
    listener_ = wte::mkConnectionListener(listener_base,
        std::bind(&ServerInstance::acceptCb, this, std::placeholders::_1),
        std::bind(&ServerInstance::listenErrorCb, this, std::placeholders::_1));

    listener_->bind(ipaddr_, port_);
    listener_->listen(128);
    listener_->startAccepting();

    // Ok we're off
    printf("Started server on %s port %hu\n", ipaddr_.c_str(),
        listener_->port());
    printf("The following resource paths are registered:\n\n");
    for (Resource *resource : matcher().resources()) {
        printf("    %s\n", resource->path().c_str());
    }
    printf("\n");
}

void ServerInstance::listenErrorCb(std::exception const& e) {
    LOG(INFO) << e.what();
}

wte::EventBase* ServerInstance::chooseBase() {
    // Simply round-robin for the moment
    static std::atomic<int> iter(0);
    return bases_[++iter % bases_.size()];
}

void ServerInstance::acceptCb(int fd) {
    wte::EventBase *base = chooseBase();

    // Released on error or completion
    auto *ctx = new RequestContext(this, base, fd);

    // Queue asynchronous reading
    base->runOnEventLoop([ctx]() { ctx->stream->startRead(&ctx->rcb); });
}

void ServerInstance::WriteCallback::complete(wte::Stream *s) {
    DCHECK(ctx_->stream == s); // XXX this parameter is apparently silly
    // TODO: support keepalives
    delete ctx_;
}

void ServerInstance::WriteCallback::error(std::runtime_error const& e) {
    LOG(INFO) << "While writing: " << e.what();
    delete ctx_;
}

void ServerInstance::ReadCallback::error(std::runtime_error const& e) {
    LOG(INFO) << "While reading: " << e.what();
    delete ctx_;
}

void ServerInstance::ReadCallback::eof() {
    int rc = http_parser_execute(&ctx_->parser, &ctx_->settings, nullptr, 0);
    if (rc != 0) {
        LOG(INFO) << "Error on eof";
        delete ctx_;
    }
}

void ServerInstance::ReadCallback::available(wte::Buffer *buffer) {
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

} // topper namespace
