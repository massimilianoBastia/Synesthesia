#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>

class Queue {

public:

    /*
        Add an item to the queue.
        This locks the queue to ensure thread safety, pushes the item, and notifies any waiting threads.
    */
    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.push(item);
        cv_.notify_one();
    }

    /*
        Try to remove an item from the queue.
        If the queue is empty, it simply returns false without blocking.
    */
    bool try_pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        if (queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    /*
        Check if the queue has been marked as done.
        This means no more items will be added.
    */
    bool has_done() const {
        std::unique_lock<std::mutex> lock(mtx_);
        return done_;
    }

    /*
        Mark the queue as done.
        This notifies all waiting threads that no more items will be added.
    */
    void set_done() {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            done_ = true;
        }
        cv_.notify_all();
    }

private:

    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    bool done_ = false;

    T pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.empty();
    }

};