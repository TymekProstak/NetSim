#include #include #include
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

{
std::ofstream clear("sim_report.txt", std::ios::trunc);
if (!clear.is_open()) {
std::cerr << "Failed to create/clear sim_report.txt\n";
return 1;
}
}

IntervalReportNotifier notifier(1);
simulate(factory, 3, [&notifier](Factory& f, TimeOffset t) {
if (notifier.should_generate_report(t)) {
std::cout << "\n=== SIMULATION REPORT AT TIME " << t << " ===\n";
generate_simulation_report(f, std::cout, t);

std::ofstream out("sim_report.txt", std::ios::app);
if (!out.is_open()) {
std::cerr << "Failed to open sim_report.txt for append\n";
return;
}

//out << "\n=== SIMULATION REPORT AT TIME " << t << " ===\n";
generate_simulation_report(f, out, t);

out.close();
}
});

return 0;
}
