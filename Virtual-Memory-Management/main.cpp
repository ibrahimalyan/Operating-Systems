#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sim_mem.h"

int main() {
    char val;
    sim_mem sim((char*)"exec_file.txt", (char*)"swap_file.txt", 16, 16, 32, 32, 16);
    sim.store(25, 'B');
    sim.store(50, 'C');


    sim.print_memory();
    sim.print_swap();
    sim.print_page_table();

    val = sim.load(25);
    std::cout << "Loaded value: " << val << std::endl;

    val = sim.load(50);
    std::cout << "Loaded value: " << val << std::endl;

    return 0;
}

