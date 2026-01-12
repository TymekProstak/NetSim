#pragma once

#include "factory.hpp"

#include <functional>
#include <set>
#include <stdexcept>

class IntervalReportNotifier {
public:
    explicit IntervalReportNotifier(TimeOffset report_interval) : report_interval_(report_interval) {}

    bool should_generate_report(Time t) const {
        return report_interval_ != 0 && (t % report_interval_ == 0);
    }

private:
    TimeOffset report_interval_;
};

class SpecificTurnsReportNotifier {
public:
    explicit SpecificTurnsReportNotifier(std::set<Time> report_turns) : report_turns_(std::move(report_turns)) {}

    bool should_generate_report(Time t) const {
        return report_turns_.count(t) > 0;
    }

private:
    std::set<Time> report_turns_;
};

inline void simulate(Factory& factory,
                     TimeOffset d,
                     const std::function<void(Factory&, TimeOffset)>& rf) {
    if (!factory.is_consistent()) {
        throw std::logic_error("Non-consistent factory");
    }

    for (Time t = 1; t <= d; ++t) {
        factory.do_deliveries(t);
        factory.do_package_passing();
        factory.do_work(t);
        rf(factory, t);
    }
}