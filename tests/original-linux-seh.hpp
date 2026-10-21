#pragma once
// A private 32-bit exception-chain cell for unchanged MSVC SEH prologues.
// Linux i386 TLS uses GS. Save/restore FS; never replace the host GS selector.
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
struct OriginalLinuxSeh {
  unsigned chain[16]{};
  unsigned short previous{};
  unsigned short selector{};
  OriginalLinuxSeh() {
    __asm__ volatile("mov %%fs,%0" : "=rm"(previous));
    chain[0] = 0xffffffffu;
    user_desc descriptor{};
    descriptor.entry_number = unsigned(-1);
    descriptor.base_addr = reinterpret_cast<unsigned>(chain);
    descriptor.limit = 0xfffff;
    descriptor.seg_32bit = 1;
    descriptor.limit_in_pages = 1;
    descriptor.useable = 1;
    if (syscall(SYS_set_thread_area, &descriptor))
      throw std::runtime_error("Cannot allocate private original SEH segment");
    selector = static_cast<unsigned short>((descriptor.entry_number << 3) | 3);
    __asm__ volatile("mov %0,%%fs" ::"rm"(selector) : "memory");
  }
  void check() const {
    if (chain[0] != 0xffffffffu)
      throw std::runtime_error("Original SEH chain did not unwind");
  }
  ~OriginalLinuxSeh() {
    __asm__ volatile("mov %0,%%fs" ::"rm"(previous) : "memory");
  }
  OriginalLinuxSeh(const OriginalLinuxSeh &) = delete;
  OriginalLinuxSeh &operator=(const OriginalLinuxSeh &) = delete;
};
