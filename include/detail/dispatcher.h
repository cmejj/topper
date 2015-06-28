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

#ifndef INCLUDE_DETAIL_DISPATCHER_H_
#define INCLUDE_DETAIL_DISPATCHER_H_

#include <string>

#include "detail/invoker.h"
#include "detail/tuple_util.h"
#include "parameter.h"
#include "resource.h"
#include "response.h"

namespace topper {
namespace detail {

class ResourceDispatcher {
public:
    template<typename... Args>
    static Response dispatch(Response (*res)(Args...),
            std::vector<std::string> const& params, UriInfo const& uriInfo) {
        return invoke(res, Extractor<sizeof...(Args),
               typename MRR<Args>::type...>::extract(params, uriInfo));
    }

    template<typename ResType, typename... Args>
    static Response dispatch(const Resource *res,
            Response (ResType::*method)(Args...) const,
            std::vector<std::string> const& params, UriInfo const& uriInfo) {
        return invoke_member(res, method, Extractor<sizeof...(Args),
            typename MRR<Args>::type...>::extract(params, uriInfo));
    }

    // No instantiation
    ResourceDispatcher() = delete;
};

} // detail namespace
} // topper namespace

#endif // INCLUDE_DETAIL_DISPATCHER_H_
