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

#ifndef SRC_RESOURCE_MATCHER_H_
#define SRC_RESOURCE_MATCHER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include "resource.h"
#include "server.h"

namespace topper {

struct Match {
    Resource *resource;
    // TODO: copying this around on every match is wasteful. The methods
    // are sort of bound to the resource; why can't these be extracted
    // self-referentially and stored?
    detail::Methods methods;
    std::vector<std::string> parameters;
};

/**
 * Selects matching resources for a URI based on registered templates.
 *
 * Template matching rules are a [dramatic] simplification of JAX-RS.
 * Matching is done component-wise on the path (i.e., on each component
 * delimited by '/' characters, and the longest resource in terms of
 * _path components_ (not literal characters as in JAX-RS) wins, with
 * ties broken by number of template parameters matched. Behavior is
 * best conveyed by example:
 *
 * Consider two resources, `/orgs/{org}/user/{id}` and
 * `/orgs/all/user/{id}`. For the following requests
 *
 *     GET /orgs/foo.com/user/7
 *
 * and
 *
 *     GET /orgs/all/user/11
 *
 * the former wins, as it matches two template variables.
 *
 */
// TODO: unregistering resource paths
class ResourceMatcher {
public:
    ResourceMatcher() { }
    ~ResourceMatcher();

    /**
     * Add a resource to the matcher.
     *
     * The caller is responsible for ensuring that the @p resource parameter
     * remains viable for the lifetime of the matcher.
     *
     * @param[in]      resource        the resource object
     * @param[in]      methods         the resource methods
     * @throws         PathException   if the paths collide
     */
    void addResource(Resource *resource, detail::Methods const& methods);

    /** @return a matching resource. */
    boost::optional<Match> match(std::string const& path) const;

    /** @return all registered resources. */
    std::vector<Resource *> const& resources() const;
private:
    void clear();

    struct Node {
        Node() : resource(nullptr), varChild(nullptr) { }
        Node(Resource *resource, detail::Methods const& methods)
            : resource(resource), methods(methods) { }
        ~Node();

        // Resource
        Resource *resource;

        // Invocation methods
        detail::Methods methods;

        // Non-variable children (literal matches)
        std::unordered_map<std::string, Node*> children;

        // Variable child
        Node *varChild;
    };

    Node head_;

    // TODO: provide resource iterator interface to avoid this duplicate array
    std::vector<Resource *> resources_;
};

} // topper namespace

#endif // SRC_RESOURCE_MATCHER_H_
