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

#include "server_impl.h"

namespace topper {

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

} // topper namespace
