#pragma once

#include "types.hpp"
#include <utility>
#include <set>

class Package {
public:
    Package(ElementID id) : id_(id) {};

    Package() {
        if (freed_IDs.empty()) {
            if (assigned_IDs.empty()) {
                id_ = 1;
                assigned_IDs = {1};
            } else {
                id_ = *assigned_IDs.rbegin() + 1;
                assigned_IDs.emplace(id_);
            }
        } else {
            id_ = *freed_IDs.begin();
            assigned_IDs.emplace(id_);
            freed_IDs.erase(freed_IDs.find(id_));
        }
    }

    Package(Package&&) = default;

    inline Package& operator=(Package&& other) noexcept {
        if (this != &other) {
            id_ = std::move(other.id_);
        }
        return *this;
    }

    inline ElementID get_id() const {
        return id_;
    }

    ~Package(){
        assigned_IDs.erase(assigned_IDs.find(id_));
        freed_IDs.emplace(id_);
    }

private:
    ElementID id_;
    inline static std::set<ElementID> assigned_IDs;
    inline static std::set<ElementID> freed_IDs; 
};