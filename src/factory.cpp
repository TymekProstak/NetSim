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
                else if(dest_data.first == NODE_TYPE_STOREHOUSE) {
                    auto storehouse_it = factory.find_storehouse_by_id(dest_data.second);
                    ramp_it->receiver_preferences_.add_receiver(&(*storehouse_it));
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

std::map<std::string, std::string> parse_line(const std::string& line) {
    std::map<std::string, std::string> result;
    std::stringstream ss(line);
    std::string tag, part;
    ss >> tag;
    while (ss >> part) {
        size_t pos = part.find('=');
        if (pos != std::string::npos) {
            result[part.substr(0,pos)] = part.substr(pos+1);
        }
    }
    return result;
}

std::pair<std::string, int> decode_node_id(const std::string& raw_id) {
    size_t dash_pos = raw_id.find('-');
    std::string type = raw_id.substr(0, dash_pos);
    int id = std::stoi(raw_id.substr(dash_pos + 1));
    return {type, id};
}


void generate_structure_report(const Factory& f, std::ostream& os) {
    os << "\n== LOADING RAMPS ==\n\n";
    for(auto it = f.ramp_cbegin(); it != f.ramp_cend(); ++it) {
        os << "LOADING RAMP #" << it->get_id() << "\n";
        os << "  Delivery interval: " << it->get_delivery_interval() << "\n";
        os << "  Receivers:\n";
        
        const auto& prefs = it->receiver_preferences_.get_preferences();

        std::vector<std::pair<std::string, ElementID>> sorted_receivers;
        for(auto const& [receiver, probability] : prefs) {
            if(dynamic_cast<const Storehouse*>(receiver)) {
                sorted_receivers.push_back({REPORT_NODE_STOREHOUSE, receiver->get_id()});
            }
            else if(dynamic_cast<const Worker*>(receiver)) {
                sorted_receivers.push_back({REPORT_NODE_WORKER, receiver->get_id()});
            }
        }

        std::sort(sorted_receivers.begin(), sorted_receivers.end(),
                  [](const auto& a, const auto& b) {
                      if(a.first != b.first) {
                          return a.first < b.first;
                      }
                      return a.second < b.second;
                  });
        
        for (const auto& receiver : sorted_receivers) {
            os << "    " << receiver.first << " #" << receiver.second << "\n";
        }
        os << "\n";
    }
    
    os << "\n== WORKERS ==\n\n";
    for(auto it = f.worker_cbegin(); it != f.worker_cend(); ++it) {
        PackageQueueType qt = it->get_queue()->get_queue_type();
        std::string sqt;
        if(qt == PackageQueueType::FIFO) {
            sqt = "FIFO";
        }
        else if(qt == PackageQueueType::LIFO) {
            sqt = "LIFO";
        }
        else {
            throw std::logic_error("Invalid queue type");
        }
        os << "WORKER #" << it->get_id() << "\n";
        os << "  Processing time: " << it->get_processing_duration() << "\n";
        os << "  Queue type: " << sqt << "\n";
        os << "  Receivers:\n";
        const auto& prefs = it->receiver_preferences_.get_preferences();

        std::vector<std::pair<std::string, ElementID>> sorted_receivers;
        for(auto const& [receiver, probability] : prefs) {
            if(dynamic_cast<const Storehouse*>(receiver)) {
                sorted_receivers.push_back({REPORT_NODE_STOREHOUSE, receiver->get_id()});
            }
            else if(dynamic_cast<const Worker*>(receiver)) {
                sorted_receivers.push_back({REPORT_NODE_WORKER, receiver->get_id()});
            }
        }

        std::sort(sorted_receivers.begin(), sorted_receivers.end(),
                  [](const auto& a, const auto& b) {
                      if(a.first != b.first) {
                          return a.first < b.first;
                      }
                      return a.second < b.second;
                  });
        
        for (const auto& receiver : sorted_receivers) {
            os << "    " << receiver.first << " #" << receiver.second << "\n";
        }
        os << "\n";
    }

    os << "\n== STOREHOUSES ==\n\n";
    for(auto it = f.storehouse_cbegin(); it != f.storehouse_cend(); ++it) {
        os << "STOREHOUSE #" << it->get_id() << "\n";
    }
}

void generate_simulation_report(const Factory& f, std::ostream& os, Time turn) {
    os << "=== [ Turn: " << turn << " ] ===\n";
    //WORKERS
    os << "\n== WORKERS == \n";

    std::vector<const Worker*> sorted_workers;
    for(auto it = f.worker_cbegin(); it != f.worker_cend(); ++it) {
        sorted_workers.push_back(&(*it));
    }
    std::sort(sorted_workers.begin(), sorted_workers.end(), [](const Worker* a, const Worker* b) {
        return a->get_id() < b->get_id();
    });

    for(const auto& worker : sorted_workers) {
        os << "WORKER #" << worker->get_id() << "\n";
        os << "  PBuffer: ";
        if(worker->get_processing_buffer()) {
            Time pt = turn - worker->get_package_processing_start_time() + 1;
            os << "#" << worker->get_processing_buffer()->get_id() << " (pt = " << pt << ")\n"; 
        } else {
            os << "(empty)\n";
        }
        
        os << "  Queue: ";
        if(worker->get_queue()->empty()) {
            os << "(empty)\n";
        } else {
            for (auto it = worker ->get_queue()->cbegin(); it != worker->get_queue()->cend(); ++it) {
                os << "#" << it->get_id() << (std::next(it) == worker->get_queue()->cend() ? "\n" : ", ");
            }
        }
        
        os << "  SBuffer: ";
        if(worker->get_sending_buffer().has_value()) {
            os << "#" << worker->get_sending_buffer()->get_id() << "\n";
        } else {
            os << "(empty)\n";
        }
        os << "\n";
    }

    os << "\n== STOREHOUSES == \n\n";
    for(auto it = f.storehouse_cbegin(); it != f.storehouse_cend(); ++it) {
        os << "STOREHOUSE #" << it->get_id() << "\n";
        os << "  Stock: ";
        if(it->cbegin() == it->cend()) {
            os << "(empty)\n";
        } else {
            for (auto pkg_it = it ->cbegin(); pkg_it != it->cend(); ++pkg_it) {
                os << "#" << pkg_it->get_id() << (std::next(pkg_it) == it->cend() ? "\n" : ", ");
            }
        }
    }
    os << "\n";
}

void link_fill(std::stringstream& link_stream, const PackageSender& package_sender, ElementID package_sender_id, std::string package_sender_type){
    const auto& prefs = package_sender.receiver_preferences_.get_preferences();

    std::for_each(prefs.begin(), prefs.end(), [&](const auto& key_value){
        std::string dest_type = (key_value.first->get_receiver_type() == ReceiverType::WORKER ? "worker" : "store");
        
        link_stream << "LINK src=" << package_sender_type << '-' << package_sender_id 
                    << " dest=" << dest_type << '-' << key_value.first->get_id() << '\n';
    });
}

void save_factory_structure(const Factory& factory, std::ostream& os){
    std::stringstream link_stream;

    //Zapis RAMP (LOADING_RAMP)
    std::for_each(factory.ramp_cbegin(), factory.ramp_cend(), [&](const Ramp& ramp) {
        os << "LOADING_RAMP id=" << ramp.get_id() << " delivery-interval=" << ramp.get_delivery_interval() << '\n';
        link_fill(link_stream, ramp, ramp.get_id(), "ramp");
    });

    //Zapis WORKER 
    std::for_each(factory.worker_cbegin(), factory.worker_cend(), [&](const Worker& worker) {
        PackageQueueType queue_type = worker.get_queue()->get_queue_type();
        std::string queue_type_str = (queue_type == PackageQueueType::FIFO) ? "FIFO" : "LIFO";
        
        os << "WORKER id=" << worker.get_id() 
           << " processing-time=" << worker.get_processing_duration() 
           << " queue-type=" << queue_type_str << '\n';
           
        link_fill(link_stream, worker, worker.get_id(), "worker");
    });

    //Zapis STOREHOUSE
    std::for_each(factory.storehouse_cbegin(), factory.storehouse_cend(), [&](const Storehouse& storehouse) {
        os << "STOREHOUSE id=" << storehouse.get_id() << '\n';
    });

    //Zapis LINKÓW
    os << link_stream.str();

    os.flush();
}