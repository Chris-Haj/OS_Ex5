#include "sim_mem.h"


sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name[], int
text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int
                 page_size, int num_of_process) :text_size{text_size},data_size{data_size},bss_size{bss_size},num_of_pages{num_of_pages},num_of_proc{num_of_process}
                 ,page_size{page_size},heap_stack_size{heap_stack_size} , TEXT_THRESH_HOLD{text_size/page_size} , DATA_THRESH_HOLD{(data_size/page_size)+TEXT_THRESH_HOLD},
                 BSS_THRESH_HOLD{(bss_size/page_size)+DATA_THRESH_HOLD}, HEAP_THRESH_HOLD{(heap_stack_size/page_size)+BSS_THRESH_HOLD},
                 STACK_THRESH_HOLD{(heap_stack_size/page_size)+HEAP_THRESH_HOLD}{
    program_fd[0] = 0;
    program_fd[1] = 0;
    if ((exe_file_name1 && strcmp(exe_file_name1, "") != 0) && (program_fd[0] = open(exe_file_name1, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
        perror("Error opening file1");
        sim_mem::~sim_mem();
        exit(1);
    }
    if (num_of_process == 2) {
        if ((exe_file_name2 && strcmp(exe_file_name2, "") != 0) && (program_fd[1] = open(exe_file_name2, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
            perror("Error opening file2");
            sim_mem::~sim_mem();
            exit(1);
        }
    }
    if ((this->swapfile_fd = open(swap_file_name, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1) {
        perror("Error creating swap memory");
        sim_mem::~sim_mem();

        exit(1);
    }

    page_table = new page_descriptor *[num_of_process];
    for (int i = 0; i < num_of_process; i++) {
        page_table[i] = new page_descriptor[num_of_pages];
        for (int j = 0; j < num_of_pages; j++) {
            page_table[i][j].V = 0;
            page_table[i][j].D = 0;
            page_table[i][j].frame = -1;
            page_table[i][j].swap_index = -1;
            if (j < TEXT_THRESH_HOLD) {
                page_table[i][j].P = 0;
            }
            else {
                page_table[i][j].P = 1;
            }
        }
    }
    for (char &i: main_memory)
        i = '0';

    swapSize=(num_of_pages-TEXT_THRESH_HOLD)*num_of_process;
    for (int i = 0; i < swapSize*num_of_pages; i++) {
        if (write(swapfile_fd, "0", sizeof(char)) == -1) {
            perror("Error writing to swap");
            exit(1);
        }
    }
    swapMemory = new char*[swapSize];
    for(int i=0 ;i< swapSize;i++){
        swapMemory[i]= new char[page_size];
        swapMemory[i][page_size]='\0';
        for(int j=0;j<page_size;j++)
            swapMemory[i][j]='0';
    }
}

sim_mem::~sim_mem() {
    if (program_fd[0] != 0)
        close(program_fd[0]);
    if (program_fd[1] != 0)
        close(program_fd[1]);
    close(swapfile_fd);
    for (int i = 0; i < num_of_proc; i++) {
        delete[] page_table[i];
    }
    delete[] page_table;
    delete[] swapMemory;
}

char sim_mem::load(int process_id, int address) {
    int id = process_id - 1;
    char PageCopy[page_size+1];
    if (address < 0) {
        fprintf(stderr, "Address cannot be a negative number!\n");
        return '\0';
    }
    int page = address / page_size, offset = address % page_size;
    if (page_table[id][page].V == 1) {
        int frame = page_table[id][page].frame;
        int physicalAdd = (frame * page_size) + offset;
        return main_memory[physicalAdd];
    }
    int lseekOffSet = page*page_size;
    if (page_table[id][page].P == 1) { // checking if access rights to page are available 1=true | 0=false
        if(page_table[id][page].D==1){ //dirty page

        }
        else{ // clean page
            if(BSS_THRESH_HOLD<=address&&address<STACK_THRESH_HOLD){ // Address is in BSS/Heap/Stack

            }
            else{ // Address in Text/Data
                CopyPageFromExe(id, lseekOffSet, PageCopy, page, id);

            }
        }
    }
    else {  //NO ACCESS RIGHTS
        //TODO {text - data - bss -heap - stack}
        CopyPageFromExe(id, lseekOffSet, PageCopy, page, id);
    }

    return '\0';
}

void sim_mem::store(int process_id, int address, char value) {



}

void sim_mem::print_memory() {
    int i;
    cout << "\n Physical memory\n";
    for (i = 0; i < MEMORY_SIZE; i++) {
        cout << "[" << main_memory[i] << "]" << endl;
    }
}

void sim_mem::print_swap() {
    char *str = new char[this->page_size];
    int i;
    cout << "\n Swap memory\n";
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++) {
            cout << i << " - " << "[" << str[i] << "]" << "\t";
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
                 page_table[j][i].V << "\t     " <<
                 page_table[j][i].D << "\t     " <<
                 page_table[j][i].P << "\t             " <<
                 page_table[j][i].frame << "\t     " <<
                 page_table[j][i].swap_index << endl;
        }
    }
}

int sim_mem::checkMemFull(int address, int process) {
    int MemoryFull = -1;
    char EmptyPage[page_size + 1];
    for(int i=0;i<page_size;i++)
        EmptyPage[i]='0';
    for(int i=0;i<MEMORY_SIZE;i+=page_size){
        if(strncmp(&main_memory[i], EmptyPage, page_size) == 0){
            MemoryFull = i/page_size;
            break;
        }
    }
    if(MemoryFull==-1){ // Memory is full
        if()
        SwapOutPage(address);
    }
    else
        ReadReq(MemoryFull);
    return -1;
}

void sim_mem::CopyPageFromExe(int id, int offset, char PageCopy[], int address, int process) {
    lseek(id,offset,SEEK_SET);
    if(read(id, PageCopy, page_size)<page_size){
        perror("Read error");
        exit(1);
    }
    checkMemFull(address, process);
}

void sim_mem::ReadReq(int address) {

}

void sim_mem::SwapOutPage(int address) {

}

void sim_mem::UpdatePageTable(page_descriptor page_table, int address, int frame) {

}
