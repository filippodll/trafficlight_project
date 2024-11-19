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

std::atomic<int> progress{0};
std::atomic<bool> bExitFlag{false};

const std::string IN_COORDS{"./coordinates.dsm"};  // input coords file

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
  std::cout << "Loading: " << std::setprecision(2) << std::fixed << (i * 100. / n) << "%"
            << '\r';
  std::cout.flush();
}

size_t constexpr MAX_TIME{86400};  // maximum time of simulation

typedef std::vector<size_t> data_t;  // data type

#ifdef __APPLE__
typedef std::thread thread_t;
#else
typedef std::jthread thread_t;
#endif

int main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0]
              << " <SEED> <DAY> <GRANULARITY> <DELAY> <DATA_FOLDER>\n";
    return 1;
  }

  int const SEED{std::stoi(argv[1])};         // seed for random number generator
  std::string const DAY{argv[2]};             // day of the week
  int const GRANULARITY{std::stoi(argv[3])};  // granularity of the data in seconds
  int const DELAY{std::stoi(argv[4])};        // delay in granularity
  std::string const DATA_FOLDER{argv[5]};     // folder containing the data files

  std::string const INPUT_FILE{std::format("{}/{}.csv", DATA_FOLDER, DAY)};
  std::string const OUT_FOLDER{std::format("./{}/", DAY)};

  size_t const NDATAPOINTS{MAX_TIME / GRANULARITY};  // number of data points

  if (fs::exists(OUT_FOLDER)) {
    fs::remove_all(OUT_FOLDER);
  }
  fs::create_directory(OUT_FOLDER);
  std::cout << std::format("Using dsm version: {}\n", dsm::version());

  // Create the graph

  TrafficLight tl1{1}, tl2{2}, tl3{3}, tl4{4}, tl5{5}, tl6{6}, tl7{7};

  // segmenti viali
  Street s0_1{
      1, 1, 480., 13.9, std::make_pair(0, 1), 3, "2.10 2.6 6 1"};  // (402) 2.10 2.6 6 1
  Street s1_0{
      2, 1, 480., 13.9, std::make_pair(1, 0), 3, "2.6 2.10 6 1"};  // (499) 2.6 2.10 6 1

  Street s1_2{
      3, 1, 386., 13.9, std::make_pair(1, 2), 3, "2.6 4.47 4 1 "};  // (501) 2.6 4.47 4 1
  Street s2_1{
      4, 1, 386., 13.9, std::make_pair(2, 1), 3, "4.47 2.6 8 1"};  // (820) 4.47 2.6 8 1

  Street s2_3{
      5, 1, 532., 13.9, std::make_pair(2, 3), 3, "4.47 4.46 4 1 "};  // (821) 4.47 4.46 4 1
  Street s3_2{
      6, 1, 532., 13.9, std::make_pair(3, 2), 3, "4.46 4.47 8 1"};  // (819) 4.46 4.47 8 1

  Street s3_4{
      7, 1, 237., 13.9, std::make_pair(3, 4), 1, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1
  Street s4_3{
      8, 1, 237., 13.9, std::make_pair(4, 3), 3, "4.45 4.46 8 1"};  // (815) 4.45 4.46 8 1
  Street s3_5{
      9, 1, 375., 13.9, std::make_pair(3, 5), 3, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1

  Street s5_4{10, 1, 135., 13.9, std::make_pair(5, 4), 3};

  Street s5_6{
      11, 1, 230., 13.9, std::make_pair(5, 6), 3, "4.45 4.44 4 1"};  // (814) 4.45 4.44 4 1
  Street s6_5{
      12, 1, 230., 13.9, std::make_pair(6, 5), 3, "4.44 4.45 8 1"};  // (812) 4.44 4.45 8 1

  Street s6_7{
      13, 1, 653., 13.9, std::make_pair(6, 7), 3, "4.44 4.41 4 1"};  // (811) 4.44 4.41 4 1
  Street s7_6{
      14, 1, 653., 13.9, std::make_pair(7, 6), 3, "4.41 4.44 8 1"};  // (801) 4.41 4.44 8 1

  Street s7_8{
      15, 1, 230., 13.9, std::make_pair(7, 8), 3, "4.41 4.42 4 1"};  // (800) 4.41 4.42 4 1
  Street s8_7{
      16, 1, 230., 13.9, std::make_pair(8, 7), 3, "4.42 4.41 8 1"};  // (803) 4.42 4.41 8 1

  // strade secondarie

  Street s9_1{17,
              1,
              250.,
              8.3,
              std::make_pair(9, 1),
              1,
              "2.5 2.6 2 1"};  // (496) 2.5 2.6 2 1      0.127 2.6 1 1 //saragozza (1)
  Street s1_9{
      18, 1, 250., 8.3, std::make_pair(1, 9), 2, "2.6 2.5 6 1"};  // (500) 2.6 2.5 6 1
  Street s10_1{19, 1, 250., 8.3, std::make_pair(10, 1), 1};

  Street s2_11{20, 1, 100., 8.3, std::make_pair(2, 11), 1};  // vallescura  (2)
  Street s11_2{21,
               1,
               100.,
               8.3,
               std::make_pair(11, 2),
               1,
               "0.127 4.47 2 1"};  // (278) 0.127 4.47 2 1
  Street s12_2{22,
               1,
               100.,
               8.3,
               std::make_pair(12, 2),
               1,
               "0.127 4.47 6 1"};  // (279) 0.127 4.47 6 1

  Street s13_3{23,
               1,
               200.,
               8.3,
               std::make_pair(13, 3),
               1,
               "0.127 4.46 2 1 "};  // (273) 0.127 4.46 2 1   (274) 0.127 4.46 3
                                    // 1    //san mamolo  (3)
  Street s3_13{24,
               1,
               200.,
               8.3,
               std::make_pair(3, 13),
               1,
               "4.46 0.127 6 1"};  // (816) 4.46 0.127 6 1
  Street s3_14{25, 1, 100., 8.3, std::make_pair(3, 14), 1};
  Street s14_3{26,
               1,
               100.,
               8.3,
               std::make_pair(14, 3),
               1,
               "0.127 4.46 6 1"};  // (275) 0.127 4.46 6 1

  Street s4_15{27, 1, 90., 8.3, std::make_pair(4, 15), 1};  // savenella  (3)

  Street s16_5{28,
               1,
               90.,
               8.3,
               std::make_pair(16, 5),
               1,
               "0.127 4.45 6 1"};  // (270) 0.127 4.45 6 1 //rubbiani   (4)

  Street s17_6{29,
               1,
               200.,
               8.3,
               std::make_pair(17, 6),
               1,
               "0.127 4.44 2 1"};  // (266) 0.127 4.44 2 1 //castiglione  (6)
  Street s6_17{30, 1, 200., 8.3, std::make_pair(6, 17), 1};
  Street s6_18{31, 1, 100., 8.3, std::make_pair(6, 18), 1};
  Street s18_6{32,
               1,
               100.,
               8.3,
               std::make_pair(18, 6),
               1,
               "0.127 4.44 6 1"};  // (267) 0.127 4.44 6 1

  Street s19_7{33, 1, 215., 8.3, std::make_pair(19, 7), 1};  // santo stefano (7)
  Street s7_19{
      34, 1, 215., 8.3, std::make_pair(7, 19), 2, "4.41 4.33 6 1"};  // (799) 4.41 4.33 6 1
  Street s20_7{35,
               1,
               200.,
               8.3,
               std::make_pair(20, 7),
               1,
               "0.127 4.41 6 1"};  // (263) 0.127 4.41 6 1

  // saragozza
  tl1.setDelay(std::make_pair(62, 40));  // 40, 70
  tl1.setLeftTurnRatio(1. / 3);
  tl1.setCapacity(1);
  tl1.addStreetPriority(s0_1.id());
  tl1.addStreetPriority(s2_1.id());
  // vallescura
  tl2.setDelay(std::make_pair(72, 39));  // 50, 75
  tl2.setLeftTurnRatio(1. / 3);
  tl2.setCapacity(1);
  tl2.addStreetPriority(s1_2.id());
  tl2.addStreetPriority(s3_2.id());
  // san mamolo
  tl3.setDelay(std::make_pair(88, 50));  // 40, 70
  tl3.setLeftTurnRatio(1. / 3);
  tl3.setCapacity(1);
  tl3.addStreetPriority(s2_3.id());
  tl2.addStreetPriority(s4_3.id());
  // savenella
  tl4.setDelay(std::make_pair(100, 15));  // 38, 106 = 144
  tl4.setCapacity(1);

  tl4.addStreetPriority(s5_4.id());
  // rubbiani
  tl5.setDelay(std::make_pair(82, 39));  // 50, 75
  tl5.setLeftTurnRatio(1. / 3);
  tl5.setCapacity(1);
  tl5.addStreetPriority(s3_5.id());
  tl5.addStreetPriority(s6_5.id());
  // castiglione
  tl6.setDelay(std::make_pair(78, 45));  // 40, 70
  tl6.setLeftTurnRatio(1. / 3);
  tl6.setCapacity(1);
  tl6.addStreetPriority(s5_6.id());
  tl6.addStreetPriority(s7_6.id());
  // santo stefano
  tl7.setDelay(std::make_pair(81, 40));  // 38, 106 = 144
  tl7.setLeftTurnRatio(1. / 3);
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
  graph.addStreets(s0_1,
                   s1_0,
                   s1_2,
                   s2_1,
                   s2_3,
                   s3_2,
                   s3_4,
                   s4_3,
                   s3_5,
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

  graph.exportCoordinates(OUT_FOLDER + "coords.csv");

  auto const& matrix{graph.adjMatrix()};
  auto const n{matrix.getColDim()};
  std::ofstream adj(OUT_FOLDER + "adj.dat");
  adj << n << '\t' << n << '\n';
  for (auto i = 0; i < n; ++i) {
    for (auto j = 0; j < n; ++j) {
      adj << matrix(i, j) << '\t';
    }
    adj << '\n';
  }
  adj.close();

  std::unordered_map<std::string_view, Unit> coilmap;
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
  dynamics.setSpeedFluctuationSTD(0.2);

  auto const& streets{dynamics.graph().streetSet()};

  std::cout << "Importing input data" << std::endl;
  std::ifstream ifs(INPUT_FILE);
  if (!ifs) {
    std::cerr << "Error opening file " << INPUT_FILE << '\n';
    return 1;
  }
  std::string line;
  std::getline(ifs, line);  // skip header
  std::map<Unit, data_t> input_data;
  std::map<Unit, data_t> output_data;
  int iValue;
  std::set<Unit> inputCoils{1, 175, 190, 233, 254, 276, 297, 341, 363, 384, 427};
  std::set<Unit> outputCoils{21, 30, 76, 155, 166};
  while (std::getline(ifs, line)) {
    std::istringstream iss(line);
    std::string token;

    std::getline(iss, token, ';');
    Unit streetId = coilmap.at(token);
    if (inputCoils.contains(streetId)) {
      auto const nodeId{streets.at(streetId)->nodePair().first};
      input_data[nodeId] = data_t(NDATAPOINTS, 0);
      for (auto i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        iValue > 0 ? input_data[nodeId][i] = iValue : input_data[nodeId][i] = 0;
      }
    } else if (outputCoils.contains(streetId)) {
      auto const nodeId{streets.at(streetId)->nodePair().second};
      output_data[nodeId] = data_t(NDATAPOINTS, 0);
      for (auto i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        iValue > 0 ? output_data[nodeId][i] = iValue : output_data[nodeId][i] = 0;
      }
    }
  }
  ifs.close();

  std::cout << "Input data imported" << std::endl;
  std::cout << "Creating itineraries" << std::endl;

  // create a vector from 0 to 20
  std::vector<Unit> outNodeList;
  outNodeList.reserve(output_data.size());
  for (const auto& [id, _] : output_data) {
    outNodeList.push_back(id);
  }
  dynamics.setDestinationNodes(outNodeList);

  std::cout << "Itineraries created" << std::endl;

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
  streetDensity << "time";
  for (const auto& [id, street] : dynamics.graph().streetSet()) {
    streetDensity << ';' << id;
  }
  streetDensity << std::endl;

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

  std::map<Unit, double> srcProbabilities, dstProbabilities;

  while (progress < MAX_TIME) {
    if (progress % 300 == 0 && current_index < NDATAPOINTS - 1) {
      srcProbabilities.clear();
      dstProbabilities.clear();
      double sum = 0.;
      size_t const idx_in{current_index};
      size_t const idx_out{(current_index + 1) % NDATAPOINTS};
      for (auto const& [id, data] : input_data) {
        srcProbabilities[id] = data[idx_in];
        sum += data[idx_in];
      }
      nAgents = static_cast<dsm::Size>(sum);
      if (sum > 1) {
        for (auto& [id, count] : srcProbabilities) {
          count /= sum;
        }
      } else {
        auto const size = input_data.size();
        for (auto const& [id, _] : input_data) {
          srcProbabilities[id] = 1. / size;
        }
      }
      sum = 0.;
      for (auto const& [id, data] : output_data) {
        dstProbabilities[id] = data[idx_out];
        sum += data[idx_out];
      }
      if (sum > 1) {
        for (auto& [id, count] : dstProbabilities) {
          count /= sum;
        }
      } else {
        auto const size = output_data.size();
        for (auto const& [id, _] : output_data) {
          dstProbabilities[id] = 1. / size;
        }
      }
      ++current_index;
      nAgents = nAgents < 10 ? 1 : nAgents /= 10;
    }
    // EVOLUTION   -   -   -

    if (progress % 30 == 0) {
      try {
        dynamics.addAgentsRandomly(nAgents, srcProbabilities, dstProbabilities);
      } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        for (auto const& [id, agent] : dynamics.agents()) {
          std::cout << "Agent ID " << id << " srcNodeID " << agent->srcNodeId().value()
                    << " dstNodeID " << agent->itineraryId();
          if (agent->streetId().has_value()) {
            std::cout << " streetID " << agent->streetId().value();
          }
          std::cout << std::endl;
        }
        bExitFlag = true;
        break;
      }
    }
    dynamics.evolve(false);

    // OUTPUTS   -   -   -

    if (dynamics.time() % 30 == 0) {
      const auto& meanSpeed{dynamics.streetMeanSpeed()};
      const auto& meanDensity{dynamics.streetMeanDensity(true)};
      const auto& meanFlow{dynamics.streetMeanFlow()};
      const auto& meanTravelTime{dynamics.meanTravelTime()};

      out << dynamics.time() << ';' << dynamics.agents().size() << ';' << meanSpeed.mean
          << ';' << meanSpeed.std << ';' << meanDensity.mean << ';' << meanDensity.std
          << ';' << meanFlow.mean << ';' << meanFlow.std << ';' << meanTravelTime.mean
          << ';' << meanTravelTime.std << std::endl;
    }
    if (dynamics.time() % 300 == 0) {
      outSpires << dynamics.time();
      for (const auto& [id, street] : dynamics.graph().streetSet()) {
        if (street->isSpire()) {
          auto& spire = dynamic_cast<SpireStreet&>(*street);
          outSpires << ';' << spire.outputCounts(true);
        } else {
          outSpires << ';';
        }
      }
      outSpires << std::endl;
    }

    if (dynamics.time() % 10 == 0) {
      streetDensity << dynamics.time();
      for (const auto& [id, street] : dynamics.graph().streetSet()) {
        streetDensity << ';' << street->density(true);
      }
      streetDensity << std::endl;
      nodeDensity << dynamics.time();
      for (const auto& [id, node] : dynamics.graph().nodeSet()) {
        nodeDensity << ';' << node->density();
      }
      nodeDensity << std::endl;
    }

    ++progress;
  }

  std::cout << "There are still " << dynamics.agents().size() << " agents in the system."
            << std::endl;

  outSpires.close();
  streetDensity.close();
  nodeDensity.close();
  out.close();

#ifdef __APPLE__
  t.join();
#endif

  return 0;
}