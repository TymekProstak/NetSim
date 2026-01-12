#include <iostream>
#include <fstream>
#include <string>
#include "factory.hpp"
#include "io.hpp"


int main() {
    std::ifstream input_file("load_factory.txt");

    if (!input_file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc pliku: load_factory.txt" << std::endl;
        return 1;
    }

    Factory factory = load_factory_structure(input_file);
    std::cout << "=== TEST 1: RAPORT STRUKTURY ===" << std::endl;
    generate_structure_report(factory, std::cout);
    std::cout << "\n" << std::string(30, '-') << "\n" << std::endl;


    if (!factory.is_consistent()) {
        std::cerr << "Blad: Fabryka nie jest spojna!" << std::endl;
        return 1;
    }

    std::cout << "=== TEST 2: RAPORT STANU (Tura 1) ===" << std::endl;
    
    factory.do_deliveries(1);
    
    factory.do_package_passing();
    
    factory.do_work(1);

    generate_simulation_report(factory, std::cout, 1);

    std::cout << "\n=== TEST 3: RAPORT STANU (Tura 2) ===" << std::endl;
    
    factory.do_deliveries(2);
    factory.do_package_passing();
    factory.do_work(2); 

    generate_simulation_report(factory, std::cout, 2);

    std::cout << "\n=== TEST 4 : RAPORT STANU (Tura 3) ===" << std::endl;
    factory.do_deliveries(3);
    factory.do_package_passing();
    factory.do_work(3); 
 
    generate_simulation_report(factory, std::cout, 3);
 
    return 0;
}