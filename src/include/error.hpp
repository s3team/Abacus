

#pragma once
#include <map>
#include <cstdint>

namespace tana {
enum class LOG_TYPE { DEBUG, INFO, MUTE };


#ifdef NDEBUG
static LOG_TYPE log_class = LOG_TYPE::MUTE;
#else
static LOG_TYPE log_class = LOG_TYPE::DEBUG;
#endif


void default_error_handler(const char *file, int line, const char *message);
void default_warn_handler(const char *file, int line, const char *message);
void debug_map(const std::map<int, uint32_t> &value_map);
} // namespace tana
