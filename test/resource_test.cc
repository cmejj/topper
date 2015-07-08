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
#include "parameter_internal.h"
#include "resource.h"
#include "response.h"

namespace topper {
namespace {

// A resource that leaves all methods defaulted
class DefaultResource : public Resource {
public:
    explicit DefaultResource(std::string const& path) : Resource(path) { }
};

// Convenience helper
template<typename R, typename... Args>
Response run(Resource const& resource, Response (R::*method)(Args...) const,
        std::vector<std::string> const& params, UriInfo const& uriInfo) {
    return detail::ResourceDispatcher::dispatch(&resource, method, params,
        uriInfo);
}

UriInfo mkBlankUriInfo() {
    static QueryParamsImpl queryParams;
    static PostParamsImpl postParams;
    static HeaderParamsImpl headerParams;
    static Entity entity;
    return UriInfo { queryParams, postParams, headerParams, entity };
}

TEST(ResourceTest, DefaultResponseIsNotAllowed) {
    DefaultResource r("/foo");
    std::vector<std::string> p;
    UriInfo u = mkBlankUriInfo();
    EXPECT_EQ(HttpCode::NOT_ALLOWED, run(r, &DefaultResource::get, p, u).code());
    EXPECT_EQ(HttpCode::NOT_ALLOWED, run(r, &DefaultResource::put, p, u).code());
    EXPECT_EQ(HttpCode::NOT_ALLOWED, run(r, &DefaultResource::post, p, u).code());
    EXPECT_EQ(HttpCode::NOT_ALLOWED, run(r, &DefaultResource::del, p, u).code());
}

TEST(ResourceTest, ResourceExposesPath) {
    const std::string kPath {"/foo"};
    EXPECT_EQ(kPath, DefaultResource(kPath).path());
}

// Resource that implements all methods and returns OK
class OkResource : public Resource {
public:
    OkResource() : Resource("/foo") { }

    Response get() const {
        return resp_;
    }

    Response put() const {
        return resp_;
    }

    Response post() const {
        return resp_;
    }

    Response del() const {
        return resp_;
    }

    Response head() const {
        return resp_;
    }
private:
    Response resp_ {HttpCode::OK, MediaType::TEXT_PLAIN, ""};
};

TEST(ResourceTest, ImplementedMethodsOverrideDefaults) {
    OkResource r;
    std::vector<std::string> p;
    UriInfo u = mkBlankUriInfo();
    EXPECT_EQ(HttpCode::OK, run(r, &OkResource::get, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &OkResource::put, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &OkResource::post, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &OkResource::del, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &OkResource::head, p, u).code());
}

// Resource with methods that use query parameters
template<typename Func>
class QueryParamResource : public Resource {
public:
    QueryParamResource(Func f) : Resource("/foo"), f_(f) { }

    Response get(QueryParams const& qp) const {
        f_(qp);
        return resp_;
    }

    Response put(QueryParams const& qp) const {
        f_(qp);
        return resp_;
    }

    Response post(QueryParams const& qp) const {
        f_(qp);
        return resp_;
    }

    Response del(QueryParams const& qp) const {
        f_(qp);
        return resp_;
    }
private:
    Func f_;
    Response resp_ {HttpCode::OK, MediaType::TEXT_PLAIN, ""};
};

TEST(ResourceTest, QueryParamsArePassedToResource) {
    std::vector<std::string> p { "foo" };
    QueryParamsImpl queryParams(
        std::unordered_multimap<std::string, std::string>{
            { "foo", "1" }, { "bar", "2" } });
    PostParamsImpl postParams;
    HeaderParamsImpl headerParams;
    Entity entity;
    UriInfo u { queryParams, postParams, headerParams, entity };
    auto compare = [&u](QueryParams const& qp) -> void {
            ASSERT_EQ(u.queryParams, qp);
        };
    QueryParamResource<decltype(compare)> r(compare);
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::get, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::put, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::post, p, u).code());
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::del, p, u).code());
}

template<typename Func>
class PostParamResource : public Resource {
public:
    PostParamResource(Func f) : Resource("/foo"), f_(f) { }

    Response post(PostParams const& pp) const {
        f_(pp);
        return resp_;
    }
private:
    Func f_;
    Response resp_ {HttpCode::OK, MediaType::TEXT_PLAIN, ""};
};

TEST(ResourceTest, PostParamsArePassedToResourcePostMethod) {
    std::vector<std::string> p { "foo" };
    QueryParamsImpl queryParams;
    HeaderParamsImpl headerParams;
    PostParamsImpl postParams(
        std::unordered_multimap<std::string, std::string>{
            { "post1", "1" }, { "post2", "2" } });
    Entity entity;
    UriInfo u { queryParams, postParams, headerParams, entity };
    auto compare = [&u](PostParams const& pp) -> void {
            ASSERT_EQ(u.postParams, pp);
        };
    PostParamResource<decltype(compare)> r(compare);
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::post, p, u).code());
}

template<typename Func>
class HeaderParamResource : public Resource {
public:
    HeaderParamResource(Func f) : Resource("/foo"), f_(f) { }

    Response post(HeaderParams const& hp) const {
        f_(hp);
        return resp_;
    }
private:
    Func f_;
    Response resp_ {HttpCode::OK, MediaType::TEXT_PLAIN, ""};
};

TEST(ResourceTest, HeaderParamsArePassedToResource) {
    std::vector<std::string> p { "foo" };
    QueryParamsImpl queryParams;
    HeaderParamsImpl headerParams(
        std::unordered_map<std::string, std::string>{
            { "header1", "value1" }, { "header2", "value2" } });
    PostParamsImpl postParams;
    Entity entity;
    UriInfo u { queryParams, postParams, headerParams, entity };
    auto compare = [&u](HeaderParams const& hp) -> void {
            ASSERT_EQ(u.headerParams, hp);
        };
    HeaderParamResource<decltype(compare)> r(compare);
    EXPECT_EQ(HttpCode::OK, run(r, &decltype(r)::post, p, u).code());
}

} // anonymous namespace
} // topper namespace
