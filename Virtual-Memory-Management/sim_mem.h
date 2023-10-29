#ifndef SIM_MEM_H
#define SIM_MEM_H




struct page_descriptor {
    bool valid;
    bool dirty;
    int frame;
    int swap_index;

};
class sim_mem {
private:
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    page_descriptor **page_table; //pointer to page table




public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int page_size);
    ~sim_mem();
    char load(int address);
    void store(int address, char value);
    void print_memory();
    void print_swap ();
    void print_page_table();

};

#endif // SIM_MEM_H
