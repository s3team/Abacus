
#pragma once

#include "ins_types.hpp"
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

namespace tana {

class CallLeakageSites {
public:
  std::vector<std::tuple<std::string, int>> leakage_stack_sites;
  const int key_id;
  const std::string key_name;

  friend std::ostream &operator<<(std::ostream &os, const CallLeakageSites &c);

  CallLeakageSites(const std::string &key_name, int key_id);

  std::shared_ptr<CallLeakageSites> clone();
};

class CallStackKey {

private:
  std::unique_ptr<CallLeakageSites> stack_sites;
  std::stack<std::string> stack_funs;
  const int key_id;
  const std::string key_name;

  int stack_depth;

  std::string current_fun_name;

public:
  CallStackKey(const std::string &key_name, int key_id,
               const std::string &start_fun);

  int updateStack(int key_id, const std::string &fun_name);

  int retStack(const std::string &fun_name);

  void fastUpdate(const std::string &fun_name);

  std::shared_ptr<CallLeakageSites> clone();
};

class CallStack {
private:
  std::map<int, std::unique_ptr<CallStackKey>> stacks;

public:
  CallStack(const std::string &start_fun_name,
            const std::vector<std::tuple<int, std::string>> &key_value_set);

  int proceed_inst(const std::vector<int> &vector_key_id,
                   const std::string &function_name);

  int ret(const std::string &function_name);

  std::shared_ptr<CallLeakageSites> cloneCallLeakageSites(int key_id);

  void fast_call_stack(const std::string &function_name);
};
} // namespace tana
