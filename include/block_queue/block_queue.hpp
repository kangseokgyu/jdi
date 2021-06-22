#ifndef _JDI_BLOCK_QUEUE_HPP_
#define _JDI_BLOCK_QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <class T> //
class block_queue {
private:
  std::queue<T> q_;
  std::mutex q_lock_;

  std::condition_variable q_cond_;

  bool clearing_ = false;

public:
  block_queue() {}
  ~block_queue() {
    clearing_ = true;
    q_cond_.notify_all();
  }

  size_t size() {
    std::unique_lock<std::mutex> guard(q_lock_);
    return q_.size();
  }

  void push(T e) {
    {
      std::unique_lock<std::mutex> guard(q_lock_);
      q_.push(e);
    }
    q_cond_.notify_one();
  }

  std::optional<T> pop_block() {
    T r;
    {
      std::unique_lock<std::mutex> guard(q_lock_);
      q_cond_.wait(guard,
                   [this]() { return !this->q_.empty() || this->clearing_; });
      if (this->q_.empty() && this->clearing_)
        return std::nullopt;

      r = q_.front();
      q_.pop();
    }
    return std::make_optional(r);
  }
};

#endif /* _JDI_BLOCK_QUEUE_HPP_ */
