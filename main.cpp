#include <iostream>
#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

int main() {
    char exe1[] = "file1", exe2[] = "file2", swap[]="swap";
    sim_mem mem_sm(exe1,exe2,
                   swap, 25,
                   50, 25, 25,
                   25, 5, 1);



//    mem_sm.store(1, 98, 'X');
//   char val = mem_sm.load(1, 98);
//    mem_sm.print_page_table();
//    mem_sm.print_memory();
//    mem_sm.print_swap();
}

