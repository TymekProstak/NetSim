#pragma once

#include "nodes.hpp"
#include "types.hpp"
#include <list>
#include <optional>

enum class NodeColor {NOT_VISITED, VISITED, VERIFIED};

template <typename Node>
class NodeCollection {
public:
    // Aliasy typów (żeby w Factory pisać NodeCollection<Ramp>::iterator)
    using container_t = std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    // Dodawanie (zauważ && - przenoszenie)
    void add(Node&& node) {
        nodes_.push_back(std::move(node));
    }

    // Wyszukiwanie (wersja do modyfikacji)
    iterator find_by_id(ElementID id) {
        return std::find_if(nodes_.begin(), nodes_.end(),
                            [id](const Node& node) {
                                return node.get_id() == id;
                            });
    }

    // Wyszukiwanie (wersja tylko do odczytu)
    const_iterator find_by_id(ElementID id) const {
        return std::find_if(nodes_.cbegin(), nodes_.cend(),
                            [id](const Node& node) {
                                return node.get_id() == id;
                            });
    }

    // Usuwanie po ID
    void remove_by_id(ElementID id) {
        auto it = find_by_id(id);
        if (it != nodes_.end()) {
            nodes_.erase(it);
        }
    }

    // Iteratory
    iterator begin() { return nodes_.begin(); }
    iterator end() { return nodes_.end(); }
    const_iterator begin() const { return nodes_.begin(); }
    const_iterator end() const { return nodes_.end(); }
    const_iterator cbegin() const { return nodes_.cbegin(); }
    const_iterator cend() const { return nodes_.cend(); }

private:
    container_t nodes_;
};

class Factory {
public:
// ---------------- RAMPY (Ramp) ----------------
    void add_ramp(Ramp&& r) {
        ramps_.add(std::move(r));
    }

    void remove_ramp(ElementID id) {
        ramps_.remove_by_id(id);
    }

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {
        return ramps_.find_by_id(id);
    }

    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {
        return ramps_.find_by_id(id);
    }

    NodeCollection<Ramp>::const_iterator ramp_cbegin() const {
        return ramps_.cbegin();
    }

    NodeCollection<Ramp>::const_iterator ramp_cend() const {
        return ramps_.cend();
    }

    // ---------------- ROBOTNICY (Worker) ----------------
    void add_worker(Worker&& w) {
        workers_.add(std::move(w));
    }

    void remove_worker(ElementID id) { remove_receiver(workers_, id); }

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {
        return workers_.find_by_id(id);
    }

    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {
        return workers_.find_by_id(id);
    }

    NodeCollection<Worker>::const_iterator worker_cbegin() const {
        return workers_.cbegin();
    }

    NodeCollection<Worker>::const_iterator worker_cend() const {
        return workers_.cend();
    }

    // ---------------- MAGAZYNY (Storehouse) ----------------
    void add_storehouse(Storehouse&& s) {
        storehouses_.add(std::move(s));
    }

    void remove_storehouse(ElementID id) { remove_receiver(storehouses_, id); }

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {
        return storehouses_.find_by_id(id);
    }

    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {
        return storehouses_.find_by_id(id);
    }

    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const {
        return storehouses_.cbegin();
    }

    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {
        return storehouses_.cend();
    }

    bool is_consistent() const;

    void do_deliveries(Time t){
        for (auto& ramp : ramps_) {
            ramp.deliver_goods(t);
        }
    }

    void do_package_passing(){
        for (auto& ramp : ramps_) {
            ramp.send_package();
        }

        for (auto& worker : workers_) {
            worker.send_package();
        }
    }

    void do_work(Time t){
        for (auto& worker : workers_) {
            worker.do_work(t);
        }
    }

    

private:

    template <typename Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id) {
        collection.remove_by_id(id);
        // DOROBIENIE USUWANIA Z PREFERENCJI INNYCH WĘZŁÓW
    }
    bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor>& colors) const;
    NodeCollection<Ramp> ramps_;
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;
};