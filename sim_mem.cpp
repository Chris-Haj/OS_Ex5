#include "sim_mem.h"


sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[], char swap_file_name[], int
text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int
                 page_size, int num_of_process) : text_size{text_size}, data_size{data_size}, bss_size{bss_size}, num_of_pages{num_of_pages}, num_of_proc{num_of_process}, page_size{page_size}, heap_stack_size{heap_stack_size}, TEXT_THRESH_HOLD{text_size / page_size}, DATA_THRESH_HOLD{(data_size / page_size) + TEXT_THRESH_HOLD},
                                                  BSS_THRESH_HOLD{(bss_size / page_size) + DATA_THRESH_HOLD}, HEAP_STACK_THRESH_HOLD{(heap_stack_size / page_size) + BSS_THRESH_HOLD} {
//    program_fd[0] = 0;
//    program_fd[1] = 0;
    EmptyPage = new char[page_size + 1];
    EmptyPage[page_size] = '\0';
    for (int i = 0; i < page_size; i++)
        EmptyPage[i] = '0';
    if (exe_file_name1 && strcmp(exe_file_name1, "") != 0) {
        program_fd[0] = open(exe_file_name1, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (program_fd[0] == -1) {
            perror("Error opening file1");
            sim_mem::~sim_mem();
            exit(1);
        }
    } else {
        fprintf(stderr, "file does not exist!");
        exit(1);
    }
    if (num_of_process == 2) {
        if (exe_file_name2 && strcmp(exe_file_name2, "") != 0) {
            program_fd[1] = open(exe_file_name2, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            if (program_fd[1] == -1) {
                perror("Error opening file2");
                sim_mem::~sim_mem();
                exit(1);
            }
        } else {
            fprintf(stderr, "file does not exist!");
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
            } else {
                page_table[i][j].P = 1;
            }
        }
    }
    for (char &i: main_memory)
        i = '0';

    swapSize = (num_of_pages - TEXT_THRESH_HOLD) * num_of_process;
    for (int i = 0; i < swapSize * page_size; i++) {
        if (write(swapfile_fd, "0", 1) == -1) {
            perror("Error writing to swap");
            exit(1);
        }
    }
    swapMemory = new char *[swapSize];
    for (int i = 0; i < swapSize; i++) {
        swapMemory[i] = new char[page_size + 1];
        swapMemory[i][page_size] = '\0';
        for (int j = 0; j < page_size; j++)
            swapMemory[i][j] = '0';
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
    process_id--;
    if (address < 0) {
        fprintf(stderr, "Address cannot be a negative number!\n");
        return '\0';
    }
    int page = address / page_size, offset = address % page_size;
    if (page >= num_of_pages) {
        fprintf(stderr, "Address does not have a page!\n");
        return '\0';
    }
    if (page_table[process_id][page].V == 1) {
        int frame = page_table[process_id][page].frame;
        int physicalAdd = (frame * page_size) + offset;
        return main_memory[physicalAdd];
    }
    int perm = page_table[process_id][page].P, dirty = page_table[process_id][page].D, bssHeapStack = address < BSS_THRESH_HOLD;
    char PageCopy[page_size + 1];
    strcpy(PageCopy, "");
    int frame = -1;
    if (perm == 0) { // FINISHED
        CopyPageFromExe(process_id, address / page_size, PageCopy);
        checkMemFull(page, PageCopy, process_id, &frame);
        int physicalAdd = (frame * page_size) + offset;
        return main_memory[physicalAdd];
    }
    if (dirty == 1) {
        BringPageFromSwap(process_id, page, PageCopy);
        checkMemFull(page, PageCopy, process_id, &frame);
        int physicalAdd = (frame * page_size) + offset;
        return main_memory[physicalAdd];
    }
    if (bssHeapStack == 0) {
        CopyPageFromExe(process_id, address / page_size, PageCopy);
        checkMemFull(page, PageCopy, process_id, &frame);
        int physicalAdd = (frame * page_size) + offset;
        return main_memory[physicalAdd];
    } else {
        fprintf(stderr, "Load error!\n");
        return '\0';
    }
}

void sim_mem::checkMemFull(int page, char PageCopy[], int id, int *f) {
    int frame = -1;
    for (int i = 0; i < MEMORY_SIZE; i += page_size)
        if (strncmp(&main_memory[i], EmptyPage, page_size) == 0) {
            frame = i / page_size;
            break;
        }
    if (frame == -1) { // if Memory is full
        int FrontFrame = FrameQ.front();
        FrameQ.pop();
        if (page_table[id][page].P == 1) {
            int EmptySwap = SearchEmptySwap();
            strncpy(swapMemory[EmptySwap], &main_memory[FrontFrame * page_size], page_size);
            page_table[id][page].frame = page_table[id][FrontFrame].frame;
            page_table[id][FrontFrame].swap_index = EmptySwap, page_table[id][FrontFrame].frame = -1;
            lseek(swapfile_fd, EmptySwap * page_size, SEEK_SET);
            write(swapfile_fd, PageCopy, page_size);
        }
        strncpy(&main_memory[FrontFrame * page_size], PageCopy, page_size);
        FrameQ.push(FrontFrame);
    } else { //Memory still has space
        page_table[id][page].frame = frame;
        strncpy(&main_memory[frame * page_size], PageCopy, page_size);
        FrameQ.push(frame);
        page_table[id][page].V = 1;
    }
    if (f != nullptr)
        *f = page_table[id][page].frame;
}

void sim_mem::CopyPageFromExe(int id, int offset, char PageCopy[]) {
    lseek(program_fd[id], offset * page_size, SEEK_SET);
    read(program_fd[id], PageCopy, page_size);
//    if (read(id, PageCopy, page_size) < page_size) {
//        perror("Read error");
//        exit(1);
//    }
}

void sim_mem::print_memory() {
    printf("\n Physical memory\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\t", main_memory[i]);
        if ((i + 1) % 10 == 0) {
            printf("\n");
        }
    }
}

void sim_mem::print_swap() {
    char *str = (char *) malloc(this->page_size * sizeof(char));
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (int i = 0; i < page_size; i++)
            printf("%d - [%c]\t", i, str[i]);
        printf("\n");
    }
    free(str);
}

void sim_mem::print_page_table() {
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j);
        printf("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
        for (int i = 0; i < num_of_pages; i++)
            printf("[%d]\t\t [%d]\t [%d]\t\t\t [%d]\t [%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame,
                   page_table[j][i].swap_index);
    }
}

void sim_mem::store(int process_id, int address, char value) {
    process_id--;
    if (address < 0)
        fprintf(stderr, "Address cannot be a negative number!\n");
    int page = address / page_size, offset = address % page_size;
    if (page >= num_of_pages) {
        fprintf(stderr, "Address does not have a page!\n");
        return;
    }
    if (page_table[process_id][page].V == 1) {
        int frame = page_table[process_id][page].frame;
        int physicalAdd = (frame * page_size) + offset;
        main_memory[physicalAdd] = value;
        return;
    }
    int perm = page_table[process_id][page].P, dirty = page_table[process_id][page].D, bssHeapStack = address < BSS_THRESH_HOLD;
    if (perm == 0) { // FINISHED
        fprintf(stderr, "No permission to write into text memory\n");
        return;
    }
    char PageCopy[page_size + 1];
    strcpy(PageCopy, "");
    if (dirty == 1) {
        BringPageFromSwap(process_id, page, PageCopy);
        checkMemFull(page, PageCopy, process_id, nullptr);
    } else if (bssHeapStack == 0) {
        CopyPageFromExe(process_id, address / page_size, PageCopy);
        checkMemFull(page, PageCopy, process_id, nullptr);
        page_table[process_id][page].D = 1;
    } else { // INIT NEW PAGE
        checkMemFull(page, EmptyPage, process_id, nullptr);
        page_table[process_id][page].D = 1;
    }
    int frame = page_table[process_id][page].frame;
    int physicalAdd = (frame * page_size) + offset;
    main_memory[physicalAdd] = value;
}

int sim_mem::SearchEmptySwap() {
    for (int i = 0; i < swapSize; i++)
        if (strncmp(swapMemory[i], EmptyPage, page_size) == 0)
            return i;
    return -1;
}

void sim_mem::BringPageFromSwap(int process, int page, char PageCopy[]) {
    int swapIndex = page_table[process][page].swap_index;
    page_table[process][page].swap_index = -1;
    strcpy(PageCopy, swapMemory[swapIndex]);
    strcpy(swapMemory[swapIndex], EmptyPage);
    lseek(swapfile_fd, swapIndex * page_size, SEEK_SET);
    if (write(swapfile_fd, EmptyPage, page_size) < page_size) {
        perror("Write error occurred!");
        exit(1);
    }
}
