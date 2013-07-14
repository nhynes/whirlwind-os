whirlwind-os
============

A simple, proto-os written in Intel assembly. Made for x86 processors (no _64)

Features:
  bootloader (gale),
  kernel (storm) -
    protected mode (32/64 bit),
    paging+virtual memory,
    interrupts,
    memory manager -
      (s)brk(),
      malloc() for allocations of < 1 page-memory manager headers,
      the beginnings of free()
