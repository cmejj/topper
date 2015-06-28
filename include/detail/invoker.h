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

#ifndef INCLUDE_DETAIL_INVOKER_H_
#define INCLUDE_DETAIL_INVOKER_H_

#include "detail/tuple_util.h"

namespace topper {
namespace detail {

//
// Reference conversion helpers
//

// "Maybe Remove Reference". By default, does not remove the reference. This
// is used for constructing the types used in a tuple for the parameter
// extractor.
template<typename R>
struct MRR {
    typedef R type;
};

// Path parameters are copied, so references must be removed for use as
// types in the return tuple from the extractor.
template<>
struct MRR<StringParam const&> {
    typedef StringParam type;
};
template<typename T>
struct MRR<IntParam<T> const&> {
    typedef IntParam<T> type;
};

//
// Invoker helpers
//

template<typename Super, typename Derived, typename Ret, typename... Args,
         unsigned int... Indices>
Ret invoke_member_helper(Super *c, Ret (Derived::*f)(Args...) const,
        index_tuple<Indices...> /*unused*/,
        std::tuple<typename MRR<Args>::type...> const& args) {
    return (static_cast<const Derived*>(c)->*f)(std::get<Indices>(args)...);
}

template<typename Ret, typename... Args, unsigned int... Indices>
Ret invoke_helper(Ret (*f)(Args...), index_tuple<Indices...> /*unused*/,
                  std::tuple<typename MRR<Args>::type...> const& args) {
    return f(std::get<Indices>(args)...);
}

// Invokes a function on a tuple of arguments
template<typename Ret, typename... Args>
Ret invoke(Ret (*f)(Args...),
        std::tuple<typename MRR<Args>::type...> const& args) {
    return invoke_helper(f, typename make_indices<Args...>::type(), args);
}

// Invokes a member function on a tuple of arguments
template<typename Super, typename Derived, typename Ret, typename... Args>
Ret invoke_member(Super *c, Ret (Derived::*f)(Args...) const,
        std::tuple<typename MRR<Args>::type...> const& args) {
    return invoke_member_helper(c, f, typename make_indices<Args...>::type(),
        args);
}

} // detail namespace
} // topper namespace

#endif // INCLUDE_DETAIL_INVOKER_H_
