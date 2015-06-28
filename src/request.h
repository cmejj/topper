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

#ifndef SRC_RESPONSE_H_
#define SRC_RESPONSE_H_

#include <string>
#include <unordered_map>

#include "parameter.h"
#include "parameter_internal.h"

namespace topper {

enum class HttpMethod {
    GET,
    PUT,
    POST,
    DELETE,
};

// Immutable
class Request {
public:
    Request(std::string const& path, std::string const& body,
            std::unordered_map<std::string, std::string> const& headers,
            HttpMethod type,
            std::unordered_multimap<std::string, std::string> &&queryParams,
            std::unordered_multimap<std::string, std::string> &&postParams)
        : path_(path), headers_(headers), type_(type),
          data_({QueryParamsImpl(std::move(queryParams)),
            PostParamsImpl(std::move(postParams)), Entity(body)}),
          uriInfo_({data_.queryParams, data_.postParams, data_.entity})
    { }

    // Returns the request URI path
    std::string const& path() const { return path_; }

    // Returns the value of the named header, or null
    const std::string* headerValue(std::string const& name) const;

    HttpMethod type() const { return type_; }

    UriInfo const& uriInfo() const { return uriInfo_; }
private:
    const std::string path_;
    const std::unordered_map<std::string, std::string> headers_;
    const HttpMethod type_;

    struct {
        QueryParamsImpl queryParams;
        PostParamsImpl postParams;
        Entity entity;
    } data_;

    const UriInfo uriInfo_;
};

} // topper namespace

#endif // SRC_RESPONSE_H_
