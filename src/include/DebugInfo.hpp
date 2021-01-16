

#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace tana {

class DebugSymbol {
public:
  std::string pwd;
  std::string file_name;
  uint64_t address;
  int line_number;
  friend std::ostream &operator<<(std::ostream &os, const DebugSymbol &dt);

  DebugSymbol(const std::string &fPwd, const std::string &fileName,
              uint64_t binaryAddress, int lineNumber)
      : pwd(fPwd), file_name(fileName), address(binaryAddress),
        line_number(lineNumber){};
};

class DebugInfo {
private:
  std::vector<std::shared_ptr<DebugSymbol>> line_info;

public:
  explicit DebugInfo(const std::string &filePWD);
  std::shared_ptr<DebugSymbol> locateSym(uint64_t addr);
  ~DebugInfo() = default;
};

} // namespace tana
