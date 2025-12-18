#pragma once

#include "package.hpp"
#include <iostream>
#include <list>

enum class PackageQueueType {
  FIFO,
  LIFO
};

class IPackageStockPile {
  public:

    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package&& other) = 0;
    virtual bool empty() const = 0;
    virtual std::size_t size() const = 0;
    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator cend() const = 0;
    virtual ~IPackageStockPile() = default;
  };

class IPackageQueue : public IPackageStockPile {
  public:
    virtual Package pop() = 0;
    virtual PackageQueueType get_queue_type() const = 0;
    ~IPackageQueue() override = default;
};

class PackageQueue : public IPackageQueue {
  public:
    PackageQueue(PackageQueueType queue_type);
    Package pop() override;
    PackageQueueType get_queue_type() const override;
    void push(Package&& other) override;
    bool empty() const override;
    std::size_t size() const override;
    const_iterator begin() const override;
    const_iterator end() const override;
    const_iterator cbegin() const override;
    const_iterator cend() const override;
    ~PackageQueue() override = default;
  private:
    PackageQueueType queue_type_;
    std::list<Package> queue_;
};