

#pragma once

#include "ins_semantics.hpp"
#include "ins_types.hpp"

namespace tana {

enum class LibcID {
  X86_Calloc,
  X86_Malloc,
  X86_Memcpy,
  X86_Mempcpy,
  X86_Memset,
  Invalid
};

class LibC_Factory {
public:
  static std::unique_ptr<Inst_Base> parseLibc(const std::string &line);
};

class LIBC_X86_Calloc : public Inst_Base {
private:
  LibcID id = LibcID ::X86_Calloc;
  uint32_t m_num, m_size;
  uint32_t m_ret_value;

public:
  LIBC_X86_Calloc(uint32_t num, uint32_t size, uint32_t ret_value)
      : Inst_Base(false) {
    m_num = num;
    m_size = size;
    m_ret_value = ret_value;
    is_function = true;
  }
  bool symbolic_execution(SEEngine *se) final;
};

class LIBC_X86_Malloc : public Inst_Base {
private:
  LibcID id = LibcID ::X86_Malloc;
  uint32_t m_size;
  uint32_t m_ret_value;

public:
  LIBC_X86_Malloc(uint32_t size, uint32_t ret_value) : Inst_Base(false) {
    m_size = size;
    m_ret_value = ret_value;
    is_function = true;
  }
  bool symbolic_execution(SEEngine *se) final;
};

class LIBC_X86_Memcpy : public Inst_Base {
private:
  LibcID id = LibcID ::X86_Memcpy;
  uint32_t m_destination, m_source, m_num;
  uint32_t m_ret_value;

public:
  LIBC_X86_Memcpy(uint32_t destination, uint32_t source, uint32_t num,
                  uint32_t ret_value)
      : Inst_Base(false) {
    m_destination = destination;
    m_source = source;
    m_num = num;
    m_ret_value = ret_value;
    is_function = true;
  }
  bool symbolic_execution(SEEngine *se) final;
};

class LIBC_X86_Mempcpy : public Inst_Base {
private:
  LibcID id = LibcID ::X86_Mempcpy;
  uint32_t m_destination, m_source, m_num;
  uint32_t m_ret_value;

public:
  LIBC_X86_Mempcpy(uint32_t destination, uint32_t source, uint32_t num,
                   uint32_t ret_value)
      : Inst_Base(false) {
    m_destination = destination;
    m_source = source;
    m_num = num;
    m_ret_value = ret_value;
    is_function = true;
  }
  bool symbolic_execution(SEEngine *se) final;
};

class LIBC_X86_Memset : public Inst_Base {
private:
  LibcID id = LibcID ::X86_Memset;
  uint32_t m_ptr, m_value, m_num;
  uint32_t m_ret_value;

public:
  LIBC_X86_Memset(uint32_t ptr, uint32_t value, uint32_t num,
                  uint32_t ret_value)
      : Inst_Base(false) {
    m_ptr = ptr;
    m_value = value;
    m_num = num;
    m_ret_value = ret_value;
    is_function = true;
  }
  bool symbolic_execution(SEEngine *se) final;
};

} // namespace tana