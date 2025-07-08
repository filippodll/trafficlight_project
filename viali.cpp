#include "dsf/dsf.hpp"
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

const std::string IN_COORDS{"./coordinates.dsf"};  // input coords file

using Dynamics = dsf::FirstOrderDynamics;
using Street = dsf::Street;
using TrafficLight = dsf::TrafficLight;

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
  // dsf::Logger::setLogLevel(dsf::log_level_t::DEBUG);
  if (argc != 11) {
    std::cerr
        << "Usage: " << argv[0]
        << " <SEED> <ALPHA> <DAY> <GRANULARITY> <DELAY> <DATA_FOLDER> <FLOW_PERCENTAGE> "
           "<OPTIMIZE> <LOCAL THRESHOLD> <NON-LOCAL THRESHOLD>\n";
    return 1;
  }

  int const SEED{std::stoi(argv[1])};             // seed for random number generator
  double const ALPHA{std::stod(argv[2])};         // alpha parameter for the dynamics
  std::string const DAY{argv[3]};                 // day of the week
  int const GRANULARITY{std::stoi(argv[4])};      // granularity of the data in seconds
  int const DELAY{std::stoi(argv[5])};            // delay in granularity
  std::string const DATA_FOLDER{argv[6]};         // folder containing the data files
  int const FLOW_PERCENTAGE{std::stoi(argv[7])};  // percentage of the maximum flow
  bool const OPTIMIZE{std::stoi(argv[8]) > 0};    // optimize the graph
  auto const LOCAL_THRESHOLD{std::stod(argv[9])};
  auto const NONLOCAL_THRESHOLD{std::stod(argv[10])};

  std::string const INPUT_FILE{std::format("{}/{}.csv", DATA_FOLDER, DAY)};
  std::string OUT_FOLDER{std::format("./output/{}", DAY)};
  auto optType = dsf::TrafficLightOptimization::SINGLE_TAIL;
  if (std::stoi(argv[8]) > 1) {
    optType = dsf::TrafficLightOptimization::DOUBLE_TAIL;
  }
  if (OPTIMIZE) {
    switch (optType) {
      case dsf::TrafficLightOptimization::SINGLE_TAIL:
        OUT_FOLDER += "-single/";
        break;
      case dsf::TrafficLightOptimization::DOUBLE_TAIL:
        OUT_FOLDER += "-double/";
        break;
    }
  } else {
    OUT_FOLDER += '/';
  }

  size_t const NDATAPOINTS{MAX_TIME / GRANULARITY};  // number of data points

  if (!fs::exists("./output")) {
    fs::create_directory("./output");
  }
  if (fs::exists(OUT_FOLDER)) {
    fs::remove_all(OUT_FOLDER);
  }
  fs::create_directory(OUT_FOLDER);
  if (fs::exists("./constants")) {
    fs::remove_all("./constants");
  }
  fs::create_directory("./constants");
  // if exists remove ./tlog.txt
  if (fs::exists("./tlog.txt")) {
    fs::remove("./tlog.txt");
  }

  std::ofstream sargs(std::format("{}/args.txt", OUT_FOLDER));
  sargs << "SEED: " << SEED << '\n';
  sargs << "ALPHA: " << ALPHA << '\n';
  sargs << "DAY: " << DAY << '\n';
  sargs << "GRANULARITY: " << GRANULARITY << '\n';
  sargs << "DELAY: " << DELAY << '\n';
  sargs << "DATA_FOLDER: " << DATA_FOLDER << '\n';
  sargs << "FLOW_PERCENTAGE: " << FLOW_PERCENTAGE << '\n';
  sargs << "OPTIMIZE: " << OPTIMIZE << '\n';
  sargs << "OPT_THRESHOLD: " << LOCAL_THRESHOLD << '\n';
  sargs << "NONLOCAL_THRESHOLD: " << NONLOCAL_THRESHOLD << '\n';
  sargs.close();

  dsf::Logger::info(std::format("Using DSM version {}", dsf::version()));
  std::cout << std::format("Output folder: {}", OUT_FOLDER) << std::endl;

  std::cout << "Creating road segments..." << std::endl;
  // segmenti viali
  Street s0_1{
      1, std::make_pair(0, 1), 500., 13.9, 3, "2.10 2.6 6 1"};  // (402) 2.10 2.6 6 1
  Street s1_0{
      2, std::make_pair(1, 0), 500., 13.9, 3, "2.6 2.10 6 1"};  // (499) 2.6 2.10 6 1

  Street s1_2{
      3, std::make_pair(1, 2), 400., 13.9, 3, "2.6 4.47 4 1"};  // (501) 2.6 4.47 4 1
  Street s2_1{
      4, std::make_pair(2, 1), 400., 13.9, 3, "4.47 2.6 8 1"};  // (820) 4.47 2.6 8 1

  Street s2_3{
      5, std::make_pair(2, 3), 550., 13.9, 3, "4.47 4.46 4 1"};  // (821) 4.47 4.46 4 1
  Street s3_2{
      6, std::make_pair(3, 2), 550., 13.9, 3, "4.46 4.47 8 1"};  // (819) 4.46 4.47 8 1

  Street s3_4{
      7, std::make_pair(3, 4), 260., 13.9, 3, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1
  Street s4_3{
      8, std::make_pair(4, 3), 260., 13.9, 3, "4.45 4.46 8 1"};  // (815) 4.45 4.46 8 1
  Street s4_5{
      9, std::make_pair(4, 5), 150., 13.9, 3, "4.46 4.45 4 1"};  // (818) 4.46 4.45 4 1

  Street s5_4{10, std::make_pair(5, 4), 150., 13.9, 3};

  Street s5_6{
      11, std::make_pair(5, 6), 300., 13.9, 3, "4.45 4.44 4 1"};  // (814) 4.45 4.44 4 1
  Street s6_5{
      12, std::make_pair(6, 5), 300., 13.9, 2, "4.44 4.45 8 1"};  // (812) 4.44 4.45 8 1

  Street s6_7{
      13, std::make_pair(6, 7), 700., 13.9, 3, "4.44 4.41 4 1"};  // (811) 4.44 4.41 4 1
  Street s7_6{
      14, std::make_pair(7, 6), 700., 13.9, 3, "4.41 4.44 8 1"};  // (801) 4.41 4.44 8 1

  Street s7_8{
      15, std::make_pair(7, 8), 230., 13.9, 3, "4.41 4.42 4 1"};  // (800) 4.41 4.42 4 1
  Street s8_7{
      16, std::make_pair(8, 7), 230., 13.9, 3, "4.42 4.41 8 1"};  // (803) 4.42 4.41 8 1

  // strade secondarie

  Street s9_1{17,
              std::make_pair(9, 1),
              750.,
              8.3,
              2,
              "2.5 2.6 2 1"};  // (496) 2.5 2.6 2 1      0.127 2.6 1 1 //saragozza (1)
  Street s1_9{
      18, std::make_pair(1, 9), 750., 8.3, 2, "2.6 2.5 6 1"};  // (500) 2.6 2.5 6 1
  Street s10_1{19, std::make_pair(10, 1), 250., 8.3, 1};

  Street s2_11{20, std::make_pair(2, 11), 300., 8.3, 1};  // vallescura  (2)
  Street s11_2{
      21, std::make_pair(11, 2), 300., 8.3, 1, "0.127 4.47 2 1"};  // (278) 0.127 4.47 2 1
  Street s12_2{22,
               std::make_pair(12, 2),
               160.,
               8.3,
               1,
               "0.127 4.47 6 1"};  // Malpertuso (279) 0.127 4.47 6 1

  Street s13_3{23,
               std::make_pair(13, 3),
               500.,
               8.3,
               2,
               "0.127 4.46 2 1"};  // (273) 0.127 4.46 2 1   (274) 0.127 4.46 3
                                   // 1    //san mamolo  (3)
  Street s3_13{
      24, std::make_pair(3, 13), 500., 8.3, 1, "4.46 0.127 6 1"};  // (816) 4.46 0.127 6 1
  Street s3_14{25, std::make_pair(3, 14), 240., 8.3, 1};
  Street s14_3{
      26, std::make_pair(14, 3), 240., 8.3, 1, "0.127 4.46 6 1"};  // (275) 0.127 4.46 6 1

  Street s4_15{27, std::make_pair(4, 15), 190., 8.3, 1};  // savenella  (3)

  Street s16_5{28,
               std::make_pair(16, 5),
               270.,
               8.3,
               1,
               "0.127 4.45 6 1"};  // (270) 0.127 4.45 6 1 //rubbiani   (4)

  Street s17_6{29,
               std::make_pair(17, 6),
               400.,
               8.3,
               1,
               "0.127 4.44 2 1"};  // (266) 0.127 4.44 2 1 //castiglione  (6)
  Street s6_17{30, std::make_pair(6, 17), 400., 8.3, 1};
  // Street s6_18{31, std::make_pair(6, 18), 200., 8.3, 1};
  Street s18_6{
      32, std::make_pair(18, 6), 200., 8.3, 1, "0.127 4.44 6 1"};  // (267) 0.127 4.44 6 1

  Street s19_7{33, std::make_pair(19, 7), 240., 8.3, 1};  // santo stefano (7)
  Street s7_19{
      34, std::make_pair(7, 19), 240., 8.3, 2, "4.41 4.33 6 1"};  // (799) 4.41 4.33 6 1
  Street s20_7{
      35, std::make_pair(20, 7), 350., 8.3, 1, "0.127 4.41 6 1"};  // (263) 0.127 4.41 6 1

  std::cout << "Creating traffic lights..." << std::endl;
  dsf::RoadNetwork graph;
  // saragozza
  graph.addNode<TrafficLight>(1, 127);
  auto& saragozza{graph.node<TrafficLight>(1)};
  saragozza.setStreetPriorities({s0_1.id(), s2_1.id()});
  saragozza.setCycle(s0_1.id(), dsf::Direction::STRAIGHT, {82, 0});
  // saragozza.setCycle(s0_1.id(), dsf::Direction::RIGHT, {112, 97});

  saragozza.setCycle(s2_1.id(), dsf::Direction::STRAIGHT, {82, 0});
  saragozza.setCycle(s2_1.id(), dsf::Direction::LEFT, {30, 82});

  // saragozza.setCycle(s9_1.id(), dsf::Direction::RIGHT, {127, 0});
  saragozza.setCycle(s9_1.id(), dsf::Direction::LEFT, {30, 97});

  saragozza.setCycle(s10_1.id(), dsf::Direction::ANY, {15, 82});

  // vallescura
  graph.addNode<TrafficLight>(2, 125);
  auto& vallescura{graph.node<TrafficLight>(2)};
  vallescura.setStreetPriorities({s1_2.id(), s3_2.id()});
  vallescura.setCycle(s1_2.id(), dsf::Direction::RIGHTANDSTRAIGHT, {78, 25});

  vallescura.setCycle(s3_2.id(), dsf::Direction::STRAIGHT, {100, 25});
  vallescura.setCycle(s3_2.id(), dsf::Direction::LEFT, {22, 103});

  vallescura.setCycle(s11_2.id(), dsf::Direction::ANY, {25, 0});
  vallescura.setCycle(s12_2.id(), dsf::Direction::ANY, {25, 0});
  // san mamolo
  graph.addNode<TrafficLight>(3, 155);
  auto& sanmamolo{graph.node<TrafficLight>(3)};
  sanmamolo.setStreetPriorities({s2_3.id(), s4_3.id()});
  sanmamolo.setCycle(s2_3.id(), dsf::Direction::RIGHTANDSTRAIGHT, {85, 0});
  s2_3.addForbiddenTurn(s3_14.id());

  sanmamolo.setCycle(s4_3.id(), dsf::Direction::RIGHTANDSTRAIGHT, {120, 0});
  sanmamolo.setCycle(s4_3.id(), dsf::Direction::LEFT, {35, 85});

  // sanmamolo.setCycle(s13_3.id(), dsf::Direction::RIGHT, {130, 25});
  sanmamolo.setCycle(s13_3.id(), dsf::Direction::LEFTANDSTRAIGHT, {35, 120});

  sanmamolo.setCycle(s14_3.id(), dsf::Direction::ANY, {35, 120});

  // savenella
  graph.addNode<TrafficLight>(4, 83);
  auto& savenella{graph.node<TrafficLight>(4)};
  savenella.setStreetPriorities({s3_4.id(), s5_4.id()});
  savenella.setCycle(s3_4.id(), dsf::Direction::STRAIGHT, {83, 0});
  savenella.setCycle(s3_4.id(), dsf::Direction::LEFT, {30, 0});

  savenella.setCycle(s5_4.id(), dsf::Direction::RIGHTANDSTRAIGHT, {53, 30});

  // rubbiani
  graph.addNode<TrafficLight>(5, 95);
  auto& rubbiani{graph.node<TrafficLight>(5)};
  rubbiani.setStreetPriorities({s4_5.id(), s6_5.id()});
  rubbiani.setCycle(s4_5.id(), dsf::Direction::STRAIGHT, {55, 0});
  rubbiani.setCycle(s6_5.id(), dsf::Direction::STRAIGHT, {55, 0});

  rubbiani.setCycle(s16_5.id(), dsf::Direction::ANY, {40, 55});

  // castiglione
  graph.addNode<TrafficLight>(6, 145);
  auto& castiglione{graph.node<TrafficLight>(6)};
  castiglione.setStreetPriorities({s5_6.id(), s7_6.id()});
  castiglione.setCycle(s5_6.id(), dsf::Direction::RIGHTANDSTRAIGHT, {60, 0});

  castiglione.setCycle(s7_6.id(), dsf::Direction::RIGHTANDSTRAIGHT, {85, 0});
  castiglione.setCycle(s7_6.id(), dsf::Direction::LEFT, {25, 60});

  castiglione.setCycle(s17_6.id(), dsf::Direction::ANY, {60, 85});
  castiglione.setCycle(s18_6.id(), dsf::Direction::ANY, {60, 85});

  // santo stefano
  graph.addNode<TrafficLight>(7, 115);
  auto& santostefano{graph.node<TrafficLight>(7)};
  santostefano.setStreetPriorities({s6_7.id(), s8_7.id()});
  santostefano.setCycle(s6_7.id(), dsf::Direction::RIGHT, {90, 0});
  santostefano.setCycle(s6_7.id(), dsf::Direction::STRAIGHT, {35, 0});

  santostefano.setCycle(s8_7.id(), dsf::Direction::RIGHTANDSTRAIGHT, {90, 35});
  santostefano.setCycle(s8_7.id(), dsf::Direction::LEFT, {55, 35});

  santostefano.setCycle(s19_7.id(), dsf::Direction::ANY, {25, 90});
  santostefano.setCycle(s20_7.id(), dsf::Direction::ANY, {25, 90});

  std::cout << "Adding streets..." << std::endl;
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
  std::cout << "Building adjacency matrix..." << std::endl;
  graph.buildAdj();
  std::cout << "Adjusting node capacities..." << std::endl;
  graph.adjustNodeCapacities();
  dsf::Logger::setLogLevel(dsf::log_level_t::DEBUG);
  graph.autoMapStreetLanes();
  dsf::Logger::setLogLevel(dsf::log_level_t::WARNING);
  // return 0;

  graph.exportNodes("./constants/coords.csv");

  std::cout << "Init input data preparation..." << std::endl;
  auto const& matrix{graph.adjacencyMatrix()};
  auto const n{matrix.n()};
  std::ofstream adj("./constants/adj.dat");
  adj << n << '\t' << n << '\n';
  for (unsigned int i = 0; i < n; ++i) {
    for (unsigned int j = 0; j < n; ++j) {
      adj << matrix(i, j) << '\t';
    }
    adj << '\n';
  }
  adj.close();

  std::map<std::string, dsf::Id> coilmap;
  std::ofstream dict("./structures_out.py");
  dict << "COIL_DICT = {" << std::endl;
  for (auto const& [id, street] : graph.edges()) {
    if (street->isSpire()) {
      dict << '\"' << street->name() << "\": " << id << ",\n";  // Python dictionary
      coilmap[street->name()] = id;
    }
  }
  dict << '}' << std::endl;
  // Now append a dict with streetId: street name
  dict << "NAME_DICT = {" << std::endl;
  for (auto const& [id, street] : graph.edges()) {
    dict << id << ": \"" << street->name() << "\",\n";  // Python dictionary
  }
  dict << '}' << std::endl;
  dict.close();
  // Create the dynamics
  Dynamics dynamics{graph, false, SEED, ALPHA};
  if (OPTIMIZE) {
    dynamics.setDataUpdatePeriod(INTERVAL_AGENTS_IN);
  }
  // dynamics.setSpeedFluctuationSTD(0.1);
  // dynamics.setMaxFlowPercentage(0.75);

  auto const& streets{dynamics.graph().edges()};
  // auto const& nodes{dynamics.graph().nodeSet()};

  std::cout << std::format("Importing input data...") << std::endl;
  std::ifstream ifs(INPUT_FILE);
  if (!ifs) {
    std::cout << std::format("Cannot open input file {}", INPUT_FILE) << std::endl;
    return 1;
  }
  std::string line;
  std::getline(ifs, line);  // skip header
  std::map<dsf::Id, data_t> input_data;
  std::map<dsf::Id, data_t> output_data;
  std::map<dsf::Id, data_t> inner_data;
  int iValue;
  std::set<dsf::Id> const inputCoils{
      1, 175, 190, 233, 254, 276, 297, 341, 363, 384, 427, /**/ 211, 406};
  std::set<dsf::Id> const outputCoils{21, 30, 76, 155, 166, /**/ 53, 77, 99, 143};
  std::set<dsf::Id> const innerCoils{23, 43, 45, 65, 67, 87, 89, 109, 111, 131, 133, 153};
  while (std::getline(ifs, line)) {
    std::istringstream iss(line);
    std::string token;

    std::getline(iss, token, ';');
    if (!coilmap.contains(token)) {
      std::cout << std::format("Unknown coil \"{}\". Skipping.", token) << std::endl;
      continue;
    }
    dsf::Id streetId = coilmap.at(token);
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
      for (size_t i = 0; i < NDATAPOINTS - 1; ++i) {
        iss >> iValue;
        if (iValue > 0) {
          inner_data[streetId][i] = iValue;
        } else {
          inner_data[streetId][i] = 0;
        }
      }
    }
  }
  ifs.close();

  std::cout << std::format("Input data imported") << std::endl;
  std::cout << std::format("Creating itineraries") << std::endl;

  std::vector<dsf::Id> outNodeList;
  outNodeList.reserve(output_data.size());
  for (auto const& id : outputCoils) {
    auto const& nid{streets.at(id)->nodePair().second};
    outNodeList.push_back(nid);
  }
  dynamics.setDestinationNodes(outNodeList);

  std::cout << std::format("Destination nodes set") << std::endl;

  // launch progress bar
  thread_t t([]() {
    while (progress < MAX_TIME && !bExitFlag) {
      printLoadingBar(progress, MAX_TIME);
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  });
  std::ofstream out(OUT_FOLDER + "data.csv");
  out << "time;n_agents;mean_speed;mean_speed_err;mean_density;mean_density_"
         "err;mean_flow;mean_flow_err;mean_traveltime;mean_traveltime_err;mean_"
         "travelspeed;mean_travelspeed_err;nGhosts\n";
  std::ofstream streetQueues(OUT_FOLDER + "queues.csv");
  streetQueues << "time";
  for (auto const& [id, street] : dynamics.graph().edges()) {
    streetQueues << ';' << id;
  }
  streetQueues << std::endl;

  size_t current_index{0};
  dsf::Size nAgents{0};

  std::map<dsf::Id, double> srcProbabilities, dstProbabilities;

  auto const& adjMatrix{dynamics.graph().adjacencyMatrix()};
  // auto const& degreeVector{adjMatrix.getDegreeVector()};

  // for (auto const& [id, value] : degreeVector) {
  //   if (value > 2) {
  //     continue;
  //   }
  //   nodes.at(id)->setTransportCapacity(std::numeric_limits<int16_t>::max());
  // }

  std::map<dsf::Id, data_t> synthetic_data;

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
      // std::unordered_map<dsf::Id, dsf::Size> synthetic_inner_data;
      for (auto const& [nodeId, node] : dynamics.graph().nodes()) {
        auto const& inputRoads{adjMatrix.getCol(nodeId)};
        auto const& outputRoads{adjMatrix.getRow(nodeId)};
        int inputCounts{0};
        int outputCounts{0};
        std::set<dsf::Id> missingInput;
        std::set<dsf::Id> missingOutput;
        // Input roads
        for (auto const& id : inputRoads) {
          auto const inputStreetId = id * adjMatrix.n() + nodeId;
          if (srcProbabilities.contains(id)) {
            inputCounts += srcProbabilities[id];
          } else if (inner_data.contains(inputStreetId)) {
            inputCounts += inner_data[inputStreetId][idx_in];
            // } else if (synthetic_inner_data.contains(inputStreetId)) {
            //   inputCounts += synthetic_inner_data[inputStreetId];
          } else {
            missingInput.emplace(inputStreetId);
          }
        }
        // Output roads
        for (auto const& id : outputRoads) {
          auto const outputStreetId = nodeId * adjMatrix.n() + id;
          if (dstProbabilities.contains(id)) {
            outputCounts += dstProbabilities[id];
          } else if (inner_data.contains(outputStreetId)) {
            outputCounts += inner_data[outputStreetId][idx_out];
            // } else if (synthetic_inner_data.contains(outputStreetId)) {
            //   outputCounts += synthetic_inner_data[outputStreetId];
          } else {
            missingOutput.emplace(outputStreetId);
          }
        }
        auto const deltaTOT{inputCounts - outputCounts};
        if (deltaTOT == 0) {
          continue;
          // } else {
          // std::cout << std::format(
          //     "Node {} has an overall delta of {}. Missing {} inputs and {} outputs.",
          //     nodeId,
          //     deltaTOT,
          //     missingInput.size(),
          //     missingOutput.size())<<std::endl;
        }
        ////////////////////////
        // Balance the nodes  //
        ////////////////////////
        // if (deltaTOT > 0) {
        //   // Input > Output ===> Add agents to output
        //   auto const deltaPerRoad{std::abs(static_cast<double>(deltaTOT)) /
        //                           missingOutput.size()};
        //   for (auto const& id : missingOutput) {
        //     if (outputCoils.contains(id)) {
        //       auto const nid = streets.at(id)->nodePair().second;
        //       if (!dstProbabilities.contains(nid)) {
        //         if (!synthetic_data.contains(id)) {
        //           synthetic_data[id] = data_t(NDATAPOINTS, 0);
        //         }
        //         dstProbabilities[nid] = deltaPerRoad;
        //         synthetic_data[id][current_index] = deltaPerRoad;
        //       }
        //     } else {
        //       if (synthetic_inner_data.contains(id)) {
        //         std::cout << std::format("Inner coil {} already has data", id)
        //                   << std::endl;
        //       }
        //       synthetic_inner_data[id] = deltaPerRoad;
        //     }
        //   }
        //   for (auto const& id : missingInput) {
        //     if (inputCoils.contains(id)) {
        //       auto const nid = streets.at(id)->nodePair().first;
        //       if (!srcProbabilities.contains(nid)) {
        //         if (!synthetic_data.contains(id)) {
        //           synthetic_data[id] = data_t(NDATAPOINTS, 0);
        //         }
        //         srcProbabilities[nid] = 0.;
        //         synthetic_data[id][current_index] = 0.;
        //       }
        //     } else {
        //       if (synthetic_inner_data.contains(id)) {
        //         std::cout << std::format("Inner coil {} already has data", id)
        //                   << std::endl;
        //       }
        //       synthetic_inner_data[id] = 0.;
        //     }
        //   }
        // } else if (deltaTOT < 0) {
        //   // Output > Input ===> Add agents to input
        //   auto const deltaPerRoad{std::abs(static_cast<double>(deltaTOT)) /
        //                           missingInput.size()};
        //   for (auto const& id : missingInput) {
        //     if (inputCoils.contains(id)) {
        //       auto const nid = streets.at(id)->nodePair().first;
        //       if (!srcProbabilities.contains(nid)) {
        //         srcProbabilities[nid] = deltaPerRoad;
        //         if (!synthetic_data.contains(id)) {
        //           synthetic_data[id] = data_t(NDATAPOINTS, 0);
        //         }
        //         synthetic_data[id][current_index] = deltaPerRoad;
        //       }
        //     } else {
        //       if (synthetic_inner_data.contains(id)) {
        //         std::cout << std::format("Inner coil {} already has data", id)
        //                   << std::endl;
        //       }
        //       synthetic_inner_data[id] = deltaPerRoad;
        //     }
        //   }
        //   for (auto const& id : missingOutput) {
        //     if (outputCoils.contains(id)) {
        //       auto const nid = streets.at(id)->nodePair().second;
        //       if (!dstProbabilities.contains(nid)) {
        //         dstProbabilities[nid] = 0.;
        //         if (!synthetic_data.contains(id)) {
        //           synthetic_data[id] = data_t(NDATAPOINTS, 0);
        //         }
        //         synthetic_data[id][current_index] = 0.;
        //       }
        //     } else {
        //       if (synthetic_inner_data.contains(id)) {
        //         std::cout << std::format("Inner coil {} already has data", id)
        //                   << std::endl;
        //       }
        //       synthetic_inner_data[id] = 0.;
        //     }
        //   }
        // }
      }
      if (dstProbabilities.size() == 1) {
        std::cout << "No buono, puoi arrivare in un nodo solo" << std::endl;
        auto const [id, count] = *dstProbabilities.begin();
        if (srcProbabilities.contains(id)) {
          srcProbabilities.erase(id);
        }
      }
      if (srcProbabilities.size() == 1) {
        std::cout << "No buono, puoi partire da un nodo solo" << std::endl;
        auto const [id, count] = *srcProbabilities.begin();
        if (dstProbabilities.contains(id)) {
          dstProbabilities.erase(id);
        }
      }
      double inputSum{std::accumulate(
          srcProbabilities.begin(),
          srcProbabilities.end(),
          0.,
          [](double acc, auto const& pair) { return acc + pair.second; })};
      nAgents = static_cast<dsf::Size>(inputSum);
      double outputSum{std::accumulate(
          dstProbabilities.begin(),
          dstProbabilities.end(),
          0.,
          [](double acc, auto const& pair) { return acc + pair.second; })};
      if (inputSum < 0 || outputSum < 0) {
        std::cout << std::format(
                         "Negative input {} or output {} weight sum", inputSum, outputSum)
                  << std::endl;
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
        std::cout << std::format(
                         "No output data for time {}, using uniform distribution with "
                         "probability {}.",
                         dynamics.time(),
                         1. / size)
                  << std::endl;
      }
      ++current_index;
      if (srcProbabilities.empty()) {
        std::cout << std::format("No input data for time {}", dynamics.time())
                  << std::endl;
        nAgents = 0;
      } else {
        auto const oldValue = nAgents;
        auto const scaleFactor{static_cast<double>(GRANULARITY) / INTERVAL_AGENTS_IN};
        nAgents *= FLOW_PERCENTAGE / 100.;
        nAgents /= scaleFactor;
        // std::cout << std::format("Time: {}, nAgents: {} -> {}",
        //                          dynamics.time(),
        //                          oldValue,
        //                          nAgents)
        //           << std::endl;
      }
      if (dstProbabilities.empty()) {
        std::cout << std::format("No output data for time {}", dynamics.time())
                  << std::endl;
      }
    }
    // EVOLUTION   -   -   -

    auto nGhosts{0};

    if (progress % INTERVAL_AGENTS_IN == 0) {
      auto const& agents{dynamics.agents()};
      // nGhosts = std::count_if(agents.begin(), agents.end(), [](auto const& agent) {
      //   return !agent.second->streetId().has_value();
      // });
      try {
        dynamics.addAgentsRandomly(nAgents, srcProbabilities, dstProbabilities);
      } catch (const std::exception& e) {
        std::cout << std::format("Error adding agents: {}", e.what()) << std::endl;
        std::cout << std::format("There are still {} agents in the system.",
                                 dynamics.agents().size())
                  << std::endl;
        std::cout << std::format("Writing agent dump to file...") << std::endl;
        // std::ofstream agentDump(OUT_FOLDER + "agent_dump.csv");
        // agentDump << "id;src;dst;delay;street\n";
        // for (auto const& [id, agent] : dynamics.agents()) {
        //   agentDump << id << ';' << agent->srcNodeId().value() << ';'
        //             << agent->itineraryId() << ';';
        //   agentDump << static_cast<int>(agent->delay()) << ';';
        //   if (agent->streetId().has_value()) {
        //     agentDump << agent->streetId().value();
        //   }
        //   agentDump << std::endl;
        // }
        bExitFlag = true;
        break;
      }
    }
    dynamics.evolve(false);

    if (OPTIMIZE && dynamics.time() % 300 == 0) {
      dynamics.optimizeTrafficLights(optType, "./tlog.txt", LOCAL_THRESHOLD, NONLOCAL_THRESHOLD);  // 0.3, NEAREST_NEIGHBOUR
    }

    // OUTPUTS   -   -   -

    if (dynamics.time() % GRANULARITY == 0) {
      // std::pair<dsf::Id, dsf::Size> maxQueue{0, 0};
      // std::clog << "Time: " << dynamics.time() << std::endl;
      // for (auto const& [id, street] : dynamics.graph().edges()) {
      //   std::clog << "Street " << id << '\t';
      //   for (auto i{0}; i < street->nLanes(); ++i) {
      //     auto const& queue{street->queue(i)};
      //     // if (queue.size() > maxQueue.second) {
      //     //   maxQueue = {id, queue.size()};
      //     // }
      //     std::clog << queue.size() << ' ';
      //   }
      //   std::clog << std::endl;
      // }
      // std::clog << "Max queue: " << maxQueue.first << " with " << maxQueue.second
      //           << " agents" << std::endl;
      auto const& meanSpeed{dynamics.streetMeanSpeed()};
      auto const& meanDensity{dynamics.streetMeanDensity(false)};
      auto const& meanFlow{dynamics.streetMeanFlow()};
      auto const& meanTravelTime{dynamics.meanTravelTime()};
      auto const& meanTravelSpeed{dynamics.meanTravelSpeed()};

      // Count agents if they have streetId == std::nullopt

      out << dynamics.time() << ';' << dynamics.agents().size() << ';' << meanSpeed.mean
          << ';' << meanSpeed.std << ';' << meanDensity.mean << ';' << meanDensity.std
          << ';' << meanFlow.mean << ';' << meanFlow.std << ';' << meanTravelTime.mean
          << ';' << meanTravelTime.std << ';' << meanTravelSpeed.mean << ';'
          << meanTravelSpeed.std << ';' << nGhosts << std::endl;
      dynamics.saveTravelSpeeds(OUT_FOLDER + "speeds.csv", true);
    }
    if (dynamics.time() % GRANULARITY == 0) {
      dynamics.saveOutputStreetCounts(OUT_FOLDER + "output_counts.csv", true);
    }

    if (dynamics.time() % GRANULARITY == 0) {
      dynamics.saveStreetDensities(OUT_FOLDER + "densities.csv");
      streetQueues << dynamics.time();
      for (auto const& [id, street] : dynamics.graph().edges()) {
        streetQueues << ';'
                     << static_cast<double>(street->nExitingAgents()) /
                            street->capacity();
      }
      streetQueues << std::endl;
    }

    ++progress;
  }

  std::cout << std::format("Simulation ended at time {} / {}", dynamics.time(), MAX_TIME)
            << std::endl;
  std::cout << std::format("There are still {} agents in the system.",
                           dynamics.agents().size())
            << std::endl;

  streetQueues.close();
  out.close();

  std::cout << std::format("Writing synthetic data to file...") << std::endl;

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