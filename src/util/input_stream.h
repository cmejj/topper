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

#ifndef SRC_UTIL_INPUT_STREAM_H_
#define SRC_UTIL_INPUT_STREAM_H_

#include <uv.h>

namespace topper {
namespace util {

/// An input stream wrapper for a libuv uv_stream_t.
///
/// This class is not thread safe.
///
class UvInputStream {
public:
    /// Closes the stream if it has not been explicitly closed.
    ///
    ~UvInputStream();

    UvInputStream() = delete;

    /// Constructs an input stream reader from an initialized stream.
    ///
    /// The underlying \p stream object must be initialized and associated with
    /// and event loop.
    ///
    /// @param[in]      stream      a libuv stream
    ///
    explicit UvInputStream(uv_stream_t *stream);

    /// Close this stream and release the associated resources.
    ///
    void close();

    /// Reads up to \p len bytes into \p buf.
    ///
    /// @param[in]      buf     an allocated buffer
    /// @param[in]      len     the buffer length
    ///
    /// @return                 the number of bytes read, or -1 on error.
    ///                         the return value 0 indicates end of file.
    ///
    ssize_t read(char *buf, size_t len);
private:
    void closeAndRelease();

    uv_stream_t *stream_;
    std::mutex
};

} // util namespace
} // topper namespace

#endif // SRC_UTIL_INPUT_STREAM_H_
