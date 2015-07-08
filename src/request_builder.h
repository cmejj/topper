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

#ifndef SRC_REQUEST_BUILDER_H_
#define SRC_REQUEST_BUILDER_H_

#include <string>
#include <unordered_map>

#include "http_parser.h"
#include "logging.h"
#include "request.h"
#include "query_string.h"

namespace topper {

// State for consuming HTTP requests from http-parser and constructing
// the immutable Request object.
class RequestBuilder {
public:
    // Casting helper
    static RequestBuilder* builder(void *data);

    static int on_url(http_parser *parser, const char *at, size_t length) {
        RequestBuilder *b = builder(parser->data);
        b->url_.append(at, length);
        return 0;
    }

    static int on_header_field(http_parser *parser, const char *at,
            size_t length) {
        RequestBuilder *b = builder(parser->data);
        switch (b->hstate_) {
        case HeaderState::INIT:
            // Receiving the first header
            b->hname_.append(at, length);
            break;
        case HeaderState::VALUE:
            // New header received; save existing header
            VLOG(3) << "Header " << b->hname_ << " = " << b->hvalue_;
            b->headers_[b->hname_] = b->hvalue_;
            // Reset buffers
            b->hname_.clear();
            b->hvalue_.clear();
            // Save new name
            b->hname_.append(at, length);
            break;
        case HeaderState::FIELD:
            // Continuing existing name
            b->hname_.append(at, length);
            break;
        }

        b->hstate_ = HeaderState::FIELD;

        return 0;
    }

    static int on_header_value(http_parser *parser, const char *at,
            size_t length) {
        RequestBuilder *b = builder(parser->data);
        switch (b->hstate_) {
        case HeaderState::INIT:
            return 1;
        case HeaderState::VALUE:
        case HeaderState::FIELD:
            b->hvalue_.append(at, length);
        }

        b->hstate_ = HeaderState::VALUE;

        return 0;
    }

    static int on_body(http_parser *parser, const char *at, size_t length) {
        RequestBuilder *b = builder(parser->data);
        b->body_.append(at, length);
        return 0;
    }

    static HttpMethod convertMethod(int method) {
        switch(method) {
        case 0:
            return HttpMethod::DELETE;
        case 1:
            return HttpMethod::GET;
        case 3:
            return HttpMethod::POST;
        case 4:
            return HttpMethod::PUT;
        default:
            throw std::runtime_error("Invalid method");
        }
    }
    static std::unordered_multimap<std::string, std::string>
    parseQueryParameters(const char *s, size_t len) {
        std::unordered_multimap<std::string, std::string> ret;
        QueryString qs(std::string(s, len));
        for (auto param : qs) {
            ret.insert(param);
        }
        return ret;
    }


    // Construct a request object (throws)
    Request build(int method) const {
        struct http_parser_url parser_url;
        // XXX return code checking?
        int rc = http_parser_parse_url(url_.c_str(), url_.size(),
            /*isconnect=*/ 0, &parser_url);
        if (rc != 0) {
            throw std::runtime_error("Error parsing url");
        }

        std::string path {"/"}; // Default to root
        if (parser_url.field_set & (1 << UF_PATH)) {
            path = std::string(&url_[parser_url.field_data[UF_PATH].off],
                parser_url.field_data[UF_PATH].len);
        }

        std::unordered_multimap<std::string, std::string> queryParams;
        if (parser_url.field_set & (1 << UF_QUERY)) {
            queryParams = parseQueryParameters(
                &url_[parser_url.field_data[UF_QUERY].off],
                parser_url.field_data[UF_QUERY].len);
        }

        std::unordered_multimap<std::string, std::string> postParams;
        if (convertMethod(method) == HttpMethod::POST) {
            // Same same; assuming application/x-www-form-urlencoded.
            // TODO: support multipart
            postParams = parseQueryParameters(body_.c_str(), body_.size());
        }

        // Sigh. Fix this.
        auto copy = headers_;
        return Request(path, body_, convertMethod(method),
            std::move(queryParams), std::move(postParams), std::move(copy));
    }
private:
    // State for parsing headers. See documentation at
    // https://github.com/joyent/http-parser for discussion.
    enum class HeaderState {
        INIT,       // First call
        VALUE,      // Reading value
        FIELD,      // Reading vield
    };

    // State for parsing headers
    HeaderState hstate_ = HeaderState::INIT;
    std::string hname_; // Buffer for header name
    std::string hvalue_; // Buffer for header value

    std::string url_; // Buffer for url parsing
    std::string body_; // Buffer for body parsing

    // Completed headers
    std::unordered_map<std::string, std::string> headers_;
};

} // topper namespace

#endif // SRC_REQUEST_BUILDER_H_
