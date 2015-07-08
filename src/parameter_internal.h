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

#ifndef SRC_PARAMETER_INTERNAL_H_
#define SRC_PARAMETER_INTERNAL_H_

#include <unordered_map>

#include "parameter.h"

namespace topper {

class QueryParamsImpl : public QueryParams {
public:
    explicit QueryParamsImpl(
            std::unordered_multimap<std::string, std::string> &&params)
        : params_(params) { }
    QueryParamsImpl() { }

    virtual std::vector<std::string> get(std::string const& name) const final;
    virtual bool operator==(QueryParams const&) const final;
private:
    std::unordered_multimap<std::string, std::string> params_;
};

inline bool QueryParamsImpl::operator==(QueryParams const& o) const {
    // This would be a critical error if we were actually using polymorphism
    // and not just pimpling. Since this is only used in unit tests,
    // perhaps we can just remove it.
    return params_ == static_cast<QueryParamsImpl const&>(o).params_;
}

inline std::vector<std::string> QueryParamsImpl::get(
        std::string const& name) const {
    // Allocating here is ABI-unsafe; this method should instead return
    // and iterable of references, where the iterable is ABI-safe. See
    // https://blogs.gentoo.org/mgorny/2012/08/20/the-impact-of-cxx-templates-on-library-abi/
    // and a variety of other articles on ABI compatibility, e.g.
    // http://chadaustin.me/cppinterface.html,
    // http://www.ros.org/reps/rep-0009.html#definition,
    // http://programmers.stackexchange.com/questions/176681/did-c11-address-concerns-passing-std-lib-objects-between-dynamic-shared-librar
    std::vector<std::string> ret;
    auto range = params_.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

class HeaderParamsImpl : public HeaderParams {
public:
    explicit HeaderParamsImpl(
            std::unordered_map<std::string, std::string>  &&params)
        : params_(params) { }
    HeaderParamsImpl() { }

    virtual std::string get(std::string const& name) const final {
        auto it = params_.find(name);
        if (it != params_.end()) {
            return it->second;
        }
        return "";
    }
    virtual bool operator==(HeaderParams const&) const final;
private:
    std::unordered_map<std::string, std::string> params_;
};

inline bool HeaderParamsImpl::operator==(HeaderParams const& o) const {
    return params_ == static_cast<HeaderParamsImpl const&>(o).params_;
}

class PostParamsImpl : public PostParams {
public:
    explicit PostParamsImpl(
            std::unordered_multimap<std::string, std::string> &&params)
        : params_(params) { }
    PostParamsImpl() { }

    virtual std::vector<std::string> get(std::string const& name) const final;
    virtual bool operator==(PostParams const&) const final;
private:
    std::unordered_multimap<std::string, std::string> params_;
};

inline bool PostParamsImpl::operator==(PostParams const& o) const {
    // This would be a critical error if we were actually using polymorphism
    // and not just pimpling. Since this is only used in unit tests,
    // perhaps we can just remove it.
    return params_ == static_cast<PostParamsImpl const&>(o).params_;
}

inline std::vector<std::string> PostParamsImpl::get(
        std::string const& name) const {
    // Allocating here is ABI-unsafe; this method should instead return
    // and iterable of references, where the iterable is ABI-safe. See
    // https://blogs.gentoo.org/mgorny/2012/08/20/the-impact-of-cxx-templates-on-library-abi/
    // and a variety of other articles on ABI compatibility, e.g.
    // http://chadaustin.me/cppinterface.html,
    // http://www.ros.org/reps/rep-0009.html#definition,
    // http://programmers.stackexchange.com/questions/176681/did-c11-address-concerns-passing-std-lib-objects-between-dynamic-shared-librar
    std::vector<std::string> ret;
    auto range = params_.equal_range(name);
    for (auto it = range.first; it != range.second; ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

} // topper namespace

#endif // SRC_PARAMETER_INTERNAL_H_
