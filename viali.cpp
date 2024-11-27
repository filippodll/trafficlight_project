#include "dsm/dsm.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <format>
#include <atomic>
#include <thread>
namespace fs = std::filesystem;



std::atomic<size_t> progress{0};
std::atomic<bool> bExitFlag{false};

const std::string IN_COORDS{"./coordinates.dsm"};  // input coords file

// Compatible with dsm 1.3.8

using Delay = uint8_t;

using Dynamics = dsm::FirstOrderDynamics<Delay>;
using Street = dsm::Street;
using TrafficLight = dsm::TrafficLight;

void printLoadingBar(int const i, int const n) {
  std::cout << "Loading: " << std::setprecision(2) << std::fixed << (i * 100. / n) << "%"
            << '\r';
  std::cout.flush();
}

size_t constexpr MAX_TIME{86400};  // maximum time of simulation
size_t constexpr INTERVAL_AGENTS_IN{30};

typedef std::vector<size_t> data_t;  // data type

#ifdef __APPLE__
typedef std::thread thread_t;
#else
typedef std::jthread thread_t;
#endif

int main(int argc, char* argv[]) {
  if (argc != 7) {
    std::cerr << "Usage: " << argv[0]
              << " <SEED> <DAY> <GRANULARITY> <DELAY> <DATA_FOLDER> <OPTIMIZE>\n";
    return 1;
  }

  int const SEED{std::stoi(argv[1])};           // seed for random number generator
  std::string const DAY{argv[2]};               // day of the week
  int const GRANULARITY{std::stoi(argv[3])};    // granularity of the data in seconds
  int const DELAY{std::stoi(argv[4])};          // delay in granularity
  std::string const DATA_FOLDER{argv[5]};       // folder containing the data files
  bool const OPTIMIZE{std::stoi(argv[6]) > 0};  // optimize the graph


  std::string const INPUT_FILE{std::format("{}/{}.csv", DATA_FOLDER, DAY)};
  std::string OUT_FOLDER{std::format("./{}", DAY)};
  if (OPTIMIZE) {
    OUT_FOLDER += "-optimized/";
  } else {
    OUT_FOLDER += '/';
  }

  size_t const NDATAPOINTS{MAX_TIME / GRANULARITY};  // number of data points

  if (fs::exists(OUT_FOLDER)) {
    fs::remove_all(OUT_FOLDER);
  }
  fs::create_directory(OUT_FOLDER);
  if (fs::exists("./constants")) {
    fs::remove_all("./constants");
  }
  fs::create_directory("./constants");
  std::cout << std::format("Using dsm version: {}", dsm::version())<< std::endl;
  std::cout << std::format("Output folder: {}", OUT_FOLDER)<< std::endl;

  // Create the graph

  TrafficLight tl1{1, 127}, tl2{2, 125}, tl3{3, 155}, tl4{4, 83}, tl5{5, 95}, tl6{6, 145}, tl7{7, 115};

  // segmenti viali
  Street s0_1{
      1, 1, 500., 13.9, std::make_pair(0, 1), 3, "2.10 2.6 6 1"};  // (402) 2.10 2.6 6 1
  Street s1_0{
      2, 1, 500., 13.9, std::make_pair(1, 0), 3, "2.6 2.10 6 1"};  // (499) 2.6 2.10 6 1

  Street s1_2{
      3, 1, 400., 13.9, std::make_pair(1, 2), 3, "2.6 4.47 4 1"};  // (501) 2.6 4.47 4 1
  Street s2_1{
      4, 1, 400., 13.9, std::make_pair(2, 1), 3, "4.47 2.6 8 1"};  // (820) 4.47 2.6 8 1

  Street s2_3{
      5, 1, 550., 13.9, std::make_pair(2, 3), 3, "4.47 4.46 4 1"};  // (821) 4.47 4.46 4 1
  Street s3_2{
      6, 1, 550., 13.9, std::make_pair(3, 2), 3, "4.46 4.47 8 1"};  // (819) 4.46 4.47 8 1

  Street s3_4{
      7, 1, 260., 13.9, std::make_pair(3, 4), 3, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1
  Street s4_3{
      8, 1, 260., 13.9, std::make_pair(4, 3), 3, "4.45 4.46 8 1"};  // (815) 4.45 4.46 8 1
  Street s4_5{
      9, 1, 150., 13.9, std::make_pair(4, 5), 3, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1

  Street s5_4{10, 1, 150., 13.9, std::make_pair(5, 4), 3};

  Street s5_6{
      11, 1, 300., 13.9, std::make_pair(5, 6), 3, "4.45 4.44 4 1"};  // (814) 4.45 4.44 4 1
  Street s6_5{
      12, 1, 300., 13.9, std::make_pair(6, 5), 3, "4.44 4.45 8 1"};  // (812) 4.44 4.45 8 1

  Street s6_7{
      13, 1, 700., 13.9, std::make_pair(6, 7), 3, "4.44 4.41 4 1"};  // (811) 4.44 4.41 4 1
  Street s7_6{
      14, 1, 700., 13.9, std::make_pair(7, 6), 3, "4.41 4.44 8 1"};  // (801) 4.41 4.44 8 1

  Street s7_8{
      15, 1, 230., 13.9, std::make_pair(7, 8), 3, "4.41 4.42 4 1"};  // (800) 4.41 4.42 4 1
  Street s8_7{
      16, 1, 230., 13.9, std::make_pair(8, 7), 3, "4.42 4.41 8 1"};  // (803) 4.42 4.41 8 1

  // strade secondarie

  Street s9_1{17,
              1,
              750.,
              8.3,
              std::make_pair(9, 1),
              2,
              "2.5 2.6 2 1"};  // (496) 2.5 2.6 2 1      0.127 2.6 1 1 //saragozza (1)
  Street s1_9{
      18, 1, 750., 8.3, std::make_pair(1, 9), 2, "2.6 2.5 6 1"};  // (500) 2.6 2.5 6 1
  Street s10_1{19, 1, 250., 8.3, std::make_pair(10, 1), 1};

  Street s2_11{20, 1, 300., 8.3, std::make_pair(2, 11), 1};  // vallescura  (2)
  Street s11_2{21,
               1,
               300.,
               8.3,
               std::make_pair(11, 2),
               1,
               "0.127 4.47 2 1"};  // (278) 0.127 4.47 2 1
  Street s12_2{22,
               1,
               160.,
               8.3,
               std::make_pair(12, 2),
               1,
               "0.127 4.47 6 1"};  // Malpertuso (279) 0.127 4.47 6 1

  Street s13_3{23,
               1,
               500.,
               8.3,
               std::make_pair(13, 3),
               2,
               "0.127 4.46 2 1"};  // (273) 0.127 4.46 2 1   (274) 0.127 4.46 3
                                   // 1    //san mamolo  (3)
  Street s3_13{24,
               1,
               500.,
               8.3,
               std::make_pair(3, 13),
               1,
               "4.46 0.127 6 1"};  // (816) 4.46 0.127 6 1
  Street s3_14{25, 1, 240., 8.3, std::make_pair(3, 14), 1};
  Street s14_3{26,
               1,
               240.,
               8.3,
               std::make_pair(14, 3),
               1,
               "0.127 4.46 6 1"};  // (275) 0.127 4.46 6 1

  Street s4_15{27, 1, 190., 8.3, std::make_pair(4, 15), 1};  // savenella  (3)

  Street s16_5{28,
               1,
               270.,
               8.3,
               std::make_pair(16, 5),
               1,
               "0.127 4.45 6 1"};  // (270) 0.127 4.45 6 1 //rubbiani   (4)

  Street s17_6{29,
               1,
               400.,
               8.3,
               std::make_pair(17, 6),
               1,
               "0.127 4.44 2 1"};  // (266) 0.127 4.44 2 1 //castiglione  (6)
  Street s6_17{30, 1, 400., 8.3, std::make_pair(6, 17), 1};
  Street s6_18{31, 1, 200., 8.3, std::make_pair(6, 18), 1};
  Street s18_6{32,
               1,
               200.,
               8.3,
               std::make_pair(18, 6),
               1,
               "0.127 4.44 6 1"};  // (267) 0.127 4.44 6 1

  Street s19_7{33, 1, 240., 8.3, std::make_pair(19, 7), 1};  // santo stefano (7)
  Street s7_19{
      34, 1, 240., 8.3, std::make_pair(7, 19), 2, "4.41 4.33 6 1"};  // (799) 4.41 4.33 6 1
  Street s20_7{35,
               1,
               350.,
               8.3,
               std::make_pair(20, 7),
               1,
               "0.127 4.41 6 1"};  // (263) 0.127 4.41 6 1

  // saragozza

  tl1.setCycle(s0_1.id(), dsm::Direction::STRAIGHT , {82, 0});
  tl1.setCycle(s0_1.id(), dsm::Direction::RIGHT , {112, 97});

  tl1.setCycle(s2_1.id(), dsm::Direction::STRAIGHT , {82, 0});
  tl1.setCycle(s2_1.id(), dsm::Direction::LEFT , {30, 82});
  
  tl1.setCycle(s9_1.id(), dsm::Direction::RIGHT , {127, 0});
  tl1.setCycle(s9_1.id(), dsm::Direction::STRAIGHT , {30, 97});

  tl1.setCycle(s10_1.id(), dsm::Direction::ANY , {15, 82});

  
  // vallescura
  tl2.setCycle(s1_2.id(), dsm::Direction::RIGHTANDSTRAIGHT , {78, 25});

  tl2.setCycle(s3_2.id(), dsm::Direction::RIGHT , {100, 25});

  tl2.setCycle(s3_2.id(), dsm::Direction::LEFT , {22, 103});

  tl2.setCycle(s11_2.id(), dsm::Direction::ANY , {25, 0});
  tl2.setCycle(s12_2.id(), dsm::Direction::ANY , {25, 0});
  // san mamolo
  tl3.setCycle(s2_3.id(), dsm::Direction::RIGHTANDSTRAIGHT , {85, 0});

  tl3.setCycle(s4_3.id(), dsm::Direction::RIGHTANDSTRAIGHT , {120, 0});
  tl3.setCycle(s4_3.id(), dsm::Direction::LEFT , {35, 85});

  tl3.setCycle(s13_3.id(), dsm::Direction::RIGHT , {130, 25});
  tl3.setCycle(s13_3.id(), dsm::Direction::LEFTANDSTRAIGHT , {35, 120});

  tl3.setCycle(s14_3.id(), dsm::Direction::ANY , {35, 120});


  // savenella
  tl4.setCycle(s3_4.id(), dsm::Direction::STRAIGHT , {83, 0});
  tl4.setCycle(s3_4.id(), dsm::Direction::LEFT , {30, 0});

  tl4.setCycle(s3_4.id(), dsm::Direction::RIGHTANDSTRAIGHT , {53, 30});


  // rubbiani
  tl5.setCycle(s4_5.id(), dsm::Direction::STRAIGHT , {55, 0});
  tl5.setCycle(s6_5.id(), dsm::Direction::STRAIGHT , {55, 0});

  tl5.setCycle(s16_5.id(), dsm::Direction::ANY , {40, 55});

  // castiglione
  tl6.setCycle(s5_6.id(), dsm::Direction::ANY , {60, 0});

  tl6.setCycle(s7_6.id(), dsm::Direction::RIGHTANDSTRAIGHT , {85, 0});
  tl6.setCycle(s5_6.id(), dsm::Direction::LEFT , {25, 60});

  tl6.setCycle(s17_6.id(), dsm::Direction::ANY , {60, 85});
  tl6.setCycle(s18_6.id(), dsm::Direction::ANY , {60, 85});


  // santo stefano
  tl7.setCycle(s6_7.id(), dsm::Direction::RIGHT , {90, 0});
  tl7.setCycle(s6_7.id(), dsm::Direction::LEFTANDSTRAIGHT , {35, 0});

  tl7.setCycle(s8_7.id(), dsm::Direction::RIGHTANDSTRAIGHT , {90, 0});
  tl7.setCycle(s6_7.id(), dsm::Direction::LEFT , {55, 35});

  tl7.setCycle(s19_7.id(), dsm::Direction::ANY , {25, 90});
  tl7.setCycle(s20_7.id(), dsm::Direction::ANY , {25, 90});

  dsm::Graph graph;
  graph.addNode(std::make_unique<TrafficLight>(tl1));
  graph.addNode(std::make_unique<TrafficLight>(tl2));
  graph.addNode(std::make_unique<TrafficLight>(tl3));
  graph.addNode(std::make_unique<TrafficLight>(tl4));
  graph.addNode(std::make_unique<TrafficLight>(tl5));
  graph.addNode(std::make_unique<TrafficLight>(tl6));
  graph.addNode(std::make_unique<TrafficLight>(tl7));
  graph.addStreets(s0_1,
                   s1_0,
                   s1_2,
                   s2_1,
                   s2_3,
                   s3_2,
                   s3_4,
                   s4_3,
                   s4_5,
                   s5_4,
                   s5_6,
                   s6_5,
                   s6_7,
                   s7_6,
                   s7_8,
                   s8_7,
                   s9_1,
                   s1_9,
                   s10_1,
                   s2_11,
                   s11_2,
                   s12_2,
                   s13_3,
                   s3_13,
                   s3_14,
                   s14_3,
                   s4_15,
                   s16_5,
                   s17_6,
                   s6_17,
                   s6_18,
                   s18_6,
                   s19_7,
                   s7_19,
                   s20_7);

  graph.makeSpireStreet(s0_1.id());
  graph.makeSpireStreet(s1_0.id());
  graph.makeSpireStreet(s1_2.id());
  graph.makeSpireStreet(s2_1.id());
  graph.makeSpireStreet(s2_3.id());
  graph.makeSpireStreet(s3_2.id());
  graph.makeSpireStreet(s3_4.id());
  graph.makeSpireStreet(s4_3.id());
  graph.makeSpireStreet(s4_5.id());
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

  graph.exportCoordinates("./constants/coords.csv");

  auto const& matrix{graph.adjMatrix()};
  auto const n{matrix.getColDim()};
  std::ofstream adj("./constants/adj.dat");
  adj << n << '\t' << n << '\n';
  for (unsigned int i = 0; i < n; ++i) {
    for (unsigned int j = 0; j < n; ++j) {
      adj << matrix(i, j) << '\t';
    }
    adj << '\n';
  }
  adj.close();

  std::unordered_map<std::string_view, dsm::Id> coilmap;
  std::ofstream dict("./structures_out.py");
  dict << "COIL_DICT = {\n";
  for (auto const& [id, street] : graph.streetSet()) {
    if (street->isSpire()) {
      dict << '\"' << street->name() << "\": " << id << ",\n";  // Python dictionary
      coilmap[street->name()] = id;
    }
  }
  dict << "}\n";
  dict.close();
  // Create the dynamics
  Dynamics dynamics{graph};
  dynamics.setSeed(SEED);
  dynamics.setMinSpeedRateo(0.95);
  if (OPTIMIZE) {
    dynamics.setDataUpdatePeriod(INTERVAL_AGENTS_IN);
  }
  dynamics.setSpeedFluctuationSTD(0.1);
  // dynamics.setMaxFlowPercentage(0.75);

  auto const& streets{dynamics.graph().streetSet()};

  std::cout << std::format("Importing input data...")<< std::endl;
  std::ifstream ifs(INPUT_FILE);
  if (!ifs) {
    std::cout << std::format("Cannot open input file {}", INPUT_FILE)<<std::endl;
    return 1;
  }
  std::string line;
  std::getline(ifs, line);  // skip header
  std::map<dsm::Id, data_t> input_data;
  std::map<dsm::Id, data_t> output_data;
  std::map<dsm::Id, data_t> inner_data;
  int iValue;
  std::set<dsm::Id> const inputCoils{
      1, 175, 190, 233, 254, 276, 297, 341, 363, 384, 427, /**/ 211, 406};
  std::set<dsm::Id> const outputCoils{21, 30, 76, 155, 166, /**/ 53, 77, 99, 143};
  std::set<dsm::Id> const innerCoils{23, 43, 45, 65, 67, 87, 68, 109, 111, 131, 133, 153};
  while (std::getline(ifs, line)) {
    std::istringstream iss(line);
    std::string token;

    std::getline(iss, token, ';');
    if (!coilmap.contains(token)) {
      std::cout << std::format("Unknown coil {}. Skipping.", token)<<std::endl;
      continue;
    }
    dsm::Id streetId = coilmap.at(token);
    if (inputCoils.contains(streetId)) {
      auto const nodeId{streets.at(streetId)->nodePair().first};
      input_data[nodeId] = data_t(NDATAPOINTS, 0);
      for (size_t i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        if (iValue > 0) {
          input_data[nodeId][i] = iValue;
        } else {
          input_data[nodeId][i] = 0;
        }
      }
    } else if (outputCoils.contains(streetId)) {
      auto const nodeId{streets.at(streetId)->nodePair().second};
      output_data[nodeId] = data_t(NDATAPOINTS, 0);
      for (size_t i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        if (iValue > 0) {
          output_data[nodeId][i] = iValue;
        } else {
          output_data[nodeId][i] = 0;
        }
      }
    } else if (innerCoils.contains(streetId)) {
      inner_data[streetId] = data_t(NDATAPOINTS, 0);
      if (streetId == 87) {
        inner_data[109] = data_t(NDATAPOINTS, 0);
      }
      if (streetId == 109) {
        inner_data[87] = data_t(NDATAPOINTS, 0);
      }
      for (size_t i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        if (iValue > 0) {
          inner_data[streetId][i] = iValue;
        } else {
          inner_data[streetId][i] = 0;
        }
        if (streetId == 87) {
          inner_data[109][i] = inner_data[streetId][i];
        }
        if (streetId == 109) {
          inner_data[87][i] = inner_data[streetId][i];
        }
      }
    }
  }
  ifs.close();

  std::cout << std::format("Input data imported")<<std::endl;
  std::cout << std::format("Creating itineraries")<<std::endl;

  std::vector<dsm::Id> outNodeList;
  outNodeList.reserve(output_data.size());
  for (const auto& id : outputCoils) {
    auto const& nid{streets.at(id)->nodePair().second};
    outNodeList.push_back(nid);
  }
  dynamics.setDestinationNodes(outNodeList);

  std::cout << std::format("Destination nodes set")<<std::endl;

  // launch progress bar
  thread_t t([]() {
    while (progress < MAX_TIME && !bExitFlag) {
      printLoadingBar(progress, MAX_TIME);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });
  std::ofstream out(OUT_FOLDER + "data.csv");
  out << "time;n_agents;mean_speed;mean_speed_err;mean_density;mean_density_"
         "err;mean_flow;mean_flow_err;mean_traveltime;mean_traveltime_err\n";
  std::ofstream streetDensity(OUT_FOLDER + "densities.csv");
  std::ofstream streetQueues(OUT_FOLDER + "queues.csv");
  streetDensity << "time";
  streetQueues << "time";
  for (const auto& [id, street] : dynamics.graph().streetSet()) {
    streetDensity << ';' << id;
    streetQueues << ';' << id;
  }
  streetDensity << std::endl;
  streetQueues << std::endl;

  std::ofstream nodeDensity(OUT_FOLDER + "nodedensities.csv");
  nodeDensity << "time";
  for (const auto& [id, node] : dynamics.graph().nodeSet()) {
    nodeDensity << ';' << id;
  }
  nodeDensity << std::endl;

  std::ofstream outSpires(OUT_FOLDER + "out_spires.csv");
  outSpires << "time";
  for (const auto& [id, street] : dynamics.graph().streetSet()) {
    outSpires << ';' << id;
  }
  outSpires << std::endl;

  size_t current_index{0};
  dsm::Size nAgents{0};

  std::map<dsm::Id, double> srcProbabilities, dstProbabilities;

  auto const& adjMatrix{dynamics.graph().adjMatrix()};

  std::map<dsm::Id, data_t> synthetic_data;

  while (progress < MAX_TIME) {
    if (progress % GRANULARITY == 0) {
      srcProbabilities.clear();
      dstProbabilities.clear();
      size_t const idx_in{current_index};
      size_t const idx_out{(current_index + DELAY) % NDATAPOINTS};
      for (auto const& [id, data] : input_data) {
        srcProbabilities[id] = data[idx_in];
      }
      for (auto const& [id, data] : output_data) {
        dstProbabilities[id] = (data[idx_out]);
      }
      // Balance every node input
      std::unordered_map<dsm::Id, dsm::Size> synthetic_inner_data;
      for (auto const& [nodeId, node] : dynamics.graph().nodeSet()) {
        auto const& inputRoads{adjMatrix.getCol(nodeId, true)};
        auto const& outputRoads{adjMatrix.getRow(nodeId, true)};
        int inputCounts{0};
        int outputCounts{0};
        std::set<dsm::Id> missingInput;
        std::set<dsm::Id> missingOutput;
        // Input roads
        for (auto const& [inputStreetId, _] : inputRoads) {
          auto const id = streets.at(inputStreetId)->nodePair().first;
          if (srcProbabilities.contains(id)) {
            inputCounts += srcProbabilities[id];
          } else if (inner_data.contains(inputStreetId)) {
            inputCounts += inner_data[inputStreetId][idx_in];
          } else if (synthetic_inner_data.contains(inputStreetId)) {
            inputCounts += synthetic_inner_data[inputStreetId];
          } else {
            missingInput.emplace(inputStreetId);
          }
        }
        // Output roads
        for (auto const& [outputStreetId, _] : outputRoads) {
          auto const id = streets.at(outputStreetId)->nodePair().second;
          if (dstProbabilities.contains(id)) {
            outputCounts += dstProbabilities[id];
          } else if (inner_data.contains(outputStreetId)) {
            outputCounts += inner_data[outputStreetId][idx_out];
          } else if (synthetic_inner_data.contains(outputStreetId)) {
            outputCounts += synthetic_inner_data[outputStreetId];
          } else {
            missingOutput.emplace(outputStreetId);
          }
        }
        auto const deltaTOT{inputCounts - outputCounts};
        if (deltaTOT == 0) {
          continue;
        } else {
          std::cout << std::format(
              "Node {} has an overall delta of {}. Missing {} inputs and {} outputs.",
              nodeId,
              deltaTOT,
              missingInput.size(),
              missingOutput.size())<<std::endl;
        }
        ////////////////////////
        // Balance the nodes  //
        ////////////////////////
        if (deltaTOT > 0) {
          // Input > Output ===> Add agents to output
          auto const deltaPerRoad{std::abs(static_cast<double>(deltaTOT)) /
                                  missingOutput.size()};
          for (auto const& id : missingOutput) {
            if (outputCoils.contains(id)) {
              auto const nid = streets.at(id)->nodePair().second;
              if (!dstProbabilities.contains(nid)) {
                if (!synthetic_data.contains(id)) {
                  synthetic_data[id] = data_t(NDATAPOINTS, 0);
                }
                dstProbabilities[nid] = deltaPerRoad;
                synthetic_data[id][current_index] = deltaPerRoad;
              }
            } else if (innerCoils.contains(id)) {
              if (synthetic_inner_data.contains(id)) {
                std::cout << std::format("Inner coil {} already has data", id)<<std::endl;
              }
              synthetic_inner_data[id] = deltaPerRoad;
            }
          }
          for (auto const& id : missingInput) {
            if (inputCoils.contains(id)) {
              auto const nid = streets.at(id)->nodePair().first;
              if (!srcProbabilities.contains(nid)) {
                if (!synthetic_data.contains(id)) {
                  synthetic_data[id] = data_t(NDATAPOINTS, 0);
                }
                srcProbabilities[nid] = 0.;
                synthetic_data[id][current_index] = 0.;
              }
            } else if (innerCoils.contains(id)) {
              if (synthetic_inner_data.contains(id)) {
                std::cout << std::format("Inner coil {} already has data", id)<<std::endl;
              }
              synthetic_inner_data[id] = 0.;
            }
          }
        } else if (deltaTOT < 0) {
          // Output > Input ===> Add agents to input
          auto const deltaPerRoad{std::abs(static_cast<double>(deltaTOT)) /
                                  missingInput.size()};
          for (auto const& id : missingInput) {
            if (inputCoils.contains(id)) {
              auto const nid = streets.at(id)->nodePair().first;
              if (!srcProbabilities.contains(nid)) {
                srcProbabilities[nid] = deltaPerRoad;
                if (!synthetic_data.contains(id)) {
                  synthetic_data[id] = data_t(NDATAPOINTS, 0);
                }
                synthetic_data[id][current_index] = deltaPerRoad;
              }
            } else if (innerCoils.contains(id)) {
              if (synthetic_inner_data.contains(id)) {
                std::cout << std::format("Inner coil {} already has data", id)<<std::endl;
              }
              synthetic_inner_data[id] = deltaPerRoad;
            }
          }
          for (auto const& id : missingOutput) {
            if (outputCoils.contains(id)) {
              auto const nid = streets.at(id)->nodePair().second;
              if (!dstProbabilities.contains(nid)) {
                dstProbabilities[nid] = 0.;
                if (!synthetic_data.contains(id)) {
                  synthetic_data[id] = data_t(NDATAPOINTS, 0);
                }
                synthetic_data[id][current_index] = 0.;
              }
            } else if (innerCoils.contains(id)) {
              if (synthetic_inner_data.contains(id)) {
                std::cout << std::format("Inner coil {} already has data", id)<<std::endl;
              }
              synthetic_inner_data[id] = 0.;
            }
          }
        }
      }
      if (dstProbabilities.size() == 1) {
        auto const [id, count] = *dstProbabilities.begin();
        if (srcProbabilities.contains(id)) {
          srcProbabilities.erase(id);
        }
      }
      if (srcProbabilities.size() == 1) {
        auto const [id, count] = *srcProbabilities.begin();
        if (dstProbabilities.contains(id)) {
          dstProbabilities.erase(id);
        }
      }
      double inputSum{std::accumulate(
          srcProbabilities.begin(),
          srcProbabilities.end(),
          0.,
          [](double acc, const auto& pair) { return acc + pair.second; })};
      nAgents = static_cast<dsm::Size>(inputSum);
      double outputSum{std::accumulate(
          dstProbabilities.begin(),
          dstProbabilities.end(),
          0.,
          [](double acc, const auto& pair) { return acc + pair.second; })};
      if (inputSum < 0 || outputSum < 0) {
        std::cout << std::format(
            "Negative input {} or output {} weight sum", inputSum, outputSum)<<std::endl;
        std::abort();
      }
      if (inputSum == 0) {
        auto const size = srcProbabilities.size();
        for (auto& [id, weight] : srcProbabilities) {
          weight = 1. / size;
        }
      }
      if (outputSum == 0) {
        auto const size = dstProbabilities.size();
        for (auto& [id, weight] : dstProbabilities) {
          weight = 1. / size;
        }
        std::cout << std::format("No output data for time {}, using uniform distribution.",
                             dynamics.time())<<std::endl;
      }
      ++current_index;
      if (srcProbabilities.empty()) {
        std::cout << std::format("No input data for time {}", dynamics.time())<<std::endl;
        nAgents = 0;
      } else {
        auto const oldValue = nAgents;
        auto const scaleFactor{static_cast<double>(GRANULARITY) / INTERVAL_AGENTS_IN};
        nAgents /= scaleFactor;
        std::cout << std::format(
            "Time: {}, nAgents: {} -> {}", dynamics.time(), oldValue, nAgents)<<std::endl;
      }
      if (dstProbabilities.empty()) {
        std::cout << std::format("No output data for time {}", dynamics.time())<<std::endl;
      }
    }
    // EVOLUTION   -   -   -

    if (progress % INTERVAL_AGENTS_IN == 0) {
      try {
        dynamics.addAgentsRandomly(nAgents, srcProbabilities, dstProbabilities);
      } catch (const std::exception& e) {
        std::cout << std::format("Error adding agents: {}", e.what())<<std::endl;
        std::cout << std::format("There are still {} agents in the system.",
                             dynamics.agents().size())<<std::endl;
        std::cout << std::format("Writing agent dump to file...")<<std::endl;
        std::ofstream agentDump(OUT_FOLDER + "agent_dump.csv");
        agentDump << "id;src;dst;street\n";
        for (auto const& [id, agent] : dynamics.agents()) {
          agentDump << id << ';' << agent->srcNodeId().value() << ';'
                    << agent->itineraryId() << ';';
          if (agent->streetId().has_value()) {
            agentDump << agent->streetId().value();
          }
          agentDump << std::endl;
        }
        bExitFlag = true;
        break;
      }
    }
    dynamics.evolve(false);

    if (OPTIMIZE && dynamics.time() % GRANULARITY == 0) {
      dynamics.optimizeTrafficLights(10, 0.1, 3. / 10);
    }

    // OUTPUTS   -   -   -

    if (dynamics.time() % GRANULARITY == 0) {
      const auto& meanSpeed{dynamics.streetMeanSpeed()};
      const auto& meanDensity{dynamics.streetMeanDensity(true)};
      const auto& meanFlow{dynamics.streetMeanFlow()};
      const auto& meanTravelTime{dynamics.meanTravelTime()};

      out << dynamics.time() << ';' << dynamics.agents().size() << ';' << meanSpeed.mean
          << ';' << meanSpeed.std << ';' << meanDensity.mean << ';' << meanDensity.std
          << ';' << meanFlow.mean << ';' << meanFlow.std << ';' << meanTravelTime.mean
          << ';' << meanTravelTime.std << std::endl;
    }
    if (dynamics.time() % GRANULARITY == 0) {
      outSpires << dynamics.time();
      for (const auto& [id, street] : dynamics.graph().streetSet()) {
        if (street->isSpire()) {
          auto& spire = dynamic_cast<dsm::SpireStreet&>(*street);
          outSpires << ';' << spire.outputCounts(true);
        } else {
          outSpires << ';';
        }
      }
      outSpires << std::endl;
    }

    if (dynamics.time() % GRANULARITY == 0) {
      streetDensity << dynamics.time();
      streetQueues << dynamics.time();
      for (const auto& [id, street] : dynamics.graph().streetSet()) {
        streetDensity << ';' << street->density(true);
        streetQueues << ';'
                     << static_cast<double>(street->nExitingAgents()) /
                            street->capacity();
      }
      streetDensity << std::endl;
      streetQueues << std::endl;
      nodeDensity << dynamics.time();
      for (const auto& [id, node] : dynamics.graph().nodeSet()) {
        nodeDensity << ';' << node->density();
      }
      nodeDensity << std::endl;
    }

    ++progress;
  }

  std::cout << std::format("Simulation ended at time {} / {}", dynamics.time(), MAX_TIME)<<std::endl;

  outSpires.close();
  streetDensity.close();
  streetQueues.close();
  nodeDensity.close();
  out.close();

  std::cout << std::format("Writing synthetic data to file...")<<std::endl;

  std::ofstream syntheticData(OUT_FOLDER + "synthetic_data.csv");
  syntheticData << "section;data\n";
  for (auto const& [id, data] : synthetic_data) {
    syntheticData << id << ';';
    for (auto const& d : data) {
      syntheticData << d << ' ';
    }
    syntheticData << std::endl;
  }
  syntheticData.close();

#ifdef __APPLE__
  t.join();
#endif

  return 0;
}