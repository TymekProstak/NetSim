#pragma once

#include "types.hpp"
#include <utility>
#include <set>

class Package {
public:
    Package(ElementID id) : id_(id) {
        assigned_IDs.insert(id_);
        if (freed_IDs.count(id_)) {
            freed_IDs.erase(id_);
        }
    }

    Package() {
        if (freed_IDs.empty()) {
            if (assigned_IDs.empty()) {
                id_ = 1;
            } else {
                id_ = *assigned_IDs.rbegin() + 1;
            }
        } else {
            id_ = *freed_IDs.begin();
            freed_IDs.erase(id_);
        }
        assigned_IDs.insert(id_);
    }

    Package(Package&& other) noexcept {
        id_ = other.id_;
        other.id_ = -1;
    }

    Package& operator=(Package&& other) noexcept {
        if (this != &other) {

            if (id_ != -1) {
                assigned_IDs.erase(id_);
                freed_IDs.insert(id_);
            }
            
            id_ = other.id_;
            
            other.id_ = -1;
        }
        return *this;
    }

    Package(const Package&) = delete;
    Package& operator=(const Package&) = delete;

    ElementID get_id() const {
        return id_;
    }

    ~Package() {
 
        if (id_ != -1) { 
            assigned_IDs.erase(id_);
            freed_IDs.insert(id_);
        }
    }

private:
    ElementID id_ = -1;
    inline static std::set<ElementID> assigned_IDs;
    inline static std::set<ElementID> freed_IDs; 
};