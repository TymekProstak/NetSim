#include "factory.hpp"
#include "io.hpp"

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

Factory load_factory_structure(std::istream& is) {
    Factory factory;
    std::string line;
    while(std::getline(is, line)) {
        if(line.empty() || line[0] == ';') {
            continue; // Pomijamy puste linie i komentarze
        }
        if(line.find(ElementTypeTags.at(ElementType::RAMP)) == 0) {
            auto params = parse_line(line);
            factory.add_ramp(Ramp(std::stoi(params["id"]), std::stoi(params["delivery-interval"])));    
        }
        else if(line.find(ElementTypeTags.at(ElementType::WORKER)) == 0) {
            auto params = parse_line(line);
            ElementID id = std::stoi(params["id"]);
            TimeOffset pd = std::stoi(params["processing-time"]);
            PackageQueueType qt = (params["queue-type"] == "FIFO") ? PackageQueueType::FIFO : PackageQueueType::LIFO;
            factory.add_worker(Worker(id, pd, std::make_unique<PackageQueue>(qt)));
        }
        else if(line.find(ElementTypeTags.at(ElementType::STOREHOUSE)) == 0) {
            auto params = parse_line(line);
            ElementID id = std::stoi(params["id"]);
            factory.add_storehouse(Storehouse(id));
        }
        else if(line.find(ElementTypeTags.at(ElementType::LINK)) == 0) {
            auto params = parse_line(line);
            auto src_data = decode_node_id(params["src"]);
            auto dest_data = decode_node_id(params["dest"]);
            if(src_data.first == NODE_TYPE_RAMP) {
                auto ramp_it = factory.find_ramp_by_id(src_data.second);
                if(dest_data.first == NODE_TYPE_WORKER) {
                    auto worker_it = factory.find_worker_by_id(dest_data.second);
                    ramp_it->receiver_preferences_.add_receiver(&(*worker_it));
                }
                else {
                    throw std::logic_error("Invalid LINK destination for RAMP");
                }
            }
            if(src_data.first == NODE_TYPE_WORKER) {
                auto worker_it = factory.find_worker_by_id(src_data.second);
                if(dest_data.first == NODE_TYPE_WORKER) {
                    auto dest_worker_it = factory.find_worker_by_id(dest_data.second);
                    worker_it->receiver_preferences_.add_receiver(&(*dest_worker_it));
                }
                else if(dest_data.first == NODE_TYPE_STOREHOUSE) {
                    auto sh_it = factory.find_storehouse_by_id(dest_data.second);
                    worker_it->receiver_preferences_.add_receiver(&(*sh_it));
                }
                else {
                    throw std::logic_error("Invalid LINK destination for WORKER");
                }
            }
        }
        else {
            throw std::logic_error("Invalid structure");
        }
    }
    return factory;
}