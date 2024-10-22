#include "dsm/dsm.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include <thread>
#include <atomic>

std::atomic_int progress{0};

// Compatible with dsm 1.3.8

using unit = uint32_t;

using Graph = dsm::Graph<unit, unit>;
using Itinerary = dsm::Itinerary<unit>;
using Dynamics = dsm::FirstOrderDynamics<unit, unit, unit>;
using TrafficLight = dsm::TrafficLight<unit, unit, unit>;
using Street = dsm::Street<unit, unit>;
using SpireStreet = dsm::SpireStreet<unit, unit>;

void printLoadingBar(int const i, int const n) {
  std::cout << "Loading: " << std::setprecision(2) << std::fixed
            << (i * 100. / n) << "%" << '\r';
  std::cout.flush();
}

int main() {
  std::cout << "Using dsm version: " << dsm::version() << '\n';
  // // Import input data
  // std::ifstream ifs{"./data/input.txt"};
  // unit timeUnit{0};
  // ifs >> timeUnit;
  // std::vector<unit> vehiclesToInsert{};
  // while (!ifs.eof()) {
  //   unit vehicleId{0};
  //   ifs >> vehicleId;
  //   vehiclesToInsert.push_back(vehicleId);
  // }
  // const unit MAX_TIME{static_cast<unit>(timeUnit * vehiclesToInsert.size())};

  // Create the graph
  
  TrafficLight tl1{1}, tl2{2}, tl3{3}, tl4{4}, tl5{5}, tl6{6}, tl7{7};


  // segmenti viali
  Street s0_1{1, 1,  480., 13.9, std::make_pair(0, 1), 3};  // (402) 2.10 2.6 6 1                              
  Street s1_0{2, 1,  480., 13.9, std::make_pair(1, 0), 3};  // (499) 2.6 2.10 6 1

  Street s1_2{3, 1,  386., 13.9, std::make_pair(1, 2), 3};  // (501) 2.6 4.47 4 1                              
  Street s2_1{4, 1,  386., 13.9,std::make_pair(2, 1), 3};  // (820) 4.47 2.6 8 1

  Street s2_3{5, 1,  532., 13.9, std::make_pair(2, 3), 3};  // (821) 4.47 4.46 4 1                             
  Street s3_2{6, 1,  532., 13.9, std::make_pair(3, 2), 3};  // (819) 4.46 4.47 8 1

  Street s3_4{7, 1,  237., 13.9, std::make_pair(3, 4), 3};  // (818) 4.46 4.45 4 1
  Street s4_3{8, 1,  237., 13.9, std::make_pair(4, 3), 3};  // (815) 4.45 4.46 8 1

  Street s4_5{9, 1,  135., 13.9, std::make_pair(4, 5), 3};
  Street s5_4{10, 1,  135., 13.9, std::make_pair(5, 4), 3};

  Street s5_6{11, 1,  230., 13.9, std::make_pair(5, 6), 3};  // (814) 4.45 4.44 4 1
  Street s6_5{12, 1,  230., 13.9, std::make_pair(6, 5), 3};  // (812) 4.44 4.45 8 1

  Street s6_7{13, 1,  653., 13.9, std::make_pair(6, 7), 3};  // (811) 4.44 4.41 4 1
  Street s7_6{14, 1,  653., 13.9, std::make_pair(7, 6), 3};  // (801) 4.41 4.44 8 1

  Street s7_8{15, 1,  230., 13.9, std::make_pair(7, 8), 3};  // (800) 4.41 4.42 4 1
  Street s8_7{16, 1,  230., 13.9, std::make_pair(8, 7), 3};  // (803) 4.42 4.41 8 1
  
  // strade secondarie

  Street s9_1{17, 1,  250., 8.3, std::make_pair(9, 1), 1};   // (496) 2.5 2.6 2 1                               //saragozza (1)
  Street s1_9{18, 1,  250., 8.3, std::make_pair(1, 9), 2};   // (500) 2.6 2.5 6 1
  Street s10_1{19, 1,  250., 8.3, std::make_pair(10, 1), 1};

  Street s2_11{20, 1,  100., 8.3, std::make_pair(2, 11), 1};                                                    //vallescura  (2)
  Street s11_2{21, 1,  100., 8.3, std::make_pair(11, 2), 1};  // (278) 0.127 4.47 2 1
  Street s12_2{22, 1,  100., 8.3, std::make_pair(12, 2), 1};  // (279) 0.127 4.47 6 1

  Street s13_3{23, 1,  200., 8.3, std::make_pair(13, 3), 1};  // (273) 0.127 4.46 2 1   (274) 0.127 4.46 3 1    //san mammolo  (3)
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
  tl1.setDelay(std::make_pair(62, 70)); // 40, 70
  tl1.setCapacity(1);
  tl1.addStreetPriority(s0_1.id());
  // vallescura
  tl2.setDelay(std::make_pair(72, 69)); // 50, 75
  tl2.setCapacity(1);
  tl2.addStreetPriority(s1_2.id());
  // san mammolo
  tl3.setDelay(std::make_pair(88, 50)); // 40, 70
  tl3.setCapacity(1);
  tl3.addStreetPriority(s2_3.id());
  // savenella
  tl4.setDelay(std::make_pair(81, 50)); // 38, 106 = 144
  tl4.setCapacity(1);
  tl4.addStreetPriority(s3_4.id());
  // rubbiani
  tl5.setDelay(std::make_pair(72, 69)); // 50, 75
  tl5.setCapacity(1);
  tl5.addStreetPriority(s1_2.id());
  // castiglione
  tl6.setDelay(std::make_pair(88, 50)); // 40, 70
  tl6.setCapacity(1);
  tl6.addStreetPriority(s2_3.id());
  // santo stefano
  tl7.setDelay(std::make_pair(81, 50)); // 38, 106 = 144
  tl7.setCapacity(1);
  tl7.addStreetPriority(s3_4.id());

  
  Graph graph;
  graph.addNode(std::make_unique<TrafficLight>(tl1));
  graph.addNode(std::make_unique<TrafficLight>(tl2));
  graph.addNode(std::make_unique<TrafficLight>(tl3));
  graph.addNode(std::make_unique<TrafficLight>(tl4));
  graph.addNode(std::make_unique<TrafficLight>(tl5));
  graph.addNode(std::make_unique<TrafficLight>(tl6));
  graph.addNode(std::make_unique<TrafficLight>(tl7));
  std::cout << "fino a qui tutto bene" << '\n';
  graph.addStreets(s0_1, s1_0, s1_2, s2_1, s2_3, s3_2, s3_4, s4_3, s4_5, s5_4, s5_6, s6_5, s6_7, s7_6, s7_8, s8_7, s9_1, s1_9, s10_1, s2_11, s11_2, s12_2, s13_3, s3_13, s3_14, s14_3, s4_15, s16_5, s17_6, s6_17, s6_18, s18_6, s19_7, s7_19, s20_7);
  graph.buildAdj();

  
  // print nodes and streets
  std::cout << "Nodes: " << graph.nodeSet().size() << '\n';
  std::cout << "Streets: " << graph.streetSet().size() << '\n';

  for (const auto &[streetId, street] : graph.streetSet()) {
    std::cout << "Street ID: " << streetId << ", lanes=" << static_cast<int>(street->nLanes()) << ", len= " << static_cast<int>(street->length()) << ", capacity= " << static_cast<int>(street->capacity()) << std::endl;
  }
  // // Create the dynamics
  // Dynamics dynamics{graph};
  // dynamics.setSeed(69);
  // dynamics.setMinSpeedRateo(0.95);
  // dynamics.setSpeedFluctuationSTD(0.2);
  // Itinerary itinerary{0, 4};
  // dynamics.addItinerary(itinerary);
  // dynamics.updatePaths();

  // auto &spire =
  //     dynamic_cast<SpireStreet &>(*dynamics.graph().streetSet().at(19));

  // // lauch progress bar
  // std::thread t([MAX_TIME]() {
  //   while (progress < MAX_TIME) {
  //     printLoadingBar(progress, MAX_TIME);
  //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //   }
  // });
  // // Evolution
  // auto it = vehiclesToInsert.begin();
  // std::ofstream ofs{"./stalingrado_output.csv"};
  // // print two columns, time and vehicles
  // ofs << "time;vehicle_flux;" << '\n';
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