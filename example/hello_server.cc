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

#include <stdlib.h>
#include <unistd.h>

#include <string>

#include "resource.h"
#include "response.h"
#include "server.h"

namespace topper {

class HelloResource : public Resource {
public:
    HelloResource() : Resource("/") { }
    Response get() const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("Hello, World\n"));
    }
};

class HelloParamResource : public Resource {
public:
    HelloParamResource() : Resource("/{user}") { }
    Response get(StringParam const& param) const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("Hello, " + param.value() + "\n"));
    }
};

class HelloDoubleParamResource : public Resource {
public:
    HelloDoubleParamResource() : Resource("/{user}/{message}") { }
    Response get(StringParam const& user, StringParam const& message) const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("Hello, " + user.value() + ", " + message.value() + "\n"));
    }

    Response put(StringParam const& user, StringParam const& message,
            Entity const& entity) const {
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("PUT Hello, " + user.value() + ", " + message.value()
                + ": " + entity.toString() + "\n"));
    }
};

class HelloQueryParamResource : public Resource {
public:
    HelloQueryParamResource() : Resource("/{user}/details/get") { }
    Response get(StringParam const& user, QueryParams const& params) const {
        std::string querystr;
        auto val = params.get("query");
        if (!val.empty()) {
            querystr = val.front();
        }
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("Hello, " + user.value() + ", query: ")
                + querystr + "\n");
    }
};

class HelloPostParamResource : public Resource {
public:
    HelloPostParamResource() : Resource("/{user}/details/post") { }
    Response post(StringParam const& user, PostParams const& params) const {
        std::string querystr;
        auto val = params.get("query");
        if (!val.empty()) {
            querystr = val.front();
        }
        return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
            std::string("Hello, " + user.value() + ", post[query]: ")
                + querystr + "\n");
    }
};

} // topper namespace

int main(int argc, char **argv) {
    topper::HelloResource hello;
    topper::HelloParamResource phello;
    topper::HelloDoubleParamResource dhello;
    topper::HelloQueryParamResource qhello;
    topper::HelloPostParamResource pphello;

    int16_t port = 31337;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    topper::Server server("127.0.0.1", port);
    server.registerResource(&hello);
    server.registerResource(&phello);
    server.registerResource(&dhello);
    server.registerResource(&qhello);
    server.registerResource(&pphello);
    server.start();

    // Also start an admin server on some ephemeral port
    server.startAdminServer("127.0.0.1", 0);

    // Wait for interruption
    server.wait();
    return 0;
}
