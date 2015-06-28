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

#include <string>

#include <boost/iterator/iterator_facade.hpp>

#ifndef SRC_QUERY_STRING_H_
#define SRC_QUERY_STRING_H_

namespace topper {

/**
 * Helper to parse a query string into individual components.
 *
 * Iterating over this collection returns the tuples `<key,value>` for
 * each of the query parameters of the form `key1=value1&key2=valu2&...`. If
 * no value is present, the value component of the tuple will be the empty
 * string.
 *
 * This parser supports '&' and ';' as separator characters.
 *
 * TODO: convert to using c strings directly.
 */
class QueryString {
public:
    explicit QueryString(std::string const& query) : query_(query) { }
    class Iterator;
    Iterator begin() const;
    Iterator end() const;
private:
    std::string query_;
};

class QueryString::Iterator :
        public boost::iterator_facade<QueryString::Iterator,
            const std::pair<std::string, std::string>,
            boost::forward_traversal_tag> {
public:
    Iterator(std::string const& query, size_t p, size_t n) : query_(query),
        psep_(p), nsep_(n) { }

    void increment() {
        if (psep_ == std::string::npos && nsep_ != std::string::npos) {
            // First round
            psep_ = 0;
        } else if (nsep_ != std::string::npos) {
            // Middle rounds
            psep_ = nsep_ + 1;
        } else {
            // Done
            psep_ = nsep_;
        }

        nsep_ = query_.find_first_of("&;", psep_);

        if (psep_ == query_.size()) {
            // Drop trailing delimiters
            psep_ = nsep_ = std::string::npos;
            return;
        }
    }

    bool equal(Iterator const& o) const {
        return &query_ == &o.query_ && psep_ == o.psep_;
    }

    std::pair<std::string, std::string> const& dereference() const {
        std::string tmp = query_.substr(psep_,
            nsep_ != std::string::npos ? nsep_ - psep_ : nsep_);
        scratch = splitKeyValue(tmp);
        return scratch;
    }
private:
    friend class boost::iterator_core_access;

    static std::pair<std::string, std::string> splitKeyValue(
            std::string const& params) {
        size_t pos = params.find('=');
        // TODO: so many unnecessary copies :(
        if (pos == std::string::npos) {
            return std::make_pair(params, "");
        }
        return std::make_pair(params.substr(0, pos),
            params.substr(pos + 1, std::string::npos));
    }

    std::string const& query_;
    mutable std::pair<std::string, std::string> scratch; // Le sigh
    size_t psep_;
    size_t nsep_;
};

inline QueryString::Iterator QueryString::begin() const {
    Iterator ret(query_, std::string::npos, 0);
    ret.increment();
    return ret;
}

inline QueryString::Iterator QueryString::end() const {
    return Iterator(query_, std::string::npos, std::string::npos);
}

} // topper namespace

#endif // SRC_QUERY_STRING_H_
