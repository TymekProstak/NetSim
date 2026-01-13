#include "nodes.hpp"

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    preferences_[r] = 0.0;
    double equal_prob = 1.0 / preferences_.size();

    for (auto& [receiver, prob] : preferences_) {
        prob = equal_prob;
    }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    preferences_.erase(r);
    if (preferences_.empty()) return;

    double equal_prob = 1.0 / preferences_.size();
    for (auto& [receiver, prob] : preferences_) {
        prob = equal_prob;
    }
}

IPackageReceiver* ReceiverPreferences::choose_receiver() const {
    if (preferences_.empty()) return nullptr;

    const double p = generate_probability_();
    
    double distribution = 0.0;
    for (const auto& [receiver, prob] : preferences_) {
        distribution += prob;
        if (p <= distribution) {
            return receiver;
        }
    }
    return preferences_.rbegin()->first;
}

void PackageSender::send_package() {
    if (!buffer_) {
        return;
    }
    IPackageReceiver* receiver = receiver_preferences_.choose_receiver();
    if (receiver == nullptr) {
        return;
    }
    receiver->receive_package(std::move(buffer_.value()));
    buffer_ = std::nullopt;
}

void Worker::do_work(Time t) {
    if (!processing_buffer_.has_value() && !queue_->empty()) {
        processing_buffer_.emplace(queue_->pop()); 
        t_ = t; 
    }

    if (processing_buffer_.has_value()) {
        if (t - t_ + 1 == pd_) {
            push_package(std::move(*processing_buffer_));
            
            processing_buffer_.reset();
        }
    }
}

void Ramp::deliver_goods(Time t) {
    if (buffer_.has_value()) {
        return;
    }

    // if  (t  % delivery_interval_ == 0  || t == 1){
    //     push_package(Package());
    //     std::cout << "Ramp " << id_ << " delivered package " << buffer_->get_id() << " at time " << t << "\n";
    // }
      if  ((t-1)  % delivery_interval_ == 0){
         push_package(Package());
        
     }
}

Storehouse::Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d)
    : id_(id), d_(std::move(d)) {}