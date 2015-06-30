Topper
======

Topper is a lightweight framework for defining RESTful services in C++. It
provides a library of event-driven IO, logging, and metrics collection that
allow developers to focus on defining services instead of the low-level
details of writing HTTP servers.

Topper is a work in progress; refer to the [task list](TODO.md) for details &
usage caveats.

TL;DR
-----

A minimal Topper server requires only a few lines of code:

    #include "resource.h"
    #include "response.h"
    #include "server.h"

    using topper::Resource;
    using topper::Server;

    class HelloResource : public Resource {
    public:
        HelloResource() : Resource("/hello/{user}") { }

        Response get(StringParameter const& param) const {
            return Response(HttpCode::OK, MediaType::TEXT_PLAIN,
                std::string("Hello, ") + param.value + std::string("\n"));
        }
    }

    int main(void) {
        HelloResource hello;
        Server server("127.0.0.1", 8080);
        server.registerResource(&hello);
        server.start();

        // Wait for interruption
        server.wait();
        return 0;
    }

Building
--------

    mkdir build
    cd build
    cmake ..
    make

Path parameters
---------------

Resources can define _path parameter_ templates that are provided to the
request handler. In the example above, the server defines an endpoint
`/hello/{user}` with a single path parameter. Requests to that endpoint will
have the last path component translated to an input parameter to the relevant
handler method; for example, requests to `/hello/visitor` will result in
invocation of `HelloResource::get` with a "visitor" parameter.

See the [hello server](example/hello_server.cc) for more examples of path
parameter usage.

### Resource matching

In progress.

Query parameters
----------------

In progress.

Post parameters
---------------

In progress.

Request entities
----------------

In progress.

Admin endpoints
---------------

Topper servers can be configured to expose an administration interface on a
separate `ip, port` tuple. This interface includes the following endpoints:

    /ping
    /metrics

The `/ping` endpoint responds to `GET` requests with a document containing
`pong`, and can be used to check whether the server is running.

The `/metrics` endpoint publishes server metrics in JSON format:

```
# Admin port 40967

$ curl http://127.0.0.1:40967/metrics |python -mjson.tool
{
    "counters": {},
    "timers": {
        "topper.resource.dispatch": {
            "count": 14,
            "m15_rate": 0.2110435060583438,
            "m1_rate": 0.3523419255254859,
            "m5_rate": 0.23273016404633015,
            "max": 0.003941,
            "mean": 0.0003459285714285714,
            "min": 2.3e-05,
            "p50": 2.55e-05,
            "p75": 8.274999999999999e-05,
            "p95": 0.003941,
            "p99": 0.003941,
            "p999": 0.003941,
            "stdev": 0.0010411176686619047
        }
    }
}
```

Runtime application metrics are provided by the
[ccmetrics](https://github.com/flandr/ccmetrics) library; refer to the library
documentation for details.

License
-------

Copyright © 2015 Nathan Rosenblum <flander@gmail.com>

Licensed under the MIT License.

References
----------

This project was inspired by the author's pleasant experience using
[Dropwizard](http://www.dropwizard.io/) to quickly bring up small, RESTful
services in Java.

Topper's path component parsing rules and interfaces ow much to
[Jersey](https://jersey.java.net/), the reference implementation of
[JAX-RS](http://jax-rs-spec.java.net/).
