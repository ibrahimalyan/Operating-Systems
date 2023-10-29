Memory management
Authored by Ibrahim Alyan
322675356
---Description---
This program we use Simulation of processor accesses to memory. We use the segment page table mechanism that allows programs to be run when
Only part of it is in memory. The program memory (also called virtual memory) is divided into pages that are loaded into the main memory as needed.

---functions---
1-load: This function loads a character from a specific memory address. It checks if the address is valid and extracts the page index, sub-table index, and offset. It handles different types of pages (text, data, BSS, heap/stack) by initializing or loading them. Finally, it returns the character value from the corresponding memory location.

2-store: This function stores a character value at a specific memory address. It first checks if the address is valid and extracts the page index, sub-table index, and offset. If the corresponding page is not valid, it marks it as valid and initializes it if necessary. Then, it writes the value to the main memory at the correct offset of the page's frame. The page is marked as dirty to indicate changes. If the page is not in the swap file, a new entry is created. Finally, the updated page is written to the swap file.

3-print_memory: This function prints the contents of the main memory in the simulated memory system. It iterates over each memory location and outputs the character stored at that location. After printing all the characters, it inserts a line break to ensure the output is formatted properly.

4-print_swap:his function prints the contents of the swap file in the simulated memory system. It iterates over each entry in the page table and checks if a swap index is assigned. If a swap index exists, it seeks to the corresponding offset in the swap file and reads a page-sized buffer of characters. It then prints each character in the buffer and inserts a line break. Finally, it frees the memory allocated for the buffer.

5-print_page_table:

---Program fiels---
sim_mem.h       this file contain the header file
sim_mem.cpp    this file contain the realization of the functions
main.cpp   	this file contai the main

---How to compile---
compile:  g++ -o sim_mem sim_mem.cpp main.cpp

run ; ./sim_mem

---input---
  sim.store(25, 'B');
  sim.store(50, 'C');

---output---
BC


---input---
 val = sim.load(25);
    std::cout << "Loaded value: " << val << std::endl;

    val = sim.load(50);
    std::cout << "Loaded value: " << val << std::endl;

---output---
Loaded value: B
Loaded value: C

