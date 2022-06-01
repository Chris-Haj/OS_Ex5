#include "sim_mem.h"


sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name[], int
text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int
                 page_size, int num_of_process) {
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    this->num_of_proc = num_of_process;
    this->heap_stack_size = heap_stack_size;
    if ((this->swapfile_fd = open(swap_file_name, O_CREAT | O_RDWR)) == -1) {
        perror("Error creating swap memory");
        exit(1);
    }
    if ((exe_file_name1 && strcmp(exe_file_name1,"") !=0 )&&(program_fd[0] = open(exe_file_name1, O_CREAT | O_RDWR)) == -1) {
        perror("Error opening file1");
        exit(1);
    }
    if (num_of_process == 2) {
        if ((exe_file_name2  && strcmp(exe_file_name2,"") !=0 )&&(program_fd[1] = open(exe_file_name2, O_CREAT | O_RDWR)) == -1) {
            perror("Error opening file2");
            exit(1);
        }
    }
    page_table = new page_descriptor *[num_of_process];
    for (int i = 0; i < num_of_process; i++) {
        page_table[i] = new page_descriptor[num_of_pages];
        for (int j = 0; j < num_of_pages; j++) {(exe_file_name1 && strcmp(exe_file_name1,"") !=0 )&&
            page_table[i][j].V = 0;
            page_table[i][j].P = 1;
            page_table[i][j].D = 0;
            page_table[i][j].frame = -1;
            page_table[i][j].swap_index = -1;
        }
    }
    for(int i=0;i<page_size*(num_of_pages-text_size);i++)
        if(write(swapfile_fd,"0",sizeof(char))==-1){
            perror("Error writing to swap");
            exit(1);
        }

}

sim_mem::~sim_mem() {
    close(program_fd[0]);
    close(program_fd[1]);
    close(swapfile_fd);
    for (int i = 0; i < num_of_proc; i++) {
        delete[] page_table[i];
    }
    delete[] page_table;

}

char sim_mem::load(int process_id, int address) {

}

void sim_mem::store(int process_id, int address, char value) {

}

void sim_mem::print_memory() {
    int i;
    cout << "\n Physical memory\n";
    for (i = 0; i < MEMORY_SIZE; i++) {
        cout << main_memory[i] << endl;
    }
}

void sim_mem::print_swap() {
    char *str = new char[this->page_size];
    int i;
    cout << "\n Swap memory\n";
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++) {
            cout << i << " - " << str[i] << "\t";
        }
        cout << endl;
    }
    delete[] str;
}

void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        cout << "\n page table of process:" << j << endl;
        cout << "Valid\t Dirty\t Permission \t Frame\t Swap index\n";
        for (i = 0; i < num_of_pages; i++) {
            cout <<
                 page_table[j][i].V <<
                 page_table[j][i].D <<
                 page_table[j][i].P <<
                 page_table[j][i].frame <<
                 page_table[j][i].swap_index;
        }
    }
}