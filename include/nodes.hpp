#pragma once
#include "storage_types.hpp"
#include "types.hpp"
#include "helpers.hpp"
#include "package.hpp"

#include <map>
#include <utility>
#include <functional>
#include <algorithm>
#include <memory>
#include <optional>

enum class ReceiverType {
    STOREHOUSE,
    WORKER
};


class IPackageReceiver {
    public:
        virtual void receive_package(Package&& p) = 0;
        virtual ElementID get_id() const = 0;
        virtual IPackageStockpile::const_iterator cbegin() const = 0;
        virtual IPackageStockpile::const_iterator cend() const = 0;
        virtual IPackageStockpile::const_iterator begin() const = 0;
        virtual IPackageStockpile::const_iterator end() const = 0;

        virtual ReceiverType get_receiver_type() const { return ReceiverType::WORKER; }

        virtual ~IPackageReceiver() = default;
    };
  


class ReceiverPreferences {
    public:
        using preferences_t = std::map<IPackageReceiver*, double>;
        using const_iterator = preferences_t::const_iterator;

        ReceiverPreferences(ProbabilityGenerator pg = probability_generator) : preferences_(), generate_probability_(std::move(pg)) {}

        void add_receiver(IPackageReceiver* r);
        void remove_receiver(IPackageReceiver* r);

        IPackageReceiver* choose_receiver() const;

        const preferences_t& get_preferences() const { return preferences_; }

        const_iterator cbegin() const  { return preferences_.cbegin(); }
        const_iterator cend() const  { return preferences_.cend(); }
        const_iterator begin() const  { return preferences_.begin(); }
        const_iterator end() const  { return preferences_.end(); }

    private:
        preferences_t preferences_;
        ProbabilityGenerator generate_probability_;
};


class Storehouse : public IPackageReceiver {
  public:
    Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d = std::make_unique<PackageQueue>(PackageQueueType::FIFO))
        : id_(id), d_(std::move(d)) {}
    void receive_package(Package&& p) override {
        d_->push(std::move(p));
    }
    ElementID get_id() const override {
        return id_;
    }
    IPackageStockpile::const_iterator cbegin() const override {
        return d_->cbegin();
    }
    IPackageStockpile::const_iterator cend() const override {
        return d_->cend();
    }
    IPackageStockpile::const_iterator begin() const override {
        return d_->begin();
    }
    IPackageStockpile::const_iterator end() const override {
        return d_->end();
    }
    
    ReceiverType get_receiver_type() const override {
        return ReceiverType::STOREHOUSE;
    }

    ~Storehouse() override = default;

    Storehouse(Storehouse&&) = default;


  private:
    ElementID id_;
    std::unique_ptr<IPackageStockpile> d_;
};


class PackageSender {
    public:
        PackageSender() = default;
    
        PackageSender(PackageSender&& p) = default;
        
        void send_package();
    
        const std::optional<Package>& get_sending_buffer() const { return buffer_; };
    
        ReceiverPreferences receiver_preferences_;
    protected:
        void push_package(Package&& p) { buffer_ = std::move(p); }
    
        std::optional<Package> buffer_ = std::nullopt;
    };

class Ramp : public PackageSender {
    public:
        Ramp(ElementID id, TimeOffset di)
            : id_(id), delivery_interval_(di) {}
    
        ElementID get_id() const { return id_; }
        TimeOffset get_delivery_interval() const { return delivery_interval_; }

        void deliver_goods(Time t);
    
        ~Ramp() = default;

        Ramp(Ramp&&) = default;
    
    private:
        ElementID id_;
        TimeOffset delivery_interval_;
    };

class Worker : public PackageSender, public IPackageReceiver {
    public:
        Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q)
            : id_(id), pd_(pd), queue_(std::move(q)) {}

        void receive_package(Package&& p) override {
            queue_->push(std::move(p));
        }

        void do_work(Time t);

        ElementID get_id() const override {
            return id_;
        }
        Time get_package_processing_start_time() const {
            return t_;
        }
        TimeOffset get_processing_duration() const {
            return pd_;
        }
        IPackageStockpile::const_iterator cbegin() const override {
            return queue_->cbegin();
        }   
        IPackageStockpile::const_iterator cend() const override {
            return queue_->cend();
        }
        IPackageStockpile::const_iterator begin() const override {
            return queue_->begin();
        }
        IPackageStockpile::const_iterator end() const override {
            return queue_->end();
        }

        ReceiverType get_receiver_type() const override {
            return ReceiverType::WORKER;
        }

        IPackageQueue* get_queue() const {
            return queue_.get();
        }

        const std::optional<Package>& get_processing_buffer() const {
            return processing_buffer_;
        }

        Worker(Worker&&) = default;

        private:
        ElementID id_;
        TimeOffset pd_;
        Time t_;
        std::unique_ptr<IPackageQueue> queue_;
        std::optional<Package> processing_buffer_ = std::nullopt;

    };

