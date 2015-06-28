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

#ifndef INCLUDE_PARAMETER_H_
#define INCLUDE_PARAMETER_H_

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "entity.h"

namespace topper {

class StringParam {
public:
    /**
     * Parses a path parameter string into a string type.
     *
     * @param[in] input the path parameter
     * @return the input parameter
     */
    static StringParam parse(std::string const& input);

    /** @return the string value. */
    std::string const& value() const { return value_; }
private:
    explicit StringParam(std::string const& v) : value_(v) { }
    std::string value_;
};

template<typename T>
class IntParam {
public:
    static_assert(std::is_integral<T>::value, "Integral type required");

    /**
     * Parses a path parameter string to an integral type.
     *
     * @param[in] input the path parameter
     * @return the integral type
     * @throws on invalid (non-integral) strings
     */
    static IntParam parse(std::string const& input) {
        size_t pos = 0;
        T val = static_cast<T>(std::stoll(input, &pos));
        if (pos != input.size()) {
            throw std::invalid_argument("Invalid integral string");
        }
        return IntParam(val);
    }

    /** @return the integral value. */
    T value() const { return value_; }
private:
    explicit IntParam(T v) : value_(v) { }
    IntParam() = delete;
    T value_;
};

class PostParams {
public:
    virtual ~PostParams() { }

    /**
     * Returns the value of post parameters matching the input.
     *
     * @param[in] name the parameter naem
     * @return 0 or more matching values
     */
    // TODO: this vector return value is not ABI-safe
    virtual std::vector<std::string> get(std::string const& name) const = 0;
    virtual bool operator==(PostParams const&) const = 0;
protected:
    PostParams() { }
};

class QueryParams {
public:
    virtual ~QueryParams() { }

    /**
     * Returns the values of query parameters matching the input.
     *
     * @param[in] name the parameter name
     * @return 0 or more matching values
     */
    // TODO: this vector return value is not ABI-safe
    virtual std::vector<std::string> get(std::string const& name) const = 0;
    virtual bool operator==(QueryParams const&) const = 0;
protected:
    QueryParams() { }
};

struct UriInfo {
    QueryParams const& queryParams;
    PostParams const& postParams;
    Entity const& entity;
};

} // topper namespace

#endif // INCLUDE_PARAMETER_H_
