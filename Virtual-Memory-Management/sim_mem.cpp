#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bitset>
#include "sim_mem.h"


#define MEMORY_SIZE 200

char main_memory[MEMORY_SIZE];

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int page_size) {
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->page_size = page_size;
    this->num_of_pages = (text_size + data_size + bss_size + heap_stack_size + page_size) / page_size;
// Allocate memory for page_table
    page_table = new page_descriptor *[4];
    int frame_counter = 0;
    for (int i = 0; i < 4; i++) {
        page_table[i] = new page_descriptor[num_of_pages / 4];
        for (int j = 0; j < num_of_pages / 4; j++) {
            page_table[i][j].valid = true; // mark as valid
            page_table[i][j].dirty = true; // mark as dirty if needed
            page_table[i][j].frame = frame_counter++; // assign incrementing frame number
            page_table[i][j].swap_index = -1;
        }
    }
    // Initialize main_memory with zeros
    for (int i = 0; i < MEMORY_SIZE; i++) {
        main_memory[i] = 0;
    }

    // Open swap
    swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, S_IRWXU);
    if (swapfile_fd == -1) {
        std::cerr << "Failed to open swap file." << std::endl;
        exit(1);
    }

    // Open executable file
    program_fd = open(exe_file_name, O_RDONLY);
    if (program_fd == -1) {
        std::cerr << "Failed to open executable file at path: " << exe_file_name << ". Please make sure the file exists and the path is correct." << std::endl;
        return;
    }

    // Initialize the SWAP file with zeroes
    lseek(this->swapfile_fd, num_of_pages * page_size - 1, SEEK_SET);
    write(this->swapfile_fd, "", 1);
}

sim_mem::~sim_mem() {
    // Close files
    close(swapfile_fd);
    close(program_fd);

    // Deallocate memory for page_table
    for (int i = 0; i < 4; i++) {
        delete[] page_table[i];
    }
    delete[] page_table;
}

char sim_mem::load(int address) {
    if (address < 0 || address >= num_of_pages * page_size) {
        std::cerr << "Invalid address: " << address << ", Total Memory Size: " << num_of_pages * page_size << "\n";
        return -1;
    }

    // Extract the page index by shifting the address 12 bits to the right and masking the last two bits.
    int pageIndex = (address >> 12) & 0x3;

    // Convert the page index to binary
    std::bitset<2> binaryPageIndex(pageIndex);

    // Extract the sub-table index by shifting the address 2 bits to the right and masking the bits 2 to 11.
    int subTableIndex = (address >> 2) & ((1 << 10) - 1);

    // Extract the offset by masking the last two bits of the address.
    int offset = address & ((1 << 2) - 1);


    if (!page_table[pageIndex][subTableIndex].valid) {
        // Page fault, load the page
        if (pageIndex == 0) {
            // Text page
            page_table[pageIndex][subTableIndex].valid = true;
            page_table[pageIndex][subTableIndex].frame = subTableIndex;
        } else {
            // Data, BSS, or Heap/Stack page
            if (page_table[pageIndex][subTableIndex].dirty) {
                // Page is in swap, load from swap file
                off_t offset = static_cast<off_t>(page_table[pageIndex][subTableIndex].swap_index * page_size);
                lseek(swapfile_fd, offset, SEEK_SET);
                char *buffer = new char[page_size];
                read(swapfile_fd, buffer, page_size);
                for (int i = 0; i < page_size; i++) {
                    main_memory[subTableIndex * page_size + i] = buffer[i];
                }
                delete[] buffer;
            } else {
                // Page is not in swap, initialize it
                if (pageIndex == 1 || pageIndex == 2) {
                    // Data or BSS page
                    for (int i = 0; i < page_size; i++) {
                        main_memory[subTableIndex * page_size + i] = 0;
                    }
                } else if (pageIndex == 3) {
                    // Heap/Stack page
                    if (offset == 0) {
                        // Page is first accessed, initialize with zeros
                        for (int i = 0; i < page_size; i++) {
                            main_memory[subTableIndex * page_size + i] = 0;
                        }
                    } else {
                        std::cerr << "Error: Attempted to read from uninitialized Heap/Stack page." << std::endl;
                        return -1;
                    }
                }
                page_table[pageIndex][subTableIndex].valid = true;
                page_table[pageIndex][subTableIndex].frame = subTableIndex;
            }
        }
    }
    return main_memory[page_table[pageIndex][subTableIndex].frame * page_size + offset];
}

void sim_mem::store(int address, char value) {
    if (address < 0 || address >= num_of_pages * page_size) {
        std::cerr << "Invalid address: " << address << ", Total Memory Size: " << num_of_pages * page_size << "\n";
        return;
    }

    // Extract the page index by shifting the address 12 bits to the right and masking the last two bits.
    int pageIndex = (address >> 12) & 0x3;

    // Convert the page index to binary
    std::bitset<2> binaryPageIndex(pageIndex);

    // Extract the sub-table index by shifting the address 2 bits to the right and masking the bits 2 to 11.
    int subTableIndex = (address >> 2) & ((1 << 10) - 1);

    // Extract the offset by masking the last two bits of the address.
    int offset = address & ((1 << 2) - 1);

    if (!page_table[pageIndex][subTableIndex].valid) {
        // Page fault, mark the page as valid and initialize if necessary
        page_table[pageIndex][subTableIndex].valid = true;
        page_table[pageIndex][subTableIndex].frame = subTableIndex;

        if (pageIndex == 3 && offset == 0) {
            // Heap/Stack page, initialize with zeros
            for (int i = 0; i < page_size; i++) {
                main_memory[subTableIndex * page_size + i] = 0;
            }
        }
    }

    // Write the value to the main_memory at the correct offset of the page's frame.
    main_memory[page_table[pageIndex][subTableIndex].frame * page_size + offset] = value;

    // Mark the page as dirty as we have made changes to it.
    page_table[pageIndex][subTableIndex].dirty = true;

    // If the page is not in swap file yet, create a new entry.
    if (page_table[pageIndex][subTableIndex].swap_index == -1) {
        page_table[pageIndex][subTableIndex].swap_index = (pageIndex * (num_of_pages / 4) + subTableIndex);
    }

    // Write the updated page to the swap file.
    off_t swapOffset = static_cast<off_t>(page_table[pageIndex][subTableIndex].swap_index * page_size);
    lseek(swapfile_fd, swapOffset, SEEK_SET);
    write(swapfile_fd, &main_memory[page_table[pageIndex][subTableIndex].frame * page_size], page_size);
}

void sim_mem::print_memory() {
    for (int i = 0; i < MEMORY_SIZE; i++) {
        std::cout << main_memory[i];
    }
    std::cout << std::endl;
}

void sim_mem::print_swap() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < num_of_pages / 4; j++) {
            if (page_table[i][j].swap_index >= 0) {
                off_t offset = static_cast<off_t>(page_table[i][j].swap_index * page_size);
                lseek(swapfile_fd, offset, SEEK_SET);
                char *buffer = new char[page_size];
                read(swapfile_fd, buffer, page_size);
                for (int k = 0; k < page_size; k++) {
                    std::cout << buffer[k];
                }
                std::cout << std::endl;
                delete[] buffer;
            }
        }
    }
}
void sim_mem::print_page_table() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < num_of_pages / 4; j++) {
            std::cout << "Page: " << (i * (num_of_pages / 4) + j);
            if (page_table[i][j].valid) {
                std::cout << " Frame: " << page_table[i][j].frame;
                if (page_table[i][j].dirty) {
                    std::cout << " (dirty)";
                }
            } else {
                std::cout << " Not in memory";
                if (page_table[i][j].swap_index >= 0) {
                    std::cout << " (in swap)";
                }
            }
            std::cout << std::endl;
        }
    }
}