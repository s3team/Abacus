/************************ >->addressconst uint32_tconst std::shared_ptr<DebugSymbol> < LessThanLessThan
	> File Name: DynSEEngine.cpp
	> Author: Qinkun Bao & Zihao Wang
	> Mail: qinkunbao@gmail.com
	> Created Time: Tue Sep  3 17:19:02 EDT 2019
 ************************************************************************/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>

namespace tana
{

class DebugSymbol
{
public:
    std::string pwd;
    std::string file_name;
    uint32_t address;
    int line_number;
    DebugSymbol(const std::string &fPwd,
                const std::string &fileName,
                uint32_t binaryAddress,
                int lineNumber) : pwd(fPwd),
                                  file_name(fileName),
                                  address(binaryAddress),
                                  line_number(lineNumber) {}
    static struct _LessThan
    {
        bool operator()(const std::shared_ptr<DebugSymbol> &left, const std::shared_ptr<DebugSymbol> &right)
        {
            return left->address < right->address;
        }
    } LessThan;
};

class DebugInfo
{
private:
    std::vector<std::shared_ptr<DebugSymbol>> line_info;

public:
    explicit DebugInfo(std::ifstream &debug_file);
    explicit DebugInfo(std::string &debug_string);
    std::shared_ptr<DebugSymbol> locateSym(uint32_t addr);
    static struct _LessThan
    {
        bool operator()(const uint32_t &left, const std::shared_ptr<DebugSymbol> &right)
        {
            return left < right->address;
        }
    } LessThan;
    ~DebugInfo() = default;
};

} // namespace tana

