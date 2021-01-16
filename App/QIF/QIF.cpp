#include "Function.hpp"
#include "InputParser.hpp"
#include "MonteCarlo.hpp"
#include "QIFSEEngine.hpp"
#include "Trace2ELF.hpp"
#include "ins_parser.hpp"
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>

using namespace std::chrono;
using namespace std;
using namespace tana;

float getEntropy(
    uint64_t MonteCarloTimes,
    std::vector<
        std::tuple<uint32_t, std::shared_ptr<tana::Constrain>, LeakageType>>
        constrains,
    const std::string &fileName, std::shared_ptr<DynamicFunction> func,
    std::map<int, uint32_t> key_value_map, std::shared_ptr<Trace2ELF> t2e) {

  FastMonteCarlo res(MonteCarloTimes, constrains, func, key_value_map);
  res.verifyConstrain();
  res.run();
  res.run_addr_group();
  res.infoConstrains(fileName);
  res.print_group_result(fileName, t2e);
  float MonteCarloResult = res.getResult();

  return abs(-log(MonteCarloResult) / log(2));
}

std::vector<std::string> split(std::string str, std::string sep) {
  char *cstr = const_cast<char *>(str.c_str());
  char *current;
  std::vector<std::string> arr;
  current = strtok(cstr, sep.c_str());
  while (current != nullptr) {
    arr.emplace_back(current);
    current = strtok(nullptr, sep.c_str());
  }
  return arr;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {

    string build_time(__TIMESTAMP__);

#ifdef NDEBUG
    cout << "Build Time: " << build_time << "\n";
    cout << "Release Build\n";
#else
    cout << "Build Time: " << build_time << "\n";
    cout << "Debug build\n";
#endif
    cout << "Usage: " << argv[0] << " <traces.txt> "
         << "<options>"
         << "\n";
    cout << left << setw(40) << "-o <file>" << setw(30)
         << "Place the output into <file>" << endl;
    cout << left << setw(40) << "-f <Function Name> " << setw(30)
         << "The location of the function file" << endl;
    cout << left << setw(40) << "-d <Debug Info> -f <Function Name> "
         << setw(30)
         << "The location of the ELF and the location of the function name"
         << endl;
    cout << left << setw(40) << "-t <Monte Carlo Times> " << setw(30)
         << "Specify the Monte Carlo times" << endl;

    return 1;
  }

  std::shared_ptr<DynamicFunction> func = nullptr;
  std::shared_ptr<Trace2ELF> t2e = nullptr;
  uint64_t MonteCarloTimes = 10000;
  InputParser input(argc, argv);

  std::string traceFileName(argv[1]);
  std::string sep("/");
  auto fileSegment = split(traceFileName, sep);
  auto fileName = "result_" + fileSegment.rbegin()[1] + fileSegment.back();
  bool cmdStatus = false;

  if (input.cmdOptionExists("-t")) {
    const std::string &mc_times = input.getCmdOption("-t");
    stringstream strValue;
    strValue << mc_times;
    uint64_t temp;
    strValue >> temp;
    MonteCarloTimes = temp;
    cmdStatus = true;
  }

  if (input.cmdOptionExists("-f") && input.cmdOptionExists("-d")) {

    const std::string &fun_name = input.getCmdOption("-f");
    func = std::make_shared<DynamicFunction>(fun_name);

    const std::string &obj_name = input.getCmdOption("-d");
    t2e = std::make_shared<Trace2ELF>(obj_name, fun_name);
    cmdStatus = true;
  }

  if (input.cmdOptionExists("-f")) {
    const std::string &fun_name = input.getCmdOption("-f");
    func = std::make_shared<DynamicFunction>(fun_name);
    cmdStatus = true;
  }

  if (input.cmdOptionExists("-o")) {
    const std::string &output_name = input.getCmdOption("-o");
    fileName = output_name;
    cmdStatus = true;
  }

  if ((!input.cmdOptionExists("-f")) && input.cmdOptionExists("-d")) {
    cout << "Error: \n Option -d must be used with -f together\n";
    exit(0);
  }

  ifstream trace_file(argv[1]);
  if (!trace_file.is_open()) {
    cout << "Can't open files" << endl;
    return 1;
  }

  if (!cmdStatus) {
    cout << "Incorrect Input Option \n";
    exit(0);
  }

  vector<unique_ptr<Inst_Base>> inst_list;

  auto start = high_resolution_clock::now();

  auto inst_size = file_inst_num(trace_file);
  std::cout << "Total Instructions:: " << inst_size << std::endl;
  trace_file.close();
  trace_file.open(argv[1]);

  parse_trace_qif(trace_file, inst_list, inst_size, func);

  auto *se = new QIFSEEngine();
  if (func != nullptr) {
    se->init(inst_list.begin(), inst_list.end(), func);
  } else {
    se->init(inst_list.begin(), inst_list.end(), nullptr);
  }
  se->run();

  auto time_se = high_resolution_clock::now();
  auto se_duration = duration_cast<microseconds>(time_se - start);

  se->reduceConstrains();
  se->printConstrains();

  auto key_value_map = se->return_key_value_map();
  auto constraints = se->getConstraints();

  std::cout << "Start Monte Carlo:" << std::endl;
  std::cout << getEntropy(MonteCarloTimes, constraints, fileName, func,
                          key_value_map, t2e)
            << std::endl;

  auto stop = high_resolution_clock::now();

  auto duration = duration_cast<microseconds>(stop - start);

  cout << "Time taken by SE: "
       << static_cast<double>(se_duration.count()) / 1000000 << " seconds"
       << endl;
  cout << "Time taken by QIF: "
       << static_cast<double>(duration.count()) / 1000000 << " seconds" << endl;

  ofstream result_file;
  result_file.open(fileName, ios_base::app);

  result_file << "Total Instructions:: " << inst_size << std::endl;

  result_file << "Time taken by SE: "
              << static_cast<double>(se_duration.count()) / 1000000
              << " seconds" << endl;

  result_file << "Time taken by QIF: "
              << static_cast<double>(duration.count()) / 1000000 << " seconds"
              << endl;

  result_file.close();

  return 0;
}