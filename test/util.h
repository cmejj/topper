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

#include <inttypes.h>

#include <vector>

#include <boost/iterator/iterator_adaptor.hpp>

namespace topper {

/**
 * Supplies ports from the IANA emphemeral range in random order.
 *
 * The iterator for this collection is cyclic.
 */
// TODO: just bind -1, expose bound port from server
class EphemeralPorts {
public:
    EphemeralPorts();
    ~EphemeralPorts();

    int16_t get();

    // Visible for testing
    static const int16_t kBegin = 49152;
    static const int16_t kEnd = 65535;

private:
    class Iterator;
    Iterator *cur_;
    std::vector<int16_t> ports_;
};

// Cyclic iterator
class EphemeralPorts::Iterator :
        public boost::iterator_adaptor<
            EphemeralPorts::Iterator,
            std::vector<int16_t>::const_iterator,
            boost::use_default,
            std::forward_iterator_tag,
            boost::use_default> {
public:
    explicit Iterator(std::vector<int16_t> const& v)
        : iterator_adaptor(v.begin()), begin_(v.begin()), end_(v.end()) { }

    Iterator(std::vector<int16_t> const& v,
             std::vector<int16_t>::const_iterator it)
        : iterator_adaptor(it), begin_(v.begin()), end_(v.end()) { }

    void increment() {
        if (++base_reference() == end_) {
            // Reset to the beginning
            base_reference() = begin_;
        }
    }
private:
    std::vector<int16_t>::const_iterator begin_;
    std::vector<int16_t>::const_iterator end_;
};

} // topper namespace
