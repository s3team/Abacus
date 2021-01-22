/*************************************************************************
	> File Name: error.h
	> Author: Qinkun Bao
	> Mail: qinkunbao@gmail.com
	> Created Time: Mon Apr 22 21:56:51 2019
 ************************************************************************/

#pragma once
#include <map>

namespace tana{
    void default_error_handler(const char *file, int line, const char *message);
    void default_warn_handler(const char *file, int line, const char *message);
    void debug_map(const std::map<int , uint32_t > &value_map);
}


