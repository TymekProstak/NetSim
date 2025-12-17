#pragma once
#include "types.hxx"
#include <iostream>
#include<algorithm>

class Package {
public:
    Package();
    Package(ElementID id) : id_(id) {};
    Package(Package&&);
    inline Package& operator=(Package&& other) noexcept {
        if(this != &other) {
            id_ = std::move(other.id_);
        }
        return *this;
    }
    inline ElementID get_id() const {
        return id_;
    }
    ~Package() = default;
private:
    ElementID id_;
};
