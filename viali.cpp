#include "dsm/dsm.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <filesystem>

#include <thread>
#include <atomic>
namespace fs = std::filesystem;

std::atomic_int progress{0};

// uncomment these lines to print densities, flows and speeds
#define PRINT_DENSITIES
// #define PRINT_FLOWS
#define PRINT_OUT_SPIRES
// #define PRINT_SPEEDS
// #define PRINT_TP
#define OPTIMIZE


constexpr double ERROR_PROBABILITY{0.05}; 
constexpr int SEED{69}; // seed for random number generator
const std::string IN_COORDS{"./coordinates.dsm"}; // input coords file
const std::string OUT_FOLDER{"./datas" + std::to_string(SEED) + "_op/"}; // output folder

// Compatible with dsm 1.3.8

using Unit = unsigned int;
using Delay = uint8_t;

using Graph = dsm::Graph;
using Itinerary = dsm::Itinerary;
using Dynamics = dsm::FirstOrderDynamics<Delay>;
using Street = dsm::Street;
using SpireStreet = dsm::SpireStreet;
using TrafficLight = dsm::TrafficLight<Delay>;

void printLoadingBar(int const i, int const n) {
  std::cout << "Loading: " << std::setprecision(2) << std::fixed
            << (i * 100. / n) << "%" << '\r';
  std::cout.flush();
}

constexpr auto MAX_TIME{
    static_cast<unsigned int>(1e6)}; // maximum time of simulation





int main() {


  if (fs::exists(OUT_FOLDER)) {
    fs::remove_all(OUT_FOLDER);
  }
  fs::create_directory(OUT_FOLDER);
  std::cout << "Using dsm version: " << dsm::version() << '\n';

  // Create the graph

  TrafficLight tl1{1}, tl2{2}, tl3{3}, tl4{4}, tl5{5}, tl6{6}, tl7{7};


  // segmenti viali
  Street s0_1{1, 1,  480., 13.9, std::make_pair(0, 1), 3};  // (402) 2.10 2.6 6 1                              
  Street s1_0{2, 1,  480., 13.9, std::make_pair(1, 0), 3};  // (499) 2.6 2.10 6 1

  Street s1_2{3, 1,  386., 13.9, std::make_pair(1, 2), 3};  // (501) 2.6 4.47 4 1                              
  Street s2_1{4, 1,  386., 13.9,std::make_pair(2, 1), 3};  // (820) 4.47 2.6 8 1

  Street s2_3{5, 1,  532., 13.9, std::make_pair(2, 3), 3};  // (821) 4.47 4.46 4 1                             
  Street s3_2{6, 1,  532., 13.9, std::make_pair(3, 2), 3};  // (819) 4.46 4.47 8 1

  Street s3_4{7, 1,  237., 13.9, std::make_pair(3, 4), 1};  // (818) 4.46 4.45 4 1
  Street s4_3{8, 1,  237., 13.9, std::make_pair(4, 3), 3};  // (815) 4.45 4.46 8 1
  Street s3_5{9, 1,  375., 13.9, std::make_pair(3, 5), 3};  // (815) 4.45 4.46 8 1


  Street s5_4{10, 1,  135., 13.9, std::make_pair(5, 4), 3};

  Street s5_6{11, 1,  230., 13.9, std::make_pair(5, 6), 3};  // (814) 4.45 4.44 4 1
  Street s6_5{12, 1,  230., 13.9, std::make_pair(6, 5), 3};  // (812) 4.44 4.45 8 1

  Street s6_7{13, 1,  653., 13.9, std::make_pair(6, 7), 3};  // (811) 4.44 4.41 4 1
  Street s7_6{14, 1,  653., 13.9, std::make_pair(7, 6), 3};  // (801) 4.41 4.44 8 1

  Street s7_8{15, 1,  230., 13.9, std::make_pair(7, 8), 3};  // (800) 4.41 4.42 4 1
  Street s8_7{16, 1,  230., 13.9, std::make_pair(8, 7), 3};  // (803) 4.42 4.41 8 1
  
  // strade secondarie

  Street s9_1{17, 1,  250., 8.3, std::make_pair(9, 1), 1};   // (496) 2.5 2.6 2 1      0.127 2.6 1 1            //saragozza (1)
  Street s1_9{18, 1,  250., 8.3, std::make_pair(1, 9), 2};   // (500) 2.6 2.5 6 1
  Street s10_1{19, 1,  250., 8.3, std::make_pair(10, 1), 1};

  Street s2_11{20, 1,  100., 8.3, std::make_pair(2, 11), 1};                                                    //vallescura  (2)
  Street s11_2{21, 1,  100., 8.3, std::make_pair(11, 2), 1};  // (278) 0.127 4.47 2 1
  Street s12_2{22, 1,  100., 8.3, std::make_pair(12, 2), 1};  // (279) 0.127 4.47 6 1

  Street s13_3{23, 1,  200., 8.3, std::make_pair(13, 3), 1};  // (273) 0.127 4.46 2 1   (274) 0.127 4.46 3 1    //san mamolo  (3)
  Street s3_13{24, 1,  200., 8.3, std::make_pair(3, 13), 1};  // (816) 4.46 0.127 6 1
  Street s3_14{25, 1,  100., 8.3, std::make_pair(3, 14), 1};
  Street s14_3{26, 1,  100., 8.3, std::make_pair(14, 3), 1};  // (275) 0.127 4.46 6 1

  Street s4_15{27, 1,  90., 8.3, std::make_pair(4, 15), 1};                                                     //savenella  (3)

  Street s16_5{28, 1,  90., 8.3, std::make_pair(16, 5), 1};   // (270) 0.127 4.45 6 1                           //rubbiani   (4)

  Street s17_6{29, 1,  200., 8.3, std::make_pair(17, 6), 1};  // (266) 0.127 4.44 2 1                           //castiglione  (6)
  Street s6_17{30, 1,  200., 8.3, std::make_pair(6, 17), 1};
  Street s6_18{31, 1,  100., 8.3, std::make_pair(6, 18), 1};
  Street s18_6{32, 1,  100., 8.3, std::make_pair(18, 6), 1};  // (267) 0.127 4.44 6 1

  Street s19_7{33, 1,  215., 8.3, std::make_pair(19, 7), 1};                                                    //santo stefano  (7)
  Street s7_19{34, 1,  215., 8.3, std::make_pair(7, 19), 2};  // (799) 4.41 4.33 6 1
  Street s20_7{35, 1,  200., 8.3, std::make_pair(20, 7), 1};  // (263) 0.127 4.41 6 1
  

  // saragozza
  tl1.setDelay(std::make_pair(62, 40)); // 40, 70
  tl1.setCapacity(1);
  tl1.addStreetPriority(s0_1.id());
  tl1.addStreetPriority(s2_1.id());
  // vallescura
  tl2.setDelay(std::make_pair(72, 39)); // 50, 75
  tl2.setCapacity(1);
  tl2.addStreetPriority(s1_2.id());
  tl2.addStreetPriority(s3_2.id());
  // san mamolo
  tl3.setDelay(std::make_pair(88, 50)); // 40, 70
  tl3.setCapacity(1);
  tl3.addStreetPriority(s2_3.id());
  tl2.addStreetPriority(s4_3.id());
  // savenella
  tl4.setDelay(std::make_pair(100, 15)); // 38, 106 = 144
  tl4.setCapacity(1);

  tl4.addStreetPriority(s5_4.id());
  // rubbiani
  tl5.setDelay(std::make_pair(82, 39)); // 50, 75
  tl5.setCapacity(1);
  tl5.addStreetPriority(s3_5.id());
  tl5.addStreetPriority(s6_5.id());
  // castiglione
  tl6.setDelay(std::make_pair(88, 40)); // 40, 70
  tl6.setCapacity(1);
  tl6.addStreetPriority(s5_6.id());
  tl6.addStreetPriority(s7_6.id());
  // santo stefano
  tl7.setDelay(std::make_pair(81, 40)); // 38, 106 = 144
  tl7.setCapacity(1);
  tl7.addStreetPriority(s6_7.id());
  tl7.addStreetPriority(s8_7.id());

  
  Graph graph;
  graph.addNode(std::make_unique<TrafficLight>(tl1));
  graph.addNode(std::make_unique<TrafficLight>(tl2));
  graph.addNode(std::make_unique<TrafficLight>(tl3));
  graph.addNode(std::make_unique<TrafficLight>(tl4));
  graph.addNode(std::make_unique<TrafficLight>(tl5));
  graph.addNode(std::make_unique<TrafficLight>(tl6));
  graph.addNode(std::make_unique<TrafficLight>(tl7));
  std::cout << "fino a qui tutto bene" << '\n';
  graph.addStreets(s0_1, s1_0, s1_2, s2_1, s2_3, s3_2, s3_4, s4_3, s3_5, s5_4, s5_6, s6_5, s6_7, s7_6, s7_8, s8_7, s9_1, s1_9, s10_1, s2_11, s11_2, s12_2, s13_3, s3_13, s3_14, s14_3, s4_15, s16_5, s17_6, s6_17, s6_18, s18_6, s19_7, s7_19, s20_7);

  graph.makeSpireStreet(s0_1.id());
  graph.makeSpireStreet(s1_0.id());
  graph.makeSpireStreet(s1_2.id());
  graph.makeSpireStreet(s2_1.id());
  graph.makeSpireStreet(s2_3.id());
  graph.makeSpireStreet(s3_2.id());
  graph.makeSpireStreet(s3_4.id());
  graph.makeSpireStreet(s4_3.id());
  graph.makeSpireStreet(s3_5.id());
  graph.makeSpireStreet(s5_6.id());
  graph.makeSpireStreet(s6_5.id());
  graph.makeSpireStreet(s6_7.id());
  graph.makeSpireStreet(s7_6.id());
  graph.makeSpireStreet(s7_8.id());
  graph.makeSpireStreet(s8_7.id());

  graph.makeSpireStreet(s9_1.id());
  graph.makeSpireStreet(s1_9.id());
  graph.makeSpireStreet(s11_2.id());
  graph.makeSpireStreet(s12_2.id());
  graph.makeSpireStreet(s13_3.id());
  graph.makeSpireStreet(s3_13.id());
  graph.makeSpireStreet(s14_3.id());
  graph.makeSpireStreet(s16_5.id());
  graph.makeSpireStreet(s17_6.id());
  graph.makeSpireStreet(s18_6.id());
  graph.makeSpireStreet(s7_19.id());
  graph.makeSpireStreet(s20_7.id());

  graph.importCoordinates(IN_COORDS);
  graph.buildAdj();
  graph.adjustNodeCapacities();
  graph.normalizeStreetCapacities();

  auto const& nNodes = graph.nodeSet().size();
  auto const& adj{graph.adjMatrix()};
  std::ofstream adjFile(OUT_FOLDER + "adj.dat");
  adjFile << nNodes << '\t' << nNodes << std::endl;
  for (auto i{0}; i < nNodes; ++i) {
    for (auto j{0}; j < nNodes; ++j) {
      adjFile << adj(i, j);
      if (j != nNodes - 1) {
        adjFile << '\t';
      }
    }
    adjFile << std::endl;
  }
  adjFile.close();

  return 0;

  // print nodes and streets
  std::cout << "Nodes: " << graph.nodeSet().size() << '\n';
  std::cout << "Streets: " << graph.streetSet().size() << '\n';

  for (const auto &[streetId, street] : graph.streetSet()) {
    std::cout << "Street ID: " << streetId << ", lanes=" << static_cast<int>(street->nLanes()) << ", len= " << static_cast<int>(street->length()) << ", capacity= " << static_cast<int>(street->capacity()) << ", nodeIN = " << street->nodePair().first << ", nodeOUT = " << street->nodePair().second <<  std::endl;
  }

  for (const auto &[nodeId, node] : graph.nodeSet()) {
    std::cout << "ID = " << node->id() << ", transportCapacity = " << node->transportCapacity() << std::endl;
  }




  // Create the dynamics
  Dynamics dynamics{graph};
  dynamics.setSeed(SEED);
  //dynamics.setErrorProbability(ERROR_PROBABILITY);
  dynamics.setMinSpeedRateo(0.95);
  dynamics.setSpeedFluctuationSTD(0.2);

  std::unordered_map<Unit, double> src{{0, 0.3}, {8, 0.27}, {9, 0.04}, {10, 0.01}, {11, 0.01}, {12, 0.01}, {13, 0.09}, {14, 0.06}, {16, 0.04}, {17, 0.1}, {18, 0.05}, {19, 0.01}, {20, 0.01}};
  std::unordered_map<Unit, double> dst{{0, 0.195}, {8, 0.22}, {9, 0.08}, {11, 0.02}, {13, 0.08}, {14, 0.08}, {15, 0.005}, {17, 0.08}, {18, 0.08}, {19, 0.16}};

  // dynamics.addItinerary(Itinerary{0, 0});
  // dynamics.addItinerary(Itinerary{1, 8});
  // dynamics.addItinerary(Itinerary{2, 9});
  // dynamics.addItinerary(Itinerary{3, 11});
  // dynamics.addItinerary(Itinerary{4, 13});
  // dynamics.addItinerary(Itinerary{5, 14});
  // dynamics.addItinerary(Itinerary{6, 15});
  // dynamics.addItinerary(Itinerary{7, 17});
  // dynamics.addItinerary(Itinerary{8, 18});
  // dynamics.addItinerary(Itinerary{9, 19});
  std::vector <Unit> itinerary{0, 8, 9, 11, 13, 14, 15, 17, 18, 19};
  dynamics.setDestinationNodes(itinerary);

  // auto &spire =
  //     dynamic_cast<SpireStreet &>(*dynamics.graph().streetSet().at(19));

  // lauch progress bar
  // std::thread t([]() {
  //   while (progress < MAX_TIME) {
  //     printLoadingBar(progress, MAX_TIME);
  //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //   }
  // });
  std::ofstream out(OUT_FOLDER + "data.csv");
  out << "time;n_agents;mean_speed;mean_speed_err;mean_density;mean_density_"
         "err;mean_flow;mean_flow_err;mean_traveltime;mean_traveltime_err;mean_flow_spires;mean_flow_spires_err\n";

#ifdef PRINT_DENSITIES
  std::ofstream streetDensity(OUT_FOLDER + "densities.csv");
  streetDensity << "time;";
  for (const auto &[id, street] : dynamics.graph().streetSet()) {
    streetDensity << id << ';';
  }
  streetDensity << '\n';

  std::ofstream nodeDensity(OUT_FOLDER + "nodedensities.csv");
  nodeDensity << "time;";
  for (const auto &[id, node] : dynamics.graph().nodeSet()) {
    nodeDensity << id << ';';
  }
  nodeDensity << '\n';
#endif

#ifdef PRINT_FLOWS
  std::ofstream streetFlow(OUT_FOLDER + "flows.csv");
  streetFlow << "time;";
  for (const auto &[id, street] : dynamics.graph().streetSet()) {
    streetFlow << id << ';';
  }
  streetFlow << '\n';
#endif
#ifdef PRINT_SPEEDS
  std::ofstream streetSpeed(OUT_FOLDER + "speeds.csv");
  streetSpeed << "time;";
  for (const auto &[id, street] : dynamics.graph().streetSet()) {
    streetSpeed << id << ';';
  }
  streetSpeed << '\n';
#endif


  // Evolution
  uint nAgents{60};
 

  while (progress < MAX_TIME) {

  // EVOLUTION   -   -   -


        if (progress % 30 == 0) {
          try {
            dynamics.addAgentsRandomly(nAgents, src, dst);
          } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            for (auto const& [id, agent] : dynamics.agents()) {
              std::cout << "Agent ID " << id << " srcNodeID " << agent->srcNodeId().value() << " dstNodeID " << agent->itineraryId();
              if (agent->streetId().has_value()) {
                std::cout << " streetID " << agent->streetId().value();
              }
              std::cout << std::endl;
            }
            break;
          }
          
        }
      dynamics.evolve(false);
      
      


  // OUTPUTS   -   -   -




    if (dynamics.time() % 30 == 0) {
      const auto &meanSpeed{dynamics.streetMeanSpeed()};
      const auto &meanDensity{dynamics.streetMeanDensity(true)};
      const auto &meanFlow{dynamics.streetMeanFlow()};
      const auto &meanTravelTime{dynamics.meanTravelTime()};
      const auto &meanSpireFlow{dynamics.meanSpireOutputFlow()};

      out << dynamics.time() << ';' << dynamics.agents().size() << ';'
          << meanSpeed.mean << ';' << meanSpeed.std << ';' << meanDensity.mean
          << ';' << meanDensity.std << ';' << meanFlow.mean << ';'
          << meanFlow.std << ';' << meanTravelTime.mean << ';'
          << meanTravelTime.std << ';' << meanSpireFlow.mean << ';'
          << meanSpireFlow.std << std::endl;
    }

  if (dynamics.time() % 10 == 0) {
#ifdef PRINT_DENSITIES
      streetDensity << dynamics.time() << ';';
      for (const auto &[id, street] : dynamics.graph().streetSet()) {
        streetDensity << street->normDensity() << ';';
      }
      streetDensity << std::endl;
      nodeDensity << dynamics.time() << ';';
      for (const auto &[id, node] : dynamics.graph().nodeSet()) {
        nodeDensity << node->density() << ';';
      }
      nodeDensity << std::endl;
#endif
#ifdef PRINT_FLOWS
      streetFlow << dynamics.time() << ';';
      for (const auto &[id, street] : dynamics.graph().streetSet()) {
        const auto &meanSpeed = dynamics.streetMeanSpeed(id);
        if (meanSpeed.has_value()) {
          streetFlow << meanSpeed.value() * street->density() << ';';
        } else {
          streetFlow << 0 << ';';
        }
      }
      streetFlow << std::endl;
#endif
#ifdef PRINT_SPEEDS
      streetSpeed << dynamics.time() << ';';
      for (const auto &[id, street] : dynamics.graph().streetSet()) {
        const auto &meanSpeed = dynamics.streetMeanSpeed(id);
        if (meanSpeed.has_value()) {
          streetSpeed << meanSpeed.value() << ';';
        } else {
          streetSpeed << 0 << ';';
        }
      }
      streetSpeed << std::endl;
#endif
    }
        
  ++progress;
  }





  // while (progress < MAX_TIME) {
  //   if (progress % 60 == 0) {
  //     if (progress != 0) {
  //       ++it;
  //     }
  //     const int agentNumber = dynamics.agents().size();
  //     if (progress % 300 == 0) {
  //       ofs << progress << ";" << spire.outputCounts(true) << std::endl;
  //     }
  //     dynamics.addAgents(0, *it / 2, 0);
  //   }
  //   dynamics.evolve(false);
  //   ++progress;
  // }
  // t.join();

  return 0;
}