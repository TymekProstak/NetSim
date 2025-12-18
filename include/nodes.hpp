#pragma once

#include "storage_types.hpp"

class IPackageReceiver{
    public:
        using const_iterator = IPackageStockPile::const_iterator;
        
        virtual void receive_package(Package&& p) = 0;
        virtual ElementID get_id() const = 0;

        virtual const_iterator begin() const = 0;
        virtual const_iterator end() const = 0;
        virtual const_iterator cbegin() const = 0;
        virtual const_iterator cend() const = 0;

        virtual ~IPackageReceiver() = default;
    };
