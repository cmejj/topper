Things to do
============

Basic functionality
-------------------

There's quite a bit of basic HTTP / REST functionality missing in the initial
implementation.

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

 - SSL support (depends on implementation in WTE, mostly)
 - spdy / HTTP/2 support
