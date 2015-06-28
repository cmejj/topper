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
#include <vector>

#include <gtest/gtest.h>

#include "detail/dispatcher.h"
#include "detail/server-impl.h"
#include "resource.h"
#include "resource_matcher.h"
#include "response.h"

namespace topper {
namespace {

class NoParamResource : public Resource {
public:
    explicit NoParamResource(std::string const& path) : Resource(path) { }
};

class OneStringParamResource : public Resource {
public:
    explicit OneStringParamResource(std::string const& path)
        : Resource(path) { }

    Response get(StringParam p1) const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN, p1.value());
    }
};

class TwoStringParamResource : public Resource {
public:
    explicit TwoStringParamResource(std::string const& path)
        : Resource(path) { }

    Response get(StringParam p1, StringParam p2) const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
                        p1.value() + " " + p2.value());
    }
};

TEST(ResourceMatcherTest, BasicFunctionality) {
    ResourceMatcher matcher;

    NoParamResource res1 {"/"};
    NoParamResource res2 {"/foo"};
    NoParamResource res3 {"/foo/bar"};

    matcher.addResource(&res1, detail::bindMethods(&res1));
    matcher.addResource(&res2, detail::bindMethods(&res2));
    matcher.addResource(&res3, detail::bindMethods(&res3));

    EXPECT_EQ(&res1, matcher.match("/").get().resource);
    EXPECT_EQ(&res2, matcher.match("/foo").get().resource);
    EXPECT_EQ(&res3, matcher.match("/foo/bar").get().resource);

    EXPECT_FALSE(matcher.match("/notreal"));
    EXPECT_FALSE(matcher.match("/notreal/foo"));
    EXPECT_FALSE(matcher.match("/notreal/foo/bar"));
}

TEST(ResourceMatcherTest, ParameterMatching) {
    ResourceMatcher matcher;

    OneStringParamResource res1 {"/{p1}"};
    OneStringParamResource res2 {"/foo/{p1}"};
    OneStringParamResource res3 {"/{p1}/foo"};
    OneStringParamResource res4 {"/foo/bar/{p1}"};

    matcher.addResource(&res1, detail::bindMethods(&res1));
    matcher.addResource(&res2, detail::bindMethods(&res2));
    matcher.addResource(&res3, detail::bindMethods(&res3));
    matcher.addResource(&res4, detail::bindMethods(&res4));

    auto validate = [&](std::string const& path, Resource *expected) {
        auto match = matcher.match(path);
            // Got a match
            ASSERT_TRUE(match) << path;
            // Got the right match
            EXPECT_EQ(expected, match.get().resource) << path;
            // Got back a parameter
            EXPECT_EQ(1U, match.get().parameters.size());
        };

    // Basics
    validate("/foo", &res1);
    validate("/foo/bar", &res2);
    validate("/bar/foo", &res3);
    validate("/foo/bar/baz", &res4);

    // Some others
    validate("/bar", &res1);
}

TEST(ResourceMatcherTest, MultiParameterMatching) {
    ResourceMatcher matcher;

    OneStringParamResource res1 {"/foo/{p1}"};
    TwoStringParamResource res2 {"/{p1}/{p2}"};

    matcher.addResource(&res1, detail::bindMethods(&res1));
    matcher.addResource(&res2, detail::bindMethods(&res2));

    auto validate = [&](std::string const& path, Resource *expected, size_t c) {
        auto match = matcher.match(path);
            // Got a match
            ASSERT_TRUE(match) << path;
            // Got the right match
            EXPECT_EQ(expected, match.get().resource) << path;
            // Got back expected number of parameters
            EXPECT_EQ(c, match.get().parameters.size());
        };

    // Matching is greedy in the number of parameters; res1 can never match.
    validate("/foo/bar", &res2, 2);

    TwoStringParamResource res3 {"/foo/{p1}/short/{p2}"};
    TwoStringParamResource res4 {"/foo/longer/{p1}/{p2}"};

    matcher.addResource(&res3, detail::bindMethods(&res3));
    matcher.addResource(&res4, detail::bindMethods(&res4));

    // Matching is greedy in the size of the literals; res4 wins
    validate("/foo/longer/short/bar", &res4, 2);

    // But res3 can still be reached
    validate("/foo/baz/short/bar", &res3, 2);
}

} // anonymous namespace
} // topper namespace
