#include <fstream>
#include <iostream>
#include <set>

#include "simulate.hpp"
#include "factory.hpp"
#include "io.hpp"

int main() {
    std::ifstream input_file("load_factory.txt");
    if (!input_file.is_open()) {
        std::cerr << "Failed to open load_factory.txt (cwd matters)\n";
        return 1;
    }

    Factory factory = load_factory_structure(input_file);

    std::cout << "=== STRUCTURE AFTER LOAD ===\n";
    generate_structure_report(factory, std::cout);

    IntervalReportNotifier notifier(1);
    simulate(factory, 10, [&notifier](Factory& f, TimeOffset t) {
        if (notifier.should_generate_report(t)) {
            std::cout << "\n=== SIMULATION REPORT AT TIME " << t << " ===\n";
            generate_simulation_report(f, std::cout,t);
        }
    });

    return 0;
}