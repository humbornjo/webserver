#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <assert.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <sys/time.h>

template <class T> class BlockDeque {
public:
  explicit BlockDeque(size_t max_capacity = 1024);

  ~BlockDeque();

  void clear();

  bool empty();

  bool full();

  void Close();

  size_t size();

  size_t capacity();

  T front();

  T back();

  void push_back(const T &item);

  void push_front(const T &item);

  bool pop(T &item);

  bool pop(T &item, int timeout);

  void flush();

private:
  std::deque<T> deq_;

  size_t capacity_;

  std::mutex mtx_;

  bool is_close_;

  std::condition_variable cond_consumer_;

  std::condition_variable cond_producer_;
};

template <class T>
BlockDeque<T>::BlockDeque(size_t max_capacity) : capacity_(max_capacity) {
  assert(max_capacity > 0);
  is_close_ = false;
}

template <class T> BlockDeque<T>::~BlockDeque() { Close(); };

template <class T> void BlockDeque<T>::clear() {
  std::lock_guard<std::mutex> locker(mtx_);
  deq_.clear();
}

template <class T> bool BlockDeque<T>::empty() {
  std::lock_guard<std::mutex> locker(mtx_);
  return deq_.empty();
}

template <class T> bool BlockDeque<T>::full() {
  std::lock_guard<std::mutex> locker(mtx_);
  return deq_.size() >= capacity_;
}

template <class T> void BlockDeque<T>::Close() {
  {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
    is_close_ = true;
  }
  cond_consumer_.notify_all();
  cond_producer_.notify_all();
}

template <class T> size_t BlockDeque<T>::size() {
  std::lock_guard<std::mutex> locker(mtx_);
  return deq_.size();
}

template <class T> size_t BlockDeque<T>::capacity() {
  std::lock_guard<std::mutex> locker(mtx_);
  return capacity_;
}

template <class T> T BlockDeque<T>::front() {
  std::lock_guard<std::mutex> locker(mtx_);
  return deq_.front();
}

template <class T> T BlockDeque<T>::back() {
  std::lock_guard<std::mutex> locker(mtx_);
  return deq_.back();
}

template <class T> void BlockDeque<T>::push_back(const T &item) {
  std::unique_lock<std::mutex> locker(mtx_);
  while (deq_.size() >= capacity_) {
    cond_producer_.wait(locker);
  }
  deq_.push_back(item);
  cond_consumer_.notify_one();
}

template <class T> void BlockDeque<T>::push_front(const T &item) {
  std::unique_lock<std::mutex> locker(mtx_);
  while (deq_.size() >= capacity_) {
    cond_producer_.wait(locker);
  }
  deq_.push_front(item);
  cond_consumer_.notify_one();
}

template <class T> bool BlockDeque<T>::pop(T &item) {
  std::unique_lock<std::mutex> locker(mtx_);
  while (deq_.empty()) {
    cond_consumer_.wait(locker);
    if (is_close_) {
      return false;
    }
  }
  item = deq_.front();
  deq_.pop_front();
  cond_producer_.notify_one();
  return true;
}

template <class T> bool BlockDeque<T>::pop(T &item, int timeout) {
  std::unique_lock<std::mutex> locker(mtx_);
  while (deq_.empty()) {
    if (cond_consumer_.wait_for(locker, std::chrono::seconds(timeout)) ==
        std::cv_status::timeout) {
      return false;
    }
    if (is_close_) {
      return false;
    }
  }
  item = deq_.front();
  deq_.pop_front();
  cond_producer_.notify_one();
  return true;
}

template <class T> void BlockDeque<T>::flush() { cond_consumer_.notify_one(); }

#endif // BLOCKQUEUE_H
