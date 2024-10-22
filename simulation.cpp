#include <iostream>
#include "dsm/dsm.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include <thread>
#include <atomic>

void printMessage() {
    std::cout << "Hello dio stronzaccione!" << std::endl;
}

// uncomment these lines to print densities, flows and speeds
#define PRINT_DENSITIES
// #define PRINT_FLOWS
#define PRINT_OUT_SPIRES
// #define PRINT_SPEEDS
// #define PRINT_TP
#define OPTIMIZE

// Compatible with dsm 1.2.1
constexpr double ERROR_PROBABILITY{0.3}; // seed for random number generator
constexpr int SEED{69}; // seed for random number generator

const std::string IN_NODES{"./osm_nodes.csv"}; // input
const std::string IN_EDGES{"./osm_edges.csv"}; // input




const std::string OUT_FOLDER{"output_simulation" + std::to_string(SEED) + "_op/"}; // output folder
constexpr auto MAX_TIME{
    static_cast<unsigned int>(1e6)}; // maximum time of simulation

using Unit = unsigned int;
using Delay = uint8_t;

using Graph = dsm::Graph<Unit, Unit>;
using Itinerary = dsm::Itinerary<Unit>;
using Dynamics = dsm::FirstOrderDynamics<Unit, Unit, Delay>;
using Street = dsm::Street<Unit, Unit>;
using SpireStreet = dsm::SpireStreet<Unit, Unit>;
using TrafficLight = dsm::TrafficLight<Unit, Unit, Delay>;





int main() {

    printMessage();


    // import road network from OSM file

    std::cout << "Using dsm version: " << dsm::version() << '\n';
    Graph graph{};
    std::cout << "Importing osm...\n";
    graph.importOSMNodes(IN_NODES);
    graph.importOSMEdges(IN_EDGES);
    graph.buildAdj();
    const auto dv = graph.adjMatrix().getDegreeVector();

    std::cout << "Number of nodes: " << graph.nodeSet().size() << '\n';
    std::cout << "Number of streets: " << graph.streetSet().size() << '\n';

    for (const auto &[streetId, street] : graph.streetSet()) {
    std::cout << "Street ID: " << streetId << ", lanes=" << static_cast<int>(street->nLanes()) << ", len= " << static_cast<int>(street->length()) << ", capacity= " << static_cast<int>(street->capacity()) << std::endl;
    
    //here we need to assign the status "spira" to the streets that have a 
    
    }


    return 0;
}
