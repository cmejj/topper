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

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <string>

namespace topper {

class Resource;
class ServerImpl;

class Server {
public:
    ~Server();
    /**
     * Configure a server for the specified addess and port.
     *
     * The server is not running until the start() method is invoked.
     *
     * This method throws on egregious configuration errors like a malformed
     * ip address string.
     *
     * @param[in]      ipaddr      the listen address, in dotted-quad notation
     * @param[in]      port        the listen port
     *
     */
    Server(std::string const& ipaddr, short port); // throws

    /** Register the resource endpoint.
     *
     * The server immediately begins serving requests for the
     * registered resource.
     *
     * @param[in]      resource        a resource
     */
    template<typename R>
    void registerResource(R *resource);

    /**
     * Start serving registered resources.
     *
     * When this method returns, the server is running bound to the
     * configured interface(s) and port, and is ready to serve requests.
     *
     * This method may throw an exception if a bind or other unrecoverable
     * error occurs.
     */
    void start(); // throws

    /**
     * Stop the server and wait for ongoing requests to complete.
     *
     * When this method returns, the server is no longer listening for
     * incomming connections and all ongoing processing has completed.
     */
    void stopAndWait();

    /**
     * Wait for the server to exit.
     *
     * Will not return until the server has been stopped.
     */
    void wait();

   /**
    * Start an admin interface running on the specified interface and port.
    *
    * The admin server provides the following endpoints:
    *
    *      /ping           Responds 'pong' to indicate that the server is up
    *
    * @param ipaddr the interface to run on (typically the loopback ip)
    * @param port the port to bind (or 0 to use any ephemeral port)
    * @throws an exception if invoked more than once
    */
   void startAdminServer(std::string const& ipaddr, short port);
private:
    ServerImpl *internal_;
};

} // topper namespace

#include "detail/server-impl.h"

#endif // SERVER_H_
