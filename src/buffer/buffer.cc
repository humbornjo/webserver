#include "buffer.h"

Buffer::Buffer(int initBuffSize): 
buffer_(initBuffSize), read_pos_(0), write_pos_(0) {}

size_t Buffer::ReadableBytes() const{
    return write_pos_-read_pos_;
}

size_t Buffer::WritableBytes() const{
    return buffer_.size() - write_pos_;
}

size_t Buffer::PrependableBytes() const {
    return read_pos_;
}

const char *Buffer::Peek() const{
    return BeginPtr_()+read_pos_;
}

void Buffer::EnsureWriteable(size_t len) {
    if (WritableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WritableBytes()>=len);
}

void Buffer::HasWritten(size_t len) {
    write_pos_+=len;
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    read_pos_+=len;
}

void Buffer::RetrieveUntil(const char *end) {
    assert(Peek() <= end);
    Retrieve(end-Peek());
}

void Buffer::RetrieveAll() {
    memset(BeginPtr_(), 0, buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    // basic_string( const CharT* s, size_type count,
    //       const Allocator& alloc = Allocator() );
    return std::string(Peek(), ReadableBytes());
}

const char *Buffer::BeginWriteConst() const {
    return BeginPtr_() + write_pos_;
}

char *Buffer::BeginWrite() {
    return BeginPtr_() + write_pos_;
}

void Buffer::Append(const std::string &str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const char *str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str+len, BeginWrite());
}

void Buffer::Append(const void *data, size_t len) {
    assert(data);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const Buffer &buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t Buffer::ReadFd(int fd, int *save_errno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    iov[0].iov_base = BeginPtr_()+write_pos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len<0) {
        *save_errno = errno;
    } else if (static_cast<size_t>(len) <= WritableBytes()) {
        write_pos_+=len;
    } else {
        write_pos_ = buffer_.size();
        Append(buff, len-writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *save_errno) {
    size_t read_size = ReadableBytes();
    ssize_t len = write(fd, Peek(), read_size);
    if (len < 0) {
        *save_errno = errno;
        return len;
    } 
    read_pos_ += len;
    return len;
}

char *Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char *Buffer::BeginPtr_() const{
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
    if (ReadableBytes() + PrependableBytes() < len) {
        buffer_.resize(write_pos_ + len + 1); // extra 1 indicates null-terminated
    } else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_()+read_pos_, BeginPtr_() + write_pos_, BeginPtr_());
        read_pos_ = 0;
        write_pos_ = readable;
        assert(readable == ReadableBytes());
    }
}
