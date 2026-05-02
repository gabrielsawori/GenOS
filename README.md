# GenOS
An open source operating system that is built independently and can be used, developed and modified by anyone as long as they include credit.

What's new in v2.1?

1. Memory Management
   Bitmap-based PMM (4KB per block) and Heap Allocator with coalesce (anti-leak) feature.
2. Input/Output
   Smart driver keyboard with mode switch (Shell/App) and anti-ghost enter filter.
3. File System
   Able to read/write files on virtual HDD and execute ELF binaries from User Space.
4. Network (L1 & L2)
   The Intel E1000 driver is capable of reading MAC, capturing packets (RX), and firing packets (TX).
5. Interrupt (IDT)
   The asynchronous IRQ system successfully connects the Hardware to the Kernel without crashing.

This kernel version is still in pre-beta status and under development. Please do not run this kernel on your computer until a stable version is available.

In version 3 there will be a pre-test version for booting on UEFI and x86_64 (64 bit) architecture.
