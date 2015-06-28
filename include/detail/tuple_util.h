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

#ifndef INCLUDE_DETAIL_TUPLE_UTIL_H_
#define INCLUDE_DETAIL_TUPLE_UTIL_H_

#include <string>
#include <vector>

#include "parameter.h"

namespace topper {
namespace detail {

//
// Vector + UriInfo to parameter utilities. The UriInfo-contained parameters
// are extracted by const-referenced, while the path parameters (which must
// undergo conversion) are extracted by value.
//

template<typename ParamType>
class GetParam {
public:
    static ParamType const& get(std::vector<std::string> const& params,
            int index, UriInfo const& uriInfo);
};

template<>
inline QueryParams const& GetParam<QueryParams>::get(std::vector<std::string> const&,
        int, UriInfo const& uriInfo) {
    return uriInfo.queryParams;
}

template<>
inline PostParams const& GetParam<PostParams>::get(std::vector<std::string> const&,
        int, UriInfo const& uriInfo) {
    return uriInfo.postParams;
}

template<>
inline Entity const& GetParam<Entity>::get(std::vector<std::string> const&,
        int, UriInfo const& uriInfo) {
    return uriInfo.entity;
}

// This needn't be a copy, if we're willing to keep the original path
// parameter vector alive for the duration of the request. Consider it.
template<>
class GetParam<StringParam> {
public:
    static StringParam get(std::vector<std::string> const& params,
            int index, UriInfo const&) {
        return StringParam::parse(params[index]);
    }
};

template<typename T>
class GetParam<IntParam<T>> {
public:
    static IntParam<T> get(std::vector<std::string> const& params,
            int index, UriInfo const&) {
        return IntParam<T>::parse(params[index]);
    }
};

//
// Index selection utilities
//

template<typename ParamType>
class NextIndex {
public:
    constexpr static int next(int index) {
        // By default, don't advance
        return index;
    }
};

// StringParams advance
template<>
constexpr int NextIndex<StringParam>::next(int index) {
    return index + 1;
}

// IntParams advance
template<typename T>
class NextIndex<IntParam<T>> {
    constexpr static int next(int index) {
        return index + 1;
    }
};

//
// Vector to tuple utilities
//

template<typename T>
struct BaseType {
    typedef typename std::remove_const<
        typename std::remove_reference<T>::type>::type type;
};

// Recursive extraction
template<int Length, int Index, typename R1, typename... R>
class ExtractorHelper {
public:
    static std::tuple<R1, R...> extract(std::vector<std::string> const& params,
            UriInfo const& uriInfo) {
        // There are two compile-time methods applied here:
        //
        //  (1) an R1-specific method to select the appropriate input parameter
        //  (either uri info field or a path param). This is "GetParam".
        //
        //  (2) an R1-specific method to determine whether to increment the
        //  index into the path parameters vector. This is "NextIndex".

        // The input types are already suitably converted to either reference
        // or value types by the invoker for use in the return tuple, but
        // will need remove-reference before template type deduction for
        // the extraction & index methods. They also need to be de-constified.

        return std::tuple_cat(std::tuple<R1>(
            GetParam<typename BaseType<R1>::type>::get(params,
                Index, uriInfo)),
            ExtractorHelper<Length - 1, NextIndex<typename BaseType<R1>::type>
                ::next(Index), R...>::extract(params, uriInfo));
    }
};

// Base case for extraction
template<int Index, typename R>
class ExtractorHelper<1, Index, R> {
public:
    static std::tuple<R> extract(std::vector<std::string> const& params,
            UriInfo const& uriInfo) {
        return std::tuple<R>(GetParam<typename BaseType<R>::type>::get(params,
            Index, uriInfo));
    }
};

template<int Length, typename... R>
class Extractor {
public:
    static std::tuple<R...> extract(std::vector<std::string> const& params,
            UriInfo const& uriInfo) {
        return ExtractorHelper<Length, 0, R...>::extract(params, uriInfo);
    }
};

// Specialization for zero parameters
template<>
class Extractor<0> {
public:
    static std::tuple<> extract(std::vector<std::string> const&,
            UriInfo const&) {
        return std::tuple<>();
    }
};

//
// Compute the indices of a tuple. This implementation is
// from this great StackOverflow post: http://bit.ly/tuple-expansion
//

template<unsigned...> struct index_tuple{};

template<unsigned I, typename IndexTuple, typename... Types>
struct make_indices_impl;

template<unsigned I, unsigned... Indices, typename T, typename... Types>
struct make_indices_impl<I, index_tuple<Indices...>, T, Types...>
{
  typedef typename
    make_indices_impl<I + 1,
                      index_tuple<Indices..., I>,
                      Types...>::type type;
};

template<unsigned I, unsigned... Indices>
struct make_indices_impl<I, index_tuple<Indices...> >
{
  typedef index_tuple<Indices...> type;
};

template<typename... Types>
struct make_indices
  : make_indices_impl<0, index_tuple<>, Types...>
{};

} // detail namespace
} // topper namespace

#endif // INCLUDE_DETAIL_TUPLE_UTIL_H_
