#ifndef OS_EX5_SIM_MEM_H
#define OS_EX5_SIM_MEM_H
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <queue>

using namespace std;
#define MEMORY_SIZE 20
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor {
    int V; // valid hit/miss
    int P; // permission access rights
    int D; // dirty page
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd[2]{}; //executable file fd
    int text_size; //memory that holds text (code)
    int data_size; //memory that holds initialized and static variables
    int bss_size; //memory that holds uninitialized and static variables
    int heap_stack_size; //memory of stack
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table; //pointer to page table
public:
    sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name[], int
    text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int
                     page_size, int num_of_process);

    ~sim_mem();

    char load(int process_id, int address);

    void store(int process_id, int address, char value);

    void print_memory();

    void print_swap();

    void print_page_table();
private:
    int swapSize;
    char *EmptyPage;
    char **swapMemory;
    queue <int> FrameQ;
    void checkMemFull(int page, char PageCopy[], int id, int *f);
    void CopyPageFromExe(int id, int offset, char PageCopy[]);
    void ReadReq(int address);
    void UpdatePageTable(page_descriptor table, int address, int frame);
    void BringPageFromSwap(int process, int page, char PageCopy[]);
    const int TEXT_THRESH_HOLD;
    const int DATA_THRESH_HOLD;
    const int BSS_THRESH_HOLD;
    const int HEAP_STACK_THRESH_HOLD;
    int SearchEmptySwap();
};


#endif //OS_EX5_SIM_MEM_H
