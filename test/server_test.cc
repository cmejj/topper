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

#include <gtest/gtest.h>

#include "server.h"
#include "util.h"

namespace topper {

class ServerTest : public ::testing::Test {
protected:
    static EphemeralPorts ports;
};

EphemeralPorts ServerTest::ports{};

TEST_F(ServerTest, MalformedAddressThrows) {
    ASSERT_THROW({Server server("1.2.3.4.5", ports.get());},
        std::invalid_argument);
    ASSERT_THROW({Server server("foo", ports.get());},
        std::invalid_argument);
}

TEST_F(ServerTest, StopAndWaitThrowsIfNotStarted) {
    Server server("127.0.0.1", ports.get());
    ASSERT_THROW({server.stopAndWait();}, std::logic_error);
}

TEST_F(ServerTest, StartThrowsIfAlreadyStarted) {
    Server server("127.0.0.1", ports.get());
    server.start();
    ASSERT_THROW({server.start();}, std::logic_error);
}

} // topper namespace
