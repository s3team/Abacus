/*****
    > File Name: DynSEEngine.cpp
    > Author: Qinkun Bao & Zihao Wang
    > Mail: qinkunbao@gmail.com
    > Created Time: Tue Sep  3 17:19:02 EDT 2019
 ************************************************************************/

#include "DebugInfo.hpp"

#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;
using namespace tana;

DebugInfo::DebugInfo(std::ifstream &debug_file)
{
    char buffer[256];
    string fPwd;
    string fileName;
    uint32_t binaryAddress;
    int lineNumber;

    if (!debug_file.is_open())
    {
        cout << "DebugInfo: Error opening debug file";
        exit(1);
    }

    // Find first line end with ':'
    while (debug_file.getline(buffer, 256) &&
           (buffer[strlen(buffer) - 1] != ':') &&
           debug_file.good())
    {
    }

    debug_file >> fileName;
    // Read content
    while (debug_file.good())
    {
        fPwd = fileName;
        if (fPwd[0] == 'C')
        {
            // Skip 'CU:'
            debug_file >> fPwd;
            // Skip table header
            debug_file.getline(buffer, 256);
            debug_file.getline(buffer, 256);
        }

        if (fPwd[fPwd.length() - 1] == ':')
        {
            fPwd.pop_back();
        }
        else
        {
            fPwd.resize(fPwd.size() - 5);
        }

        debug_file >> fileName;
        while (debug_file.good() && fileName[fileName.length() - 1] != ':' && fileName[fileName.length() - 1] != ']')
        {
            debug_file >> dec >> lineNumber >> hex >> binaryAddress;
            auto ds = make_shared<DebugSymbol>(fPwd, fileName, binaryAddress, lineNumber);
            line_info.insert(
                upper_bound(line_info.begin(), line_info.end(), ds, DebugSymbol::LessThan), ds);
            ds.reset();
            debug_file >> fileName;
        }
    }
}

DebugInfo::DebugInfo(std::string &debug_string) {
    stringstream debug_info;
    debug_info << debug_string;

    char buffer[256];
    string fPwd;
    string fileName;
    uint32_t binaryAddress;
    int lineNumber;


    // Find first line end with ':'
    while (debug_info.getline(buffer, 256) &&
           (buffer[strlen(buffer) - 1] != ':') &&
           debug_info.good())
    {
    }

    debug_info >> fileName;
    // Read content
    while (debug_info.good())
    {
        fPwd = fileName;
        if (fPwd[0] == 'C')
        {
            // Skip 'CU:'
            debug_info >> fPwd;
            // Skip table header
            debug_info.getline(buffer, 256);
            debug_info.getline(buffer, 256);
        }

        if (fPwd[fPwd.length() - 1] == ':')
        {
            fPwd.pop_back();
        }
        else
        {
            fPwd.resize(fPwd.size() - 5);
        }

        debug_info >> fileName;
        while (debug_info.good() && fileName[fileName.length() - 1] != ':' && fileName[fileName.length() - 1] != ']')
        {
            debug_info >> dec >> lineNumber >> hex >> binaryAddress;
            auto ds = make_shared<DebugSymbol>(fPwd, fileName, binaryAddress, lineNumber);
            line_info.insert(
                    upper_bound(line_info.begin(), line_info.end(), ds, DebugSymbol::LessThan), ds);
            ds.reset();
            debug_info >> fileName;
        }
    }
}

shared_ptr<DebugSymbol> DebugInfo::locateSym(uint32_t addr)
{
    const auto target = upper_bound(line_info.begin(), line_info.end(), addr, DebugInfo::LessThan);
    if (target != line_info.begin())
    {
        return *(target - 1);
    }
    else
    {
        return nullptr;
    }
}
