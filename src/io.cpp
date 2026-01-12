#include "io.hpp"
#include "factory.hpp"

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
