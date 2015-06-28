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

#ifndef INCLUDE_RESOURCE_H_
#define INCLUDE_RESOURCE_H_

#include <string>

#include "entity.h"
#include "parameter.h"
#include "response.h"

namespace topper {

/**
 * A default implementation of a REST API resource.
 *
 * Implementors should override one or more of the HTTP request methods,
 * defining a set of parameters compatible with the resource's path
 * template.
 */
class Resource {
public:
    Response get() const {
        return Response::notAllowed();
    }

    Response put() const {
        return Response::notAllowed();
    }

    Response post() const {
        return Response::notAllowed();
    }

    Response del() const {
        return Response::notAllowed();
    }

    ~Resource() { }

    /** @return the path template for the resource. */
    std::string const& path() const { return path_; }
protected:
    explicit Resource(std::string const& path) : path_(path) { }
    const std::string path_;
};

} // topper namespace

#endif // INCLUDE_RESOURCE_H_
