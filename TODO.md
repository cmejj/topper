Things to do
============

Basic functionality
-------------------

There's quite a bit of basic HTTP / REST functionality missing in the initial
implementation.

### Support post / put entities

Not exactly representational state transfer w/o it.

### Support query parameters

Presently Topper only supports path parameters in methods. Ideally the path
parameter matching mechanism should support query parameters as well, e.g.

    Response get(StringParameter const& param1, QueryParam<int> const& q1,
        QueryParam<std::string> const& q2, ...);

One thing that's missing here is a defaulting mechanism, which is somewhat
problematic. An alternative would be to use a dictionary-based scheme, e.g.

    Response get(StringParameter const& param1, QueryParams const& q) {

        // ...

        int q1 = q['q1'];
        std::string q2 = q['q2'];

        // ...
    }

Unfortunately, this precludes automatic validation of queries.

### Support form parameters

Same as query parameters above, and with the same challenges of ordering and
defaulting.

### Missing HTTP verbs

 - Support OPTIONS
 - Support HEAD
 - Support TRACE

### WTF about chunked encoding

The Joyent http-parser is less easy to use with chunked encodings; you don't
know that you have all of the headers until on-message-complete.

Interface convenience
---------------------

 - Iterable parameter collections

Performance and limits
----------------------

 - Use streams for documents in requests and responses
 - Chunked encoding
 - Respect keep-alive
 - Move path parameters into tuples?
 - Use `forward_as_parameters` or whatever that was?

Testing
-------

 - Add HTTP client library for testing

Next version support
--------------------

 - SSL support
 - spdy support
