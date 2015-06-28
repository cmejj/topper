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

#ifndef INCLUDE_RESPONSE_H_
#define INCLUDE_RESPONSE_H_

#include <string>

namespace topper {

// A partial list of HTTP response codes
enum class HttpCode {
    OK              = 200,
    CREATED         = 201,
    FORBIDDEN       = 403,
    NOT_FOUND       = 404,
    NOT_ALLOWED     = 405,
    INTERNAL_ERROR  = 500,
};

enum class MediaType {
    NONE,
    APPLICATION_JSON,
    TEXT_PLAIN,
};

class Response {
public:
    explicit Response(HttpCode code);
    Response(HttpCode code, MediaType type, std::string const& content);

    /**
     * Constructs a full HTTP/1.1 response suitable for transmission.
     *
     * @return the response as a string
     */
    std::string to_string() const;

    /** @return the response code. */
    HttpCode code() const { return code_; }

    /** @return a 405 response. */
    static Response notAllowed();

    /** @return a 404 response. */
    static Response notFound();
private:
    HttpCode code_;
    MediaType type_;
    std::string content_;
};

} // topper namespace

#endif // INCLUDE_RESPONSE_H_
