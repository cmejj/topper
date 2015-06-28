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

#include <algorithm>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include "util.h"

namespace topper {

int16_t EphemeralPorts::get() {
    int16_t ret = **cur_;
    (*cur_)++;
    return ret;
}

EphemeralPorts::~EphemeralPorts() {
    delete cur_;
}

EphemeralPorts::EphemeralPorts() {
    ports_.reserve(kEnd - kBegin + 1);
    for (int16_t i = kBegin; i <= kEnd; ++i) {
        ports_.push_back(i);
    }

    boost::mt19937 gen;

    // Fischer-Yates shuffle
    for (size_t i = ports_.size() - 1; i > 0; --i) {
        boost::random::uniform_int_distribution<size_t> range(0, i);
        size_t j = range(gen);
        std::swap(ports_[i], ports_[j]);
    }

    cur_ = new Iterator(ports_);
}

} // topper namespace
