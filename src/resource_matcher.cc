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

#include <deque>
#include <set>
#include <stack>
#include <string>

#include <boost/optional.hpp>

#include "logging.h"
#include "path_components.h"
#include "resource_matcher.h"

namespace topper {

namespace {

// Returns true if the component is {...}
bool isVariable(std::string const& component) {
    return component.size() >= 2 && component[0] == '{'
        && component[component.size() - 1] == '}';
}

} // anonymous namespace

ResourceMatcher::Node::~Node() {
    DCHECK(children.empty());
}

ResourceMatcher::~ResourceMatcher() {
    clear();
}

void ResourceMatcher::addResource(Resource *resource,
        detail::Methods const& methods) {
    DCHECK(resource);

    Node *cur = &head_;

    for (auto const& component : PathComponents(resource->path())) {
        bool isvar = isVariable(component);

        if (isVariable(component)) {
            // Variable component handling
            if (!cur->varChild) {
                cur->varChild = new Node();
            }
            cur = cur->varChild;
            continue;
        }
        auto match = cur->children.find(component);
        if (match != cur->children.end()) {
            cur = match->second;
        } else {
            Node *next = new Node();
            cur->children[component] = next;
            cur = next;
        }
    }

    // If there is already a resource at the terminal node, this is a resource
    // template exception condition and an error, which we log and throw
    if (cur->resource) {
        LOG(ERROR) << resource->path() << " is already registered";
        throw std::runtime_error("Resource registration collision");
    }

    cur->resource = resource;
    cur->methods = methods;

    resources_.push_back(resource);
}

std::vector<Resource *> const& ResourceMatcher::resources() const {
    return resources_;
}

void ResourceMatcher::clear() {
    // Post-order DFS, deleting nodes as we go
    struct SearchState {
        Node *node;
        Node *parent;
        bool visited;
    };

    std::stack<SearchState> searchStack;
    searchStack.push({&head_, /*parent=*/ nullptr, /*visited=*/ false});

    while (!searchStack.empty()) {
        auto cur = searchStack.top();

        if (!cur.visited) {
            // We'll come back to this after processing its children
            searchStack.top().visited = true;
            for (auto const& tuple : cur.node->children) {
                searchStack.push({tuple.second, cur.node, /*visited=*/ false});
            }
            // Forget about the children
            cur.node->children.clear();
            continue;
        }

        searchStack.pop();

        // Post-order step; clean up this node
        if (cur.node != &head_){
            delete cur.node;
        }
    }
}

boost::optional<Match> ResourceMatcher::match(std::string const& path) const {
    struct SearchState {
        const Node *cur;
        std::vector<std::string> variables;
        Resource *matched;
        const detail::Methods *methods;
        bool terminated;
        std::string literals;
    };

    std::deque<SearchState> states = {{&head_, {}, NULL, NULL, false, ""}};

    for (auto const& component : PathComponents(path)) {
        // Variable matches add additional search paths
        decltype(states) add;

        // Process matches for existing search paths
        for (auto it = states.begin(), end = states.end(); it != end; ++it) {
            auto& state = *it;
            const Node *cur = state.cur;

            if (state.terminated) {
                // TODO: it would be more efficient to prune these when
                // there are large numbers of matching paths
                continue;
            }

            // Reset possible match
            state.matched = NULL;

            // A variable child adds a new seach path
            if (state.cur->varChild) {
                SearchState s = {state.cur->varChild, state.variables,
                    state.cur->varChild->resource,
                    &state.cur->varChild->methods,
                    false, state.literals + "."};
                s.variables.push_back(component);
                add.push_back(s);
            }

            // Check for literal matches
            auto match = cur->children.find(component);
            if (match == cur->children.end()) {
                // Search path is a dead end, modulo variables
                state.terminated = true;
                continue;
            }

            // Move to the matching node
            state.cur = match->second;

            // Save literal path
            state.literals += component;

            if (state.cur->resource) {
                // Possible match, if this is the last component
                state.matched = state.cur->resource;
                state.methods = &state.cur->methods;
            }
        }

        states.insert(states.end(), add.begin(), add.end());
    }

    // For each of the search paths that were explored, the path is a candidate
    // for a match if (1) it has not terminated and (2) it has a non-null
    // Resource. Sort the matches by the most template matches and then by
    // the aggregate length of non-template components and then
    // lexicographically.
    struct compare {
        bool operator()(const SearchState *s1, const SearchState *s2) {
            if (s1->variables.size() > s2->variables.size()) {
                return true;
            } else if (s1->literals.size() > s2->literals.size()) {
                return true;
            }
            return s1->literals > s2->literals;
        }
    };
    std::set<const SearchState *, compare> candidates;
    for (auto const& state : states) {
        if (state.matched) {
            candidates.insert(&state);
        }
    }

    if (candidates.empty()) {
        VLOG(3) << "No matches for " << path;
        return boost::optional<Match>();
    }

    const SearchState *ret = *candidates.begin();
    return boost::make_optional(Match{ret->matched, *ret->methods,
        ret->variables});
}

} // topper namespace
