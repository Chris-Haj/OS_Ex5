#include <iostream>
#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

int main() {
    char exe1[] = "file1", exe2[] = "file2", swap[]="swap";
/*    int fd = open(exe1,O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    int fd2= open(exe2,O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    char a = 'a';
    char e= '1';
    for(int i=0;i<200;i++){
        if((i+1)%5!=0){
            write(fd,&a,1);
            write(fd2,&e,1);
            a++;
            e++;
//            if(a=='E') {
//                a='a';
//                e='e';
//            }
//            else if(a=='e'){
//                a='A';
//                e='E';
//            }
        }
        else{
            write(fd,"-",1);
            write(fd2,"-",1);
        }
    }*/
    sim_mem mem_sm(exe1,exe2,
                   swap, 25,
                   50, 25, 25,
                   25, 5, 2);



    cout<<mem_sm.load(1,0)<<endl;
    cout<<mem_sm.load(1,5)<<endl;
    cout<<mem_sm.load(1,10)<<endl;
    cout<<mem_sm.load(1,15)<<endl;
    cout<<mem_sm.load(1,20)<<endl;
    cout<<mem_sm.load(1,25)<<endl;
    cout<<mem_sm.load(1,30)<<endl;
    cout<<mem_sm.load(2,0)<<endl;
    cout<<mem_sm.load(2,5)<<endl;
    cout<<mem_sm.load(2,10)<<endl;
    cout<<mem_sm.load(2,15)<<endl;
    cout<<mem_sm.load(2,20)<<endl;
    cout<<mem_sm.load(2,25)<<endl;
    cout<<mem_sm.load(2,30)<<endl;
//    mem_sm.store(1,22,'x');
//    mem_sm.store(1,50,'y');
//    mem_sm.store(1,53,'y');
//    mem_sm.store(1,80,'w');
//    mem_sm.store(1,90,'z');
//    mem_sm.store(2,22,'x');
//    mem_sm.store(2,50,'y');
//    mem_sm.store(2,53,'y');
//    mem_sm.store(2,80,'w');
//    mem_sm.store(2,90,'z');
    mem_sm.print_page_table();
    mem_sm.print_memory();
    mem_sm.print_swap();
}

