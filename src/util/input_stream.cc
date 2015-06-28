
#include "util/input_stream.h"

namespace topper {
namespace util {

UvInputStream::~UvInputStream() {
    if (stream_) {
        closeAndRelease();
    }
}

UvInputStream::UvInputStream(uv_stream_t *stream) : stream_(stream) { }

void UvInputStream::closeAndRelease() {
    // DCHECK(stream_);
    uv_close(reinterpret_cast<uv_handle_t*>(stream_), nullptr);
    delete stream_;
    stream_ = nullptr;
}

void UvInputStream::close() {
    // Idempotent
    if (stream_) {
        closeAndRelease();
    }
}

namespace {

void readCb(uv_stream_t *stream, ssize_t nread, uv_buf_t buf) {
    //auto *ctx = reinterpret_cast<UvInputStream*>(stream->data);
}

uv_buf_t allocCb(uv_handle_t *handle, size_t suggested) {
    // TODO: because we issue sequential reads, we can maintain a fixed buffer and
    // avoid the allocator overhead here.
    return uv_buf_init(new char[suggested], suggested);
}

} // unnamed namespace

ssize_t UvInputStream::read(char *buf, size_t len) {
    // DCHECK(buf);

    // start_read

    // await callback

    // stop read if enough data
}

} // util namespace
} // topper namespace
