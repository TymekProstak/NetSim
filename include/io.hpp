#include "types.hpp"

#include <istream>
#include <string>
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

class Factory;

enum class ElementType {
    RAMP,
    WORKER,
    STOREHOUSE,
    LINK
};

const std::map<ElementType, std::string> ElementTypeTags = {
    {ElementType::RAMP, "LOADING_RAMP"},
    {ElementType::WORKER, "WORKER"},
    {ElementType::STOREHOUSE, "STOREHOUSE"},
    {ElementType::LINK, "LINK"}
};

// Definicje typów węzłów do dekodowania
const std::string NODE_TYPE_RAMP = "ramp";
const std::string NODE_TYPE_WORKER = "worker";
const std::string NODE_TYPE_STOREHOUSE = "store";

// Definicje typów węzłów do raportów
const std::string REPORT_NODE_WORKER = "worker";
const std::string REPORT_NODE_STOREHOUSE = "storehouse";

std::map<std::string, std::string> parse_line(const std::string& line);

std::pair<std::string, int> decode_node_id(const std::string& raw_id);

Factory load_factory_structure(std::istream& is);
void generate_structure_report(const Factory& f, std::ostream& os);
void generate_simulation_report(const Factory& f, std::ostream& os, Time turn);