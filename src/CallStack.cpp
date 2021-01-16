
#include "CallStack.hpp"
#include <iostream>
#include <memory>
#include <tuple>

using namespace std;

namespace tana {

CallLeakageSites::CallLeakageSites(const std::string &key_name, int key_id)
    : key_name(key_name), key_id(key_id) {}

std::ostream &operator<<(std::ostream &os, const CallLeakageSites &c) {
  os << c.key_name;
  for (const auto &call : c.leakage_stack_sites) {
    os << " -> ";
    const string &fun_name = get<0>(call);
    os << "(" << fun_name << ")";
  }

  return os;
}

std::shared_ptr<CallLeakageSites> CallLeakageSites::clone() {

  std::shared_ptr<CallLeakageSites> obj(new CallLeakageSites(*this));
  return obj;
}

CallStackKey::CallStackKey(const string &name, int key_id,
                           const std::string &start_fun)
    : key_id(key_id), key_name(name), stack_depth(0),
      current_fun_name(start_fun) {
  auto first_call = std::make_tuple(start_fun, stack_depth);

  stack_sites = std::make_unique<CallLeakageSites>(key_name, key_id);
  stack_sites->leakage_stack_sites.push_back(first_call);
  stack_funs.push(start_fun);
}

int CallStackKey::updateStack(int input_key_id, const std::string &fun_name) {
  if (input_key_id != key_id)
    return 0;

  tuple<string, int> current_stack = stack_sites->leakage_stack_sites.back();
  if (!stack_funs.empty()) {
    string current_fun = stack_funs.top();
    int current_stack_max = get<1>(current_stack);

    if (current_fun == fun_name) {
      return current_stack_max;
    }

    ++stack_depth;
    stack_funs.push(fun_name);

    auto call = std::make_tuple(fun_name, stack_depth);
    stack_sites->leakage_stack_sites.push_back(call);
  }
  return stack_depth;
}

int CallStackKey::retStack(const std::string &fun_name) {
  if (!stack_funs.empty()) {
    auto current_fun = stack_funs.top();
    if (current_fun != fun_name) {
      return 0;
    }
    stack_funs.pop();
  }
  --stack_depth;
  return stack_depth;
}

void CallStackKey::fastUpdate(const std::string &fun_name) {
  if (fun_name == current_fun_name)
    return;

  auto info = std::make_tuple(fun_name, 0);
  stack_sites->leakage_stack_sites.push_back(info);
  current_fun_name = fun_name;
}

std::shared_ptr<CallLeakageSites> CallStackKey::clone() {
  return stack_sites->clone();
}

CallStack::CallStack(
    const std::string &start_fun_name,
    const std::vector<std::tuple<int, std::string>> &key_value_set) {

  for (const auto &key_tuple : key_value_set) {
    int key_id = get<0>(key_tuple);
    string key_str;
    key_str = get<1>(key_tuple);
    auto element =
        std::make_unique<CallStackKey>(key_str, key_id, start_fun_name);
    stacks[key_id] = std::move(element);
  }
}

int CallStack::proceed_inst(const std::vector<int> &vector_key_id,
                            const std::string &function_name) {
  int count = 0;
  // for(const int &key_id : vector_key_id)
  //{
  //   (stacks.at(key_id))->updateStack(key_id, function_name);
  //    ++count;
  //}
  std::vector<int> key_vector;
  for (auto &it : stacks) {
    key_vector.push_back(it.first);
  }
  for (const int &key_id : key_vector) {
    (stacks.at(key_id))->updateStack(key_id, function_name);
    ++count;
  }
  return count;
}

void CallStack::fast_call_stack(const std::string &function_name) {
  std::vector<int> key_vector;
  for (auto &it : stacks) {
    key_vector.push_back(it.first);
  }
  for (const int &key_id : key_vector) {
    (stacks.at(key_id))->fastUpdate(function_name);
  }
}

int CallStack::ret(const std::string &function_name) {
  std::vector<int> key_vector;
  for (auto &it : stacks) {
    key_vector.push_back(it.first);
  }
  for (const int &key_id : key_vector) {
    (stacks.at(key_id))->retStack(function_name);
  }
  return key_vector.size();
}

std::shared_ptr<CallLeakageSites> CallStack::cloneCallLeakageSites(int key_id) {
  return stacks.at(key_id)->clone();
}
} // namespace tana