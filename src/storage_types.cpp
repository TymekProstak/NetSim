#include "storage_types.hpp"

PackageQueue::PackageQueue(PackageQueueType queue_type) : queue_type_(queue_type), queue_() {};

Package PackageQueue::pop() {
  Package p;
  if(queue_.empty()) {
    throw std::out_of_range("The queue is empty");
  }
  if(queue_type_ == PackageQueueType::FIFO) {
    Package temp = std::move(queue_.front());
    queue_.pop_front();
    return temp;
  }
  if(queue_type_ == PackageQueueType::LIFO) {
    Package temp = std::move(queue_.back());
    queue_.pop_back();
    return temp;
  }
  return p;
}

PackageQueueType PackageQueue::get_queue_type() const {
  return queue_type_;
}

void PackageQueue::push(Package&& other) {
  queue_.push_back(std::move(other));
}

bool PackageQueue::empty() const {
  return queue_.empty();
}

std::size_t PackageQueue::size() const {
  return queue_.size();
}

std::list<Package>::const_iterator PackageQueue::begin() const {
  return queue_.begin();
}

std::list<Package>::const_iterator PackageQueue::end() const {
  return queue_.end();
}

std::list<Package>::const_iterator PackageQueue::cbegin() const {
  return queue_.cbegin();
}

std::list<Package>::const_iterator PackageQueue::cend() const {
  return queue_.cend();
}