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

#ifndef SRC_PATH_COMPONENTS_H_
#define SRC_PATH_COMPONENTS_H_

namespace topper {

/**
 * Helper to represent a magfs path ('a/b/c') as its individual components.
 *
 * Use this instead of doing the component-wise find('/') dance yourself.
 * Regrettably still does a lot of string copying. Really ought to add
 * immutable strings that support efficient substring extraction.
 *
 * Trailing slashes are ignored: the path 'a/b/c/' decomposes to the
 * coponent list [ a, b, c ], not [ a, b, c, '' ]
 *
 * Leading slashes and consecutive delimiters are not handled.
 */
class PathComponents {
public:
    explicit PathComponents(std::string const& path) : path_(path) { }
    class Iterator;
    Iterator begin() const;
    Iterator end() const;
private:
    std::string path_;
};

class PathComponents::Iterator :
        public boost::iterator_facade<PathComponents::Iterator,
                                      const std::string,
                                      boost::forward_traversal_tag> {
public:
    Iterator(std::string const& path, size_t p, size_t n) : path_(path),
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

        nsep_ = path_.find('/', psep_);

        if (psep_ == path_.size()) {
            // Drop trailing delimiters
            psep_ = nsep_ = std::string::npos;
            return;
        }
    }

    bool equal(Iterator const& o) const {
        return &path_ == &o.path_ && psep_ == o.psep_;
    }

    std::string const& dereference() const {
        scratch = path_.substr(psep_,
            nsep_ != std::string::npos ? nsep_ - psep_ : nsep_);
        return scratch;
    }
private:
    friend class boost::iterator_core_access;

    std::string const& path_;
    mutable std::string scratch; // Le sigh
    size_t psep_;
    size_t nsep_;
};

inline PathComponents::Iterator PathComponents::begin() const {
    Iterator ret(path_, std::string::npos, 0);
    ret.increment();
    return ret;
}

inline PathComponents::Iterator PathComponents::end() const {
    return Iterator(path_, std::string::npos, std::string::npos);
}

} // topper namespace

#endif // SRC_PATH_COMPONENTS_H_
