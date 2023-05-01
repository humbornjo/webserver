#include "heaptimer.h"

void HeapTimer::adjust(int id, int new_expires) {
    assert(ref_.find(id) != ref_.end());
    heap_[ref_[id]].expires = Clock::now() + MS(new_expires);
    if (!shiftdown_(ref_[id], heap_.size())) {
        shiftup_(ref_[id]);
    }
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    if (ref_.find(id) == ref_.end()) {
        size_t i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        shiftup_(i);
    } else {
        i = ref_[id];
        heap_[i].cb = cb;
        heap_[i].expires = Clock::now() + MS(timeout);
        if (!shiftdown_(i, heap_.size())) {
            shiftup_(i);
        }
    }
}

void HeapTimer::DoWork(int id) {
    if (ref_.find(id) != ref_.end()) {
        size_t i = ref_[id];
        TimerNode node = heap_[i];
        node.cb();
        del_(i);
    }
}

void HeapTimer::clear() {
    heap_.clear();
    ref_.clear();
}

void HeapTimer::tick() {
    if (heap_.empty()) {
        return;
    }
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

int HeapTimer::GetNextTick() {
    tick();
    size_t res = 0;
    if (heap_.empty()) {
        int64_t tmp_time = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (tmp_time>=0) {
            res = tmp_time;
        }
    }
    return res;
}

void HeapTimer::del_(size_t i) {
    assert(!heap_.empty() && i >= 0 && i < heap_.size());
    size_t n = heap_.size()-1;
    if (i<n) {
        SwapNode_(i, n);
        if (!shiftdown_(i, n)) {
            shiftup_(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::shiftup_(size_t i) {
    assert(i>=0 && i<heap_.size());
    size_t parent = (i-1) >> 1;
    while (parent >= 0) {
        if (heap_[parent] < heap_[i]){
            break;
        }
        SwapNode_(parent, i);
        i = parent;
        parent = (i-1)>>1;
    }
}

bool HeapTimer::shiftdown_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i_copy = i;
    size_t child = i*2 + 1;
    while (child < n) {
        if (child + 1 < n && heap_[child+1] < heap_[child]){
            child+=1;
        }
        if (heap_[i] < heap_[child]) {
            break;
        }
        SwapNode_(child, i);
        i = child;
        child = i*2 + 1;
    }
    return i_copy < i;
}

void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}