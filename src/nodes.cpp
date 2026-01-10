#include "nodes.hpp"

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    preferences_[r] = 0.0;
    double equal_prob_step = 1.0 / preferences_.size();
    double prob_sum = equal_prob_step;
    for (auto& [receiver, prob] : preferences_) {
        prob = prob_sum;
        prob_sum += equal_prob_step;
        }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    preferences_.erase(r);
    if (!preferences_.empty()) {
        double equal_prob_step = 1.0 / preferences_.size();
        double prob_sum = equal_prob_step;
        for (auto& [receiver, prob] : preferences_) {
            prob = prob_sum;
            prob_sum += equal_prob_step;
        }
    }
}

IPackageReceiver* ReceiverPreferences::choose_receiver() const {
    if (preferences_.empty()) {
        return nullptr;
    }

    const double p = generate_probability_();
    if (p < 0.0 || p > 1.0) {
        return nullptr;
    }

    double prev_p = 0.0;
    for (const auto& [receiver, prob] : preferences_) {
        if (prev_p < p && p <= prob) {
            return receiver;
        }
        prev_p = prob;
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
        t_ = t;
        processing_buffer_.emplace(queue_->pop());
    }

    if (processing_buffer_.has_value() && (t - t_ + 1 == pd_)) {
        push_package(Package(processing_buffer_->get_id()));
        processing_buffer_.reset();
    }
}

void Ramp::deliver_goods(Time t) {
    if (buffer_.has_value()) {
        return;
    }

    if  ((t - 1) % delivery_interval_ == 0){
        push_package(Package());
    }
}