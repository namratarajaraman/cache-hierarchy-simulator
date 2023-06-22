#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sim.h"
#include<string.h>
#include<iostream>
#include<vector>
#include <algorithm>
#include<math.h>
#include <cstdlib>
using namespace std;
/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/

//-------------------CACHE ALLOCATION------------------
//-----------------globals (sorry)----------------
vector<instruction_line> inst_list;
unsigned mask;
cache_params_t params;
CACHE c1;
CACHE c2;
bool l2_dne=0;
bool hit_L1=0;
PREFETCH_UNIT prefetch;
bool prefetch_exists=false;
//------------------------------------------------------

//------------------READ FILE, GET CACHE SPECS, PARSE INSTRUCTIONS------------------
//------------------THINK BIG GET CACHE MAKE EM BLINK FAST--------------------------
void parse_file(int argc, char *argv[]) { //read in commands into an array
    FILE *fp;            // File pointer.
    char *trace_file;        // This variable holds the trace file name.
       // Look at the sim.h header file for the definition of struct cache_params_t.
    char rw;            // This variable holds the request's type (read or write) obtained from the trace.
    uint32_t addr;        // This variable holds the request's address obtained from the trace.
    // The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.
    //----address parse variables----
    int no_sets_c1=0;
    int no_sets_L2=0;
    int tag_size_c1=0;
    int index_size_c1=0;
    int boffset_size_c1=0;
    int tag_size_L2=0;
    int index_size_L2=0;
    int boffset_size_L2=0;
    int indexblock=0;
    instruction_line inst;

    // Exit with an error if the number of command-line arguments is incorrect.
    if (argc != 9) {
        printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
        exit(EXIT_FAILURE);
    }

    // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
    params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
    params.L1_SIZE = (uint32_t) atoi(argv[2]);
    params.L1_ASSOC = (uint32_t) atoi(argv[3]);
    params.L2_SIZE = (uint32_t) atoi(argv[4]);
    params.L2_ASSOC = (uint32_t) atoi(argv[5]);
    params.PREF_N = (uint32_t) atoi(argv[6]);
    params.PREF_M = (uint32_t) atoi(argv[7]);
    trace_file = argv[8];

    // Open the trace file for reading.
    fp = fopen(trace_file, "r");
    if (fp == (FILE *) NULL) {
        // Exit with an error if file open failed.
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    // Print simulator configuration.
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
    printf("L1_SIZE:    %u\n", params.L1_SIZE);
    printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
    printf("L2_SIZE:    %u\n", params.L2_SIZE);
    printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
    printf("PREF_N:     %u\n", params.PREF_N);
    printf("PREF_M:     %u\n", params.PREF_M);
    printf("trace_file: %s\n", trace_file);
    //printf("===================================\n");
    printf("\n");
    // Read requests from the trace file and echo them back.
    while (fscanf(fp, "%c %x\n", &rw, &addr) ==
           2) {    // Stay in the loop if fscanf() successfully parsed two tokens as specified.
        if (rw == 'r') {
            //printf("r %x\n", addr);
        } else if (rw == 'w') {
            //printf("w %x\n", addr);
        } else {
            printf("Error: Unknown request type %c.\n", rw);
            exit(EXIT_FAILURE);
        }

        //------------------address parse------------------------
        if(params.PREF_N>0){
            prefetch_exists=true;
        }
        //determines # of sets
        no_sets_c1 = (params.L1_SIZE) / (params.L1_ASSOC * params.BLOCKSIZE);
        if(params.L2_SIZE>0) {
            no_sets_L2 = (params.L2_SIZE) / (params.L2_ASSOC * params.BLOCKSIZE);
            l2_dne = 0;
        }
        else{l2_dne=1;} //there is no L2

        //size of tag, index, and block offset for each set
        if(no_sets_c1>0){
            tag_size_c1=32-log2(no_sets_c1)-log2(params.BLOCKSIZE);
            index_size_c1=log2(no_sets_c1);
            boffset_size_c1=log2(params.BLOCKSIZE);
        }
        if(l2_dne==false){
            tag_size_L2=32-log2(no_sets_L2)-log2(params.BLOCKSIZE);
            index_size_L2=log2(no_sets_L2);
            boffset_size_L2=log2(params.BLOCKSIZE);
        }
        //set address and rw
        inst.addr=int(addr);
        inst.rw=rw;
        //extract and set tag bits- L1
        inst.tag=int(addr)>>(32-tag_size_c1);
        //printf("Tag: %x\n", inst.tag);
        mask = (1 << (32-tag_size_c1)) - 1;
        //extract and set index bits
        indexblock=(int(addr) & mask);
        inst.index=indexblock>>(32-(tag_size_c1+index_size_c1));
        //printf("Index: %x\n", inst.index);
        //extract and set block offset bits
        mask=(1<<(32-(tag_size_c1+index_size_c1)))-1;
        inst.block_offset=(int(addr) & mask);

        //extract and set tag bits- L2
        inst.tag2=int(addr)>>(32-tag_size_L2);
        //printf("Tag: %x\n", inst.tag2);
        mask = (1 << (32-tag_size_L2)) - 1;
        indexblock=(int(addr) & mask);
        inst.index2=indexblock>>(32-(tag_size_L2+index_size_L2));
        //printf("Index: %x\n", inst.index2);
        mask=(1<<(32-(tag_size_L2+index_size_L2)))-1;
        inst.block_offset2=(int(addr) & mask);


        //add instance to instruction list array
        inst_list.push_back(inst);
        //----------------------------------------------
    }

}

void bubbleSort(int arr[], int n)
{
    int i, j;
    bool swapped;
    for (i = 0; i < n-1; i++)
    {
        swapped = false;
        for (j = 0; j < n-i-1; j++)
        {
            if (arr[j] > arr[j+1])
            {
                swap(arr[j], arr[j+1]);
                swapped = true;
            }
        }

        // IF no two elements were swapped
        // by inner loop, then break
        if (swapped == false)
            break;
    }
}


int main (int argc, char *argv[]) {
    parse_file(argc, argv); //build memory vector and get cache parameters
    int* lru_counters1;
    int* lru_counters2;
    int* lru_countersp;
    int normal_r_misses=0;
    int normal_w_misses=0;
    //initialize caches
    c1.init_cache(1, params.L1_SIZE, params.L1_ASSOC, params.BLOCKSIZE);

    //initialize L2 if L2 parameters are specified
    if(params.L2_SIZE>0) {
        c2.init_cache(2, params.L2_SIZE, params.L2_ASSOC, params.BLOCKSIZE);
    }

    //initialize prefetch unit if parameters are specified
    if(prefetch_exists==true){
        prefetch.init_prefetch(params.PREF_M, params.PREF_N);
    }


    ///////////////////////////////////////////////////////
    // Issue the request to the c1 cache instance here.
    ///////////////////////////////////////////////////////
    for (int i=0; i<inst_list.size(); i++){

        //write
        if(inst_list[i].rw=='w'){
            c1.write_cache(inst_list[i]);
        }
        //read
        else if(inst_list[i].rw=='r'){
                c1.read_cache(inst_list[i]);
        }
        //-----
    }

    //float miss_rate1=(c1.read_misses+c1.write_misses)/(c1.reads+c1.writes);
    //print outputs
    //level 1
    printf("===== L1 contents =====\n");

    //make room for lru ordering arrays
    lru_counters1= (int*) malloc(c1.assoc*sizeof(int));
    if(l2_dne==false){
        lru_counters2=(int*) malloc(c2.assoc*sizeof(int));
    }
    if(prefetch_exists==true){
        lru_countersp=(int*) malloc(prefetch.unit.size()*sizeof(int));
    }

    if(l2_dne==true) {
        //populate array of lru counters to be sorted
        for (int i = 0; i < c1.num_sets; i++) {
            printf("set      %d:   ", i);
            for (int j = 0; j < c1.assoc; j++) {
                lru_counters1[j]=c1.set_arr[i].block_arr[j].lru_counter;
            }
            //sort the array
            bubbleSort(lru_counters1, c1.assoc);
            //mru -> lru order of array
            for(int k=c1.assoc-1; k>=0; k--){
                for(int l=0; l<c1.assoc; l++){
                    if(c1.set_arr[i].block_arr[l].lru_counter==lru_counters1[k]) {
                        (c1.set_arr[i].block_arr[l].dirty_bit==1)?(printf("%x D ", c1.set_arr[i].block_arr[l].tag)):(printf("%x ", c1.set_arr[i].block_arr[l].tag));                    }
                }
            }
            cout<<endl;
        }
    }

    //level 2 exists
    else {
        //cout<<"printing level 1"<<endl;
        for (int i = 0; i < c1.num_sets; i++) {
            printf("set      %d:   ", i);
            for (int j = 0; j < c1.assoc; j++) {
                lru_counters1[j]=c1.set_arr[i].block_arr[j].lru_counter;
            }
            //sort the array
            bubbleSort(lru_counters1, c1.assoc);
            //mru -> lru order of array
            for(int k=c1.assoc-1; k>=0; k--){
                for(int l=0; l<c1.assoc; l++){
                    if(c1.set_arr[i].block_arr[l].lru_counter==lru_counters1[k]) {
                        (c1.set_arr[i].block_arr[l].dirty_bit==1)?(printf("%x D ", c1.set_arr[i].block_arr[l].tag)):(printf("%x ", c1.set_arr[i].block_arr[l].tag));                    }
                }
            }
            cout<<endl;
        }
        printf("\n");
        printf("===== L2 contents =====\n");
        //cout<<"going thru l2"<<endl;
        for (int i = 0; i < c2.num_sets; i++) {
            printf("set      %d:   ", i);
            for (int j = 0; j < c2.assoc; j++) {
                lru_counters2[j]=c2.set_arr[i].block_arr[j].lru_counter;
            }
            //sort the array
            bubbleSort(lru_counters2, c2.assoc);
            //mru -> lru order of array
            for(int k=c2.assoc-1; k>=0; k--){
                for(int l=0; l<c2.assoc; l++){
                    if(c2.set_arr[i].block_arr[l].lru_counter==lru_counters2[k]) {
                        (c2.set_arr[i].block_arr[l].dirty_bit==1)?(printf("%x D ", c2.set_arr[i].block_arr[l].tag)):(printf("%x ", c2.set_arr[i].block_arr[l].tag));                    }
                }
            }
            cout<<endl;
        }

    }
    //printf("%d\n", miss_rate1);
    printf("\n");

    //display stream buffer
    if(prefetch_exists==true) {
        cout<<"===== Stream Buffer(s) contents ====="<<endl;
        for(int i=0; i<prefetch.unit.size(); i++){
            lru_countersp[i]=prefetch.unit[i].lru_counter;
        }
        bubbleSort(lru_countersp, prefetch.unit.size());
        for(int k=prefetch.unit.size()-1; k>=0; k--) {
            for (int i = 0; i < prefetch.unit.size(); i++) {
                if(prefetch.unit[i].lru_counter==lru_countersp[k]) {
                    prefetch.unit[i].sb.displayQueue();
                }
            }
            cout<<endl;
        }
        cout<<endl;
    }

    printf("===== Measurements =====\n");
    if(l2_dne==true) {
        normal_r_misses=c1.read_misses-c1.pref_reads;
        normal_w_misses=c1.write_misses-c1.pref_writes;
        printf("a. L1 reads:                   %d\n", c1.reads);
        printf("b. L1 read misses:             %d\n", normal_r_misses);
        printf("c. L1 writes:                  %d\n", c1.writes);
        printf("d. L1 write misses:            %d\n", normal_w_misses);
        printf("e. L1 miss rate:               %.4f\n",
               double(normal_r_misses + normal_w_misses) / double(c1.reads + c1.writes));
        printf("f. L1 writebacks:              %d\n", c1.writebacks);
        printf("g. L1 prefetches:              %d\n", c1.prefetches);
        printf("h. L2 reads (demand):          %d\n", 0);
        printf("i. L2 read misses (demand):    %d\n", 0);
        printf("j. L2 reads (prefetch):        %d\n", 0);
        printf("k. L2 read misses (prefetch):  %d\n", 0);
        printf("l. L2 writes:                  %d\n", 0);
        printf("m. L2 write misses:            %d\n", 0);
        printf("n. L2 miss rate:               %.4f\n", 0.0000);
        printf("o. L2 writebacks:              %d\n", 0);
        printf("p. L2 prefetches:              %d\n", 0);
        printf("q. memory traffic:             %d\n", normal_r_misses + normal_w_misses + c1.writebacks + c1.prefetches);
    }

    else{
        normal_r_misses=c1.read_misses-c1.pref_reads;
        normal_w_misses=c1.write_misses-c1.pref_writes;
        printf("a. L1 reads:                   %d\n", c1.reads);
        printf("b. L1 read misses:             %d\n", normal_r_misses);
        printf("c. L1 writes:                  %d\n", c1.writes);
        printf("d. L1 write misses:            %d\n", normal_w_misses);
        printf("e. L1 miss rate:               %.4f\n",
               double(normal_r_misses + normal_w_misses) / double(c1.reads + c1.writes));
        printf("f. L1 writebacks:              %d\n", c1.writebacks);
        printf("g. L1 prefetches:              %d\n", 0);
        printf("h. L2 reads (demand):          %d\n", c2.reads);
        normal_r_misses=c2.read_misses-c2.pref_reads;
        normal_w_misses=c2.write_misses-c2.pref_writes;
        printf("i. L2 read misses (demand):    %d\n", normal_r_misses);
        printf("j. L2 reads (prefetch):        %d\n", 0);
        printf("k. L2 read misses (prefetch):  %d\n", 0);
        printf("l. L2 writes:                  %d\n", c2.writes);
        printf("m. L2 write misses:            %d\n", normal_w_misses);
        printf("n. L2 miss rate:               %.4f\n", double(normal_r_misses) / double(c2.reads));
        printf("o. L2 writebacks:              %d\n", c2.writebacks);
        printf("p. L2 prefetches:              %d\n", c2.prefetches);
        printf("q. memory traffic:             %d\n", normal_r_misses + normal_w_misses + c2.writebacks + c2.prefetches);

    }


    return 0;
}


