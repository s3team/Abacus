
#include "error.hpp"
#include <iostream>
#include <map>
#include <string>

namespace tana {

void default_error_handler(const char *file, int line, const char *message) {
  if (log_class == LOG_TYPE::MUTE) {
    return;
  }

  std::string file_name(file);
  std::string error_message(message);
  std::cerr << "\n********************************" << std::endl;
  std::cerr << "Error at :" << line << "\n";
  std::cerr << "File name: " << file_name << "\n";
  std::cerr << "Message: " << error_message << "\n";
  std::cerr << "********************************\n" << std::endl;
  // exit(0);
}

void default_warn_handler(const char *file, int line, const char *message) {
  if (log_class == LOG_TYPE::INFO || log_class == LOG_TYPE::MUTE) {
    return;
  }

  std::string file_name(file);
  std::string error_message(message);
  std::cerr << "\n********************************" << std::endl;
  std::cerr << "Error at :" << line << "\n";
  std::cerr << "File name: " << file_name << "\n";
  std::cerr << "Message: " << error_message << "\n";
  std::cerr << "********************************\n" << std::endl;
}

void debug_map(const std::map<int, uint32_t> &value_map) {
  for (auto const &x : value_map) {
    std::cout << x.first << ':' << std::hex << x.second << std::dec << " ";
  }

  std::cout << std::endl;
}

} // namespace tana