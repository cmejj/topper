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

#include <sstream>
#include <string>

#include "response.h"

namespace topper {

namespace {

int codeToInt(HttpCode code) {
    return static_cast<int>(code);
}

std::string mediaTypeToString(MediaType type) {
    switch (type) {
    case MediaType::APPLICATION_JSON:
        return "application/json";
    case MediaType::TEXT_PLAIN:
        return "text/plain";
    case MediaType::NONE:
        // Punt to default
        return "application/octet-stream";
    }
}

std::string reasonPhrase(HttpCode code) {
    switch (code) {
    case HttpCode::OK:
        return "OK";
    case HttpCode::CREATED:
        return "Created";
    case HttpCode::FORBIDDEN:
        return "Forbidden";
    case HttpCode::NOT_FOUND:
        return "Not Found";
    case HttpCode::NOT_ALLOWED:
        return "Method Not Allowed";
    case HttpCode::INTERNAL_ERROR:
        return "Internal Server Error";
    }
}

} // anonymous namespace

Response::Response(HttpCode code) : code_(code), type_(MediaType::TEXT_PLAIN)
    { }

Response::Response(HttpCode code, MediaType type, std::string const& content)
        : code_(code),
          type_(type),
          content_(content) { }

std::string Response::to_string() const {
    static const std::string kCrlf("\r\n");
    static const std::string kVersion("HTTP/1.1");
    std::stringstream response;

    // Status line
    response << kVersion << " " << codeToInt(code_) << " "
             << reasonPhrase(code_) << kCrlf;

    // Headers
    response << "Content-Length: " << content_.size() << kCrlf
             << "Connection: close" << kCrlf
             << "Content-Type: " << mediaTypeToString(type_) << kCrlf << kCrlf;

    // Body
    response << content_;

    return response.str();
}

Response Response::notAllowed() {
    return Response(HttpCode::NOT_ALLOWED);
}

Response Response::notFound() {
    return Response(HttpCode::NOT_FOUND);
}

} // topper namespace
