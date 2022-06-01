#include <iostream>
#include "sim_mem.h"


int main() {
    char val;
    sim_mem mem_sm("exec_file", "", "swap_file", 25, 50, 25, 25, 25, 5, 1);
    mem_sm.store(1, 98, 'X');
    val = mem_sm.load(1, 98);
    mem_sm.print_memory();
    mem_sm.print_swap();
}