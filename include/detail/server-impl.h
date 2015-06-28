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

#ifndef INCLUDE_DETAIL_SERVER_IMPL_H_
#define INCLUDE_DETAIL_SERVER_IMPL_H_

#include <functional>
#include <string>
#include <vector>

#include "detail/dispatcher.h"
#include "entity.h"
#include "parameter.h"
#include "server.h"

namespace topper {
namespace detail {

typedef std::function<Response(std::vector<std::string> const&, UriInfo const& uriInfo)> Method;
struct Methods {
    Method get;
    Method put;
    Method post;
    Method del;
};

void doRegister(ServerImpl *server, Resource *resource, Methods const& methods);

template<typename R>
detail::Methods bindMethods(R *r) {
    // Bind calls for each method
    return {
        [=](std::vector<std::string> const& params, UriInfo const& uriInfo) {
            return detail::ResourceDispatcher::dispatch(r, &R::get, params,
                uriInfo);
        },
        [=](std::vector<std::string> const& params, UriInfo const& uriInfo) {
            return detail::ResourceDispatcher::dispatch(r, &R::put, params,
                uriInfo);
        },
        [=](std::vector<std::string> const& params, UriInfo const& uriInfo) {
            return detail::ResourceDispatcher::dispatch(r, &R::post, params,
                uriInfo);
        },
        [=](std::vector<std::string> const& params, UriInfo const& uriInfo) {
            return detail::ResourceDispatcher::dispatch(r, &R::del, params,
                uriInfo);
        },
    };
}

} // detail namespace

template<typename R>
void Server::registerResource(R *r) {
    // Bind functions for each
    doRegister(internal_, r, detail::bindMethods(r));
}

} // topper namespace

#endif // INCLUDE_DETAIL_SERVER_IMPL_H_
