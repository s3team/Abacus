/*************************************************************************
	> File Name: QIF_EXP.cpp
	> Author: 
	> Mail: 
	> Created Time: Wed Oct 30 21:07:05 2019
 ************************************************************************/

#include<iostream>
#include <memory>
#include <chrono>
#include <cmath>
#include <cstring>
#include <tuple>
#include <iomanip>
#include "MonteCarlo.hpp"
#include "ins_parser.hpp"
#include "QIFSEEngine.hpp"
#include "Function.hpp"
#include "Trace2ELF.hpp"
#include "InputParser.hpp"


using namespace std::chrono;
using namespace std;
using namespace tana;

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
        cout << "Usage: " << argv[0] << " <traces.txt> " << "<options>" << "\n";
        cout << left << setw(40) << "-o <file>" << setw(30) << "Place the output into <file>" << endl;

        return 1;
    }

    std::shared_ptr<Function> func = nullptr;
    std::shared_ptr<Trace2ELF> t2e = nullptr;
    std::vector<std::tuple<uint32_t, uint32_t >> key_symbol;
    InputParser input(argc, argv);

    std::string traceFileName(argv[1]);
    std::string sep("/");
    auto fileSegment = split(traceFileName, sep);
    auto fileName = "result_" + fileSegment.rbegin()[1] + fileSegment.back();

    if (input.cmdOptionExists("-o")) {
        const std::string &output_name = input.getCmdOption("-o");
        fileName = output_name;
    }


    vector<unique_ptr<Inst_Base>> inst_list;
    vector<uint8_t> key_value;

    uint64_t max_instruction, each_step;
    vector<double> se_time_vector;
    max_instruction = 1000000;
    each_step = 20000;

    for(uint64_t start_ins = 0; start_ins <= max_instruction; start_ins +=each_step ) {


        auto start = high_resolution_clock::now();

        ifstream trace_file(argv[1]);
        if (!trace_file.is_open()) {
            cout << "Can't open files" << endl;
            return 1;
        }

        parse_trace_qif(trace_file, key_symbol, inst_list, key_value, func, start_ins);

        for (const auto &key_item : key_symbol) {

            std::cout << "Key Start Address: " << std::hex << get<0>(key_item) << std::dec
                      << " Length: " << get<1>(key_item) << std::endl;
        }

        uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp;
        auto reg = (inst_list.front())->vcpu;
        eax = reg.gpr[0];
        ebx = reg.gpr[1];
        ecx = reg.gpr[2];
        edx = reg.gpr[3];
        esi = reg.gpr[4];
        edi = reg.gpr[5];
        esp = reg.gpr[6];
        ebp = reg.gpr[7];

        auto *se = new QIFSEEngine(eax, ebx, ecx, edx, esi, edi, esp, ebp);
        if (func != nullptr) {
            se->init(inst_list.begin(), inst_list.end(), key_symbol, key_value, func);
        } else {
            se->init(inst_list.begin(), inst_list.end(), key_symbol, key_value);

        }
        se->run();

        auto time_se = high_resolution_clock::now();
        auto se_duration = duration_cast<microseconds>(time_se - start);

        double now_second = (se_duration.count()) / 1000000.0;

        se_time_vector.push_back(now_second);

        trace_file.close();
    }

    std::ofstream se_time_file;
    se_time_file.open(fileName + ".csv");
    for (auto &time:se_time_vector )
    {
        se_time_file << time << ",";
    }
    se_time_file.close();

    return 1;


}