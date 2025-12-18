#pragma once

#include "types.hpp"
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

#endif