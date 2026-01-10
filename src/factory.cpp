#include "factory.hpp"

bool Factory::is_consistent() const{
    std::map<const PackageSender*, NodeColor> colors;
    for (const auto& ramp : ramps_) {
        colors[&ramp] = NodeColor::NOT_VISITED;
    }
    for (const auto& worker : workers_) {
        colors[&worker] = NodeColor::NOT_VISITED;
    }
    try{
        for (const auto& ramp: ramps_){
            has_reachable_storehouse(&ramp, colors);
        }
    }
    catch (const std::logic_error&){
        return false;
    }
    return true;
}

bool Factory::has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& colors) const{
    if (colors[sender] == NodeColor::VERIFIED) {
        return true;
    }
    colors[sender] = NodeColor::VISITED;

    if (sender->receiver_preferences_.get_preferences().empty()){
        throw std::logic_error("No receivers connected");
    }

    bool has_other_receiver = false;

    for (const auto& [receiver, prob] : sender->receiver_preferences_.get_preferences()) {
        if (receiver->get_receiver_type() == ReceiverType::STOREHOUSE) {
            has_other_receiver = true;
            continue;
        }
        else if (receiver->get_receiver_type() == ReceiverType::WORKER){
            IPackageReceiver* receiver_ptr = receiver;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto PackageSender_ptr = dynamic_cast<PackageSender*>(worker_ptr);
            if (PackageSender_ptr == sender) {
                continue;
            }
            
            if (colors[PackageSender_ptr] == NodeColor::NOT_VISITED) {
                has_reachable_storehouse(PackageSender_ptr, colors);
                has_other_receiver = true;
            }
            else if (colors[PackageSender_ptr] == NodeColor::VERIFIED) {
                has_other_receiver = true;
            }
            
        }
    }
    colors[sender] = NodeColor::VERIFIED;
    if (has_other_receiver) {
        return true;
    }
    else{
        throw std::logic_error("No receivers connected");
    }
}