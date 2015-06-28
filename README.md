Topper
======


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

Note the similarity to Jersey

### Resource matching


Query parameters
----------------

In progress.

Post parameters
---------------

In progress.

Request entities
----------------

In progress.

License
-------
Copyright © 2015 Nathan Rosenblum <flander@gmail.com>

Licensed under the MIT License.
