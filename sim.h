#ifndef SIM_CACHE_H
#define SIM_CACHE_H
#include <string.h>
#include<vector>
#include<iostream>
#include<math.h>
using namespace std;


typedef
struct {
   uint32_t BLOCKSIZE;
   uint32_t L1_SIZE;
   uint32_t L1_ASSOC;
   uint32_t L2_SIZE;
   uint32_t L2_ASSOC;
   uint32_t PREF_N;
   uint32_t PREF_M;
} cache_params_t;

// Put additional data structures here as per your requirement.
//-------------------------INSTRUCTION LINE-------------------------
typedef
struct {
    char rw;
    uint32_t addr;
    unsigned long long tag;
    unsigned long long tag2;
    unsigned int index;
    unsigned int index2;
    unsigned int block_offset;
    unsigned int block_offset2;
} instruction_line;
//----------------------------------------------------------------------------



//------------------------BLOCK & FUNCTIONS-----------------------------------
class block{
public:
    unsigned long long tag;
    bool valid;
    bool dirty_bit;
    int lru_counter;
    block();
    instruction_line current_inst;
};
block::block(void){
    tag=0;
    valid=0;
    dirty_bit=0;
    lru_counter=0;
}
//---------------------------------------------------



//-------------------SET AND FUNCTIONS----------------
class SET{
public:
    int current_counter;
    int lru_block;
    int mru_block;
    vector<block>block_arr;
    SET();
    SET(int a);
    int set_number;
};

SET::SET() {
    current_counter = 0;
    lru_block=0;
    mru_block=0;
    set_number=0;
}

SET::SET(int a){
    block_arr.resize(a);
    current_counter=0;
    lru_block=0;
    mru_block=0;
    set_number=0;
}
//------------------------------------------------------------
class ring_queue{
    // Initialize front and rear
public:
    int rear, front;

    // Circular Queue
    int size;
    unsigned int *arr;
    void enQueue(int value);
    int deQueue();
    void displayQueue();
    //Queue();
    void resize_queue(int s);
};

void ring_queue::resize_queue(int s){
    front = rear = -1;
    size = s;
    arr = (unsigned int *) malloc(s*sizeof(int));
}


void ring_queue::enQueue(int value){
    if (front == -1){
        front = rear = 0;
        arr[rear] = value;
    }
    else if ((front == 0 && rear == size-1) ||
        (rear == (front-1)%(size-1))){
        printf("\nQueue is Full");
        return;
    }
    else if (rear == size-1 && front != 0){
        rear = 0;
        arr[rear] = value;
    }

    else{
        rear++;
        arr[rear] = value;
    }
}

// Function to delete element from Circular Queue
int ring_queue::deQueue(){
    if (front == -1){
        printf("\nQueue is Empty");
        return -1;
    }

    int data = arr[front];
    arr[front] = -1;
    if (front == rear){
        front = -1;
        rear = -1;
    }
    else if (front == size-1) {
        front = 0;
    }
    else {
        front++;
    }

    return data;
}

// Function displaying the elements
// of Circular Queue
void ring_queue::displayQueue(){
    if (front == -1){
        return;
    }
    if (rear >= front){
        for (int i = front; i <= rear; i++)
            printf("%x ",arr[i]);
    }
    else{
        for (int i = front; i < size; i++)
            printf("%x ", arr[i]);

        for (int i = 0; i <= rear; i++)
            printf("%x ", arr[i]);
    }
}



//----------------------CACHE AND FUNCTIONS-------------------
class CACHE{
public:
    int cache_level;
    int cache_size;
    int blocksize;
    int assoc;
    int num_sets;
    int reads;
    int read_misses;
    int writes;
    int write_misses;
    int writebacks;
    int prefetches;
    int pref_reads;
    int pref_writes;
    vector<SET> set_arr;
    CACHE();
    void init_cache(int l, int s, int a, int b);
    void write_cache(instruction_line inst);
    void read_cache(instruction_line inst);
    int find_lru(int set_num);
};
CACHE::CACHE() {
    int cache_level=0;
    int cache_size=0;
    int blocksize=0;
    int assoc=0;
    int num_sets=0;
    int reads=0;
    int read_misses=0;
    int writes=0;
    int write_misses=0;
    int writebacks=0;
    int prefetches=0;
    int pref_reads=0;
    int pref_writes=0;
}
void CACHE::init_cache(int l, int s, int a, int b) {
    num_sets=s/(a*b);
    set_arr.resize(num_sets);
    //
    for(int i=0; i<num_sets; i++){
        set_arr[i].block_arr.resize(a);
        set_arr[i].set_number=i;
    }
    reads=0;
    read_misses=0;
    writes=0;
    write_misses=0;
    cache_level=l;
    cache_size=s;
    assoc=a;
    blocksize=b;
}

int CACHE::find_lru(int set_num){
    int lru_block=0;
    for(int i=0; i<assoc; i++){
        if(set_arr[set_num].block_arr[i].lru_counter < set_arr[set_num].block_arr[lru_block].lru_counter){
            lru_block=i;
        }
    }
    set_arr[set_num].lru_block=lru_block;
    return lru_block;
}
//-------------------------STREAM BUFFER OBJECT & FUNCTIONS----------------------
class STREAM_BUFFER{
public:
    int lru_counter;
    int valid;
    ring_queue sb;
    STREAM_BUFFER();
    void init_buffer(int s);

    int find_sb_hit(unsigned int addr);
    void sb_hit_rearrange(unsigned int addr);
    void sb_miss_populate(unsigned int addr);
};

STREAM_BUFFER::STREAM_BUFFER(){
    lru_counter=0;
    valid=0;
}

void STREAM_BUFFER::init_buffer(int s){
    sb.resize_queue(s);
    lru_counter=0;
    valid=0;
}
extern CACHE c1;
extern CACHE c2;
extern bool l2_dne;
//find if an address is in the stream buffer
int STREAM_BUFFER::find_sb_hit(unsigned int addr) {
    int index=-1;
    for(int i=0; i<=sb.size; i++){
        if((addr>>(int(log2(c1.blocksize)))) == sb.arr[i]){
            index=i;
            break;
        }
    }
    return index;
}

//rearrange a hit in a stream buffer
void STREAM_BUFFER::sb_hit_rearrange(unsigned int addr){
    unsigned int parsed_address=addr>>(int(log2(c1.blocksize)));
    int hit_site=(find_sb_hit(addr)+1)%sb.size;
    //printf("Starting item %x through %x\n", parsed_address, parsed_address+sb.size);
    //printf("Address: %x\n", parsed_address);
    int num_of_adds=0;
    int o=1;
    int k=sb.front;
    do{
        num_of_adds++;
        k=(k+1)%sb.size;
    }
    while(k!=hit_site);
    //num_of_adds++;
    //stream buffer repopulate

    //clear queue of stale data
    for(int j=0; j<sb.size; j++){
        if(valid==1) {
            sb.deQueue();
        }
        //cout<<"dequeuing"<<endl;
    }
    for (int i=0; i<sb.size; i++){
        sb.enQueue(parsed_address+o);
        o++;
    }
    valid=1;

    //increment prefetch
    for (int m=0; m<num_of_adds; m++){
        if(l2_dne==true){
            c1.prefetches++;
        }
        else{
            c2.prefetches++;
        }
    }
}

//filling if address misses in prefetch buffer and in cache (case 1)
void STREAM_BUFFER::sb_miss_populate(unsigned int addr){
    unsigned int parsed_address=addr>>(int(log2(c1.blocksize)));

    int o=1;
    //clear queue
    for(int j=0; j<sb.size; j++){
        if(valid==1) {
            sb.deQueue();
        }
        //cout<<"dequeuing"<<endl;
    }
    for (int i=0; i<sb.size; i++){
        sb.enQueue(parsed_address+o);
        o++;
    }
    valid=1;

    //increment prefetch
    for(int j=0; j<sb.size; j++){
        if(l2_dne==true){
            c1.prefetches++;
        }
        else{
            c2.prefetches++;
        }
    }

}




//---------------------------PREFETCH UNIT----------------------
class PREFETCH_UNIT{
public:
    int pref_current_counter;
    vector<STREAM_BUFFER> unit;
    int prefetch_accesses;
    PREFETCH_UNIT();
    void init_prefetch(int M, int N);
    int find_prefetch_lru();
    void find_and_populate_lru_buffer(unsigned int addr);
    int multiple_hits(unsigned int addr);
    int find_invalid_buffer(unsigned int addr);
    void if_miss_cache_SB(unsigned int addr); //scenario 1 function
};

PREFETCH_UNIT::PREFETCH_UNIT() {
    pref_current_counter=0;
    prefetch_accesses=0;
}

void PREFETCH_UNIT::init_prefetch(int M, int N) {
    pref_current_counter=0;
    prefetch_accesses=0;
    unit.resize(N);
    for (int i=0; i<unit.size(); i++){
        unit[i].init_buffer(M);
    }
}

int PREFETCH_UNIT::find_prefetch_lru() {
    int lru_buffer=0;
    for(int i=0; i<unit.size(); i++){
        if (unit[i].valid == 0) continue;
        if(unit[i].lru_counter<unit[lru_buffer].lru_counter){
            lru_buffer=i;
        }
    }
    return lru_buffer;
}

void PREFETCH_UNIT::find_and_populate_lru_buffer(unsigned int addr){ //if it missed in prefetch
    int lru_buffer = 0;
    lru_buffer = find_prefetch_lru();
    unit[lru_buffer].sb_miss_populate(addr);
    unit[lru_buffer].lru_counter=pref_current_counter++;
}

void PREFETCH_UNIT::if_miss_cache_SB(unsigned int addr){ //scenario 1
    //look for invalid SBs
    int target_buffer=0;
    bool invalid_exists= false;
    for(int i=0; i<unit.size(); i++){
        if(unit[i].valid==0){
            target_buffer=i;
            invalid_exists=true;
            break;
        }
    }
    //populate invalid buffer. if all SBs are valid, repopulate the LRU buffer
    if(invalid_exists==true){ //populates invalid buffer and sets valid to 1
        unit[target_buffer].sb_miss_populate(addr);
    }
    else{
        target_buffer=find_prefetch_lru();
        unit[target_buffer].sb_miss_populate(addr);
    }
    unit[target_buffer].lru_counter=pref_current_counter++; //update to MRU
    //cout<<"Newly populated buffer is buffer: "<<target_buffer<<endl;
    //unit[target_buffer].sb.displayQueue();
    //cout<<endl;
}

// returns -1 if miss in prefetch unit
int PREFETCH_UNIT::multiple_hits(unsigned int addr){
    //find MRU block for redundancy cases
    int used_buff=-1;
    int used_buff_lru=-1;
    for(int i=0; i<unit.size(); i++){
        if((unit[i].find_sb_hit(addr))!=-1){ //if it hits in this stream buffer
            if(unit[i].lru_counter>used_buff_lru){
                used_buff=i;
                used_buff_lru = unit[i].lru_counter;
            }
        }

    }
    if (used_buff != -1) {
        unit[used_buff].lru_counter = pref_current_counter++;
    }
    return used_buff;
}

//-----------------------------CACHE METHODS---------------

//-------------GLOBALS------------
extern bool hit_L1;
extern bool prefetch_exists;
extern PREFETCH_UNIT prefetch;
//-----------------------------------------------
//--------------------WRITE----------------------
//-----------------------------------------------
void CACHE::write_cache(instruction_line inst){
    bool inst_in_cache=0;
    bool free_space=0;
    int current_lru_block=0;
    int set_num=0, block_num=0;
    unsigned long long comp_tag=0;
    int found=0;
    int found_in_stream_buffer=0;

    if(cache_level==1){
        set_num=inst.index;
        comp_tag=inst.tag;
    }
    else if(cache_level==2){
        set_num=inst.index2;
        comp_tag=inst.tag2;
    }



    set_arr[set_num].current_counter++;

    //checks if tag is in cache at given set

    for (int j=0; j<assoc; j++){
        if(set_arr[set_num].block_arr[j].tag==comp_tag){

            inst_in_cache=1;
            block_num=j;
            break;
        }
    }
    //--------------------PREFETCH CHECK------------------
    //check if address is in prefetch unit
    //conditions to check stream buffer:
    //1. If we're at L1 and L2 doesn't exist, and if prefetch is enabled
    //2. If L2 exists, prefetch is enabled, and we're in L2.
    if((cache_level==1 && l2_dne==true && prefetch_exists==true) || (cache_level==2 && l2_dne==false && prefetch_exists==true)){
        found_in_stream_buffer=prefetch.multiple_hits(inst.addr);
    }

    //now the real simulation starts...
    if(inst_in_cache==1){ //the line is in the cache. HIT PROTOCOL.
        writes++;

        set_arr[set_num].block_arr[block_num].tag=comp_tag;//redundant but whatev
        set_arr[set_num].block_arr[block_num].dirty_bit = 1;
        set_arr[set_num].block_arr[block_num].lru_counter=set_arr[set_num].current_counter;
        set_arr[set_num].block_arr[block_num].current_inst=inst;
        if((cache_level==1 && l2_dne==true && prefetch_exists==true) || (cache_level==2 && l2_dne==false && prefetch_exists==true)){
            //cout<<"Exploring prefetch: "<<endl;
            //scenario 3 or 4 for prefetch
            //(scenario 4)
            //if address hits in cache and prefetch unit
            if (found_in_stream_buffer != -1) { //scenario 4- rearrange buffer with hit
                prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
            } //else do nothing, carry on (scenario 3)
        }
    }

    else{ //the line is NOT in the cache.
        write_misses++;
        //is there an invalid block?
        for(int j=0; j<assoc; j++){
            if(set_arr[set_num].block_arr[j].valid==false){
                free_space=true;
                block_num=j;

                break;
            }
        }
        writes++;

        if(free_space == true){ //if there is an invalid block:
            if(cache_level==1 && l2_dne==false){ //cache level 1 doesn't have prefetch, let L2 do its thing
                //normal
                c2.read_cache(inst);
            }

            else if (cache_level == 1 && l2_dne==true){ //if there's no L2 cache and we're in the L1 cache
                if(prefetch_exists == true){
                    if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                        prefetch.if_miss_cache_SB(inst.addr);
                    }
                    else{ //scenario 2- miss in cache, hit in prefetch
                        pref_writes++;
                        prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                    }
                }
            }

            else if(cache_level==2){ //if we at the L2 cache
                if(prefetch_exists == true){
                    if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                        prefetch.if_miss_cache_SB(inst.addr);
                    }
                    else{ //scenario 2- miss in cache, hit in prefetch
                        pref_writes++;
                        prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                    }
                }
            }

            set_arr[set_num].block_arr[block_num].tag=comp_tag;
            set_arr[set_num].block_arr[block_num].dirty_bit=1;
            set_arr[set_num].block_arr[block_num].valid=1;
            set_arr[set_num].block_arr[block_num].lru_counter=set_arr[set_num].current_counter;
            set_arr[set_num].block_arr[block_num].current_inst=inst;
        }

        else{ //if no/there's no free space/invalid block
            //identify LRU block in set
            //the lru block is given by an index representing its position in the set

            //FIND LRU BLOCK FIND LRU BLOCK FIND LRU BLOCK
            current_lru_block = find_lru(set_num);

            //is dirty bit 0 in LRU?
            if (set_arr[set_num].block_arr[current_lru_block].dirty_bit == 0){
                // bring in the allocated block
                if (cache_level == 1 && l2_dne == false) {
                        c2.read_cache(inst);
                }

                else if (cache_level == 1 && l2_dne==true){ //if just L1 cache, no L2 exists
                    if(prefetch_exists == true){
                        if(found_in_stream_buffer == -1){ //miss in cache and stream buffer- scenario 1
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache, hit in stream buffer
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                            pref_writes++;
                        }
                    }
                }

                else if (cache_level == 2){
                    if(prefetch_exists == true){
                        if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache, hit in prefetch
                            pref_writes++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                }

                set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                set_arr[set_num].block_arr[current_lru_block].dirty_bit=1;
                set_arr[set_num].block_arr[current_lru_block].valid=1;
                set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                set_arr[set_num].block_arr[current_lru_block].current_inst=inst;
            }

            else{ //need to evict; issue L2 write
                if(cache_level==1 && l2_dne==false){ //
                    writebacks++;
                    //evict
                    c2.write_cache(set_arr[set_num].block_arr[current_lru_block].current_inst);
                    //issue c2 read to bring the allocated block in
                    c2.read_cache(inst);
                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=1;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;
                }

                else if(cache_level==1 && l2_dne==true){ //L2 not in the picture
                    if(prefetch_exists==true){//check if it is there in the sb
                        if(found_in_stream_buffer==-1){ //scenario 1- not in cache or stream buffer
                            writebacks++;
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache but hit in buffer
                            writebacks++;
                            pref_writes++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                    else { //prefetch isn't enabled
                        writebacks++;
                    }
                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=1;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;

                }

                else if(cache_level==2){
                    if(prefetch_exists==true){
                        if(found_in_stream_buffer==-1){ //scenario 1- not in cache or stream buffer
                            writebacks++;
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache but hit in buffer
                            writebacks++;
                            pref_writes++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                    else { //prefetch doesn't exist
                        writebacks++;
                    }

                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=1;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;

                }


            }

        }


    }

    find_lru(set_num);
}
//-----------------------------------------------
//-----------------READ--------------------------
//-----------------------------------------------
void CACHE::read_cache(instruction_line inst) {
    bool inst_in_cache=0;
    bool free_space=0;
    int current_lru_block=0;
    int set_num=0, block_num=0;
    unsigned long long comp_tag=0;
    int found=0;
    int found_in_stream_buffer=0;

    if(cache_level==1){
        set_num=inst.index;
        comp_tag=inst.tag;
    }
    else if(cache_level==2){
        set_num=inst.index2;
        comp_tag=inst.tag2;
    }

    set_arr[set_num].current_counter++;

    //checks if tag is in cache at given set

    for (int j=0; j<assoc; j++){
        if(set_arr[set_num].block_arr[j].tag==comp_tag){

            inst_in_cache=1;
            block_num=j;
            break;
        }
    }
    reads++;

    //--------------------PREFETCH CHECK------------------
    //check if address is in prefetch unit
    //conditions to check stream buffer:
    //1. If we're at L1 and L2 doesn't exist, and if prefetch is enabled
    //2. If L2 exists, prefetch is enabled, and we're in L2.
    if((cache_level==1 && l2_dne==true && prefetch_exists==true) || (cache_level==2 && l2_dne==false && prefetch_exists==true)){
        found_in_stream_buffer=prefetch.multiple_hits(inst.addr);
    }

    if(inst_in_cache==1){ //the line is in the cache.
        if((cache_level==1 && l2_dne==true && prefetch_exists==true) || (cache_level==2 && l2_dne==false && prefetch_exists==true)){
            //if(prefetch_exists==true) {//is prefetch implemented?
            //cout<<"Exploring prefetch: "<<endl;
            //scenario 3 or 4 for prefetch
            //(scenario 4)
            //if address hits in cache and prefetch unit
            if (found_in_stream_buffer != -1) { //scenario 4- rearrange buffer with hit
                prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
            } //else do nothing, carry on (scenario 3)
        }
        set_arr[set_num].block_arr[block_num].tag=comp_tag;//redundant but whatev
        set_arr[set_num].block_arr[block_num].lru_counter=set_arr[set_num].current_counter;
    }

    else{ //the line is NOT in the c1 cache.

        read_misses++;
        //is there an invalid block?
        for(int j=0; j<assoc; j++){
            if(set_arr[set_num].block_arr[j].valid==false){
                free_space=true;
                block_num=j;

                break;
            }
            else{

            }
        }
        //if yes:
        if(free_space == true){

            if(cache_level==1 && l2_dne==false){ //normal
                c2.read_cache(inst);
            } // else L2 doesnt exist,, read from memory

            else if (cache_level == 1 && l2_dne==true){ //if there's no L2 cache and we're in the L1 cache
                if(prefetch_exists == true){
                    if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                        prefetch.if_miss_cache_SB(inst.addr);
                    }
                    else{ //scenario 2- miss in cache, hit in prefetch
                        pref_reads++;
                        prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                    }
                }
            }

            else if(cache_level==2){ //if we at the L2 cache
                if(prefetch_exists == true){
                    if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                        prefetch.if_miss_cache_SB(inst.addr);
                    }
                    else{ //scenario 2- miss in cache, hit in prefetch
                        pref_reads++;
                        prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                    }
                }
            }

            set_arr[set_num].block_arr[block_num].tag=comp_tag;
            set_arr[set_num].block_arr[block_num].dirty_bit=0;
            set_arr[set_num].block_arr[block_num].valid=1;
            set_arr[set_num].block_arr[block_num].lru_counter=set_arr[set_num].current_counter;
            set_arr[set_num].block_arr[block_num].current_inst=inst;

        }

        else{ //if no/there's no free space/invalid block
            //identify LRU block in set
            //the lru block is given by an index representing its position in the set

            //find LRU block
            current_lru_block=find_lru(set_num);

            //is dirty bit 0 in LRU?
            if (set_arr[set_num].block_arr[set_arr[set_num].lru_block].dirty_bit == 0){

                if(cache_level==1 && l2_dne==false) { //if l2 cache exists
                    c2.read_cache(inst);
                } //else no L2 cache.. hence read from memory

                else if (cache_level == 1 && l2_dne==true){ //if just L1 cache, no L2 exists
                    if(prefetch_exists == true){
                        if(found_in_stream_buffer == -1){ //miss in cache and stream buffer- scenario 1
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache, hit in stream buffer
                            pref_reads++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                }

                else if (cache_level == 2){
                    if(prefetch_exists == true){
                        if(found_in_stream_buffer == -1){ //scenario 1- miss in both cache and prefetch
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache, hit in prefetch
                            pref_reads++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                }

                set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                set_arr[set_num].block_arr[current_lru_block].dirty_bit=0;
                set_arr[set_num].block_arr[current_lru_block].valid=1;
                set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                set_arr[set_num].block_arr[current_lru_block].current_inst=inst;
            }

            else{ //need to evict; issue L2 write
                if(cache_level==1 && l2_dne==false){ //??

                    writebacks++;
                    c2.write_cache(set_arr[set_num].block_arr[current_lru_block].current_inst);
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=0;
                    c2.read_cache(inst);

                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=0;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;
                }

                else if (cache_level==1 && l2_dne==true){
                    if(prefetch_exists==true){
                        if(found_in_stream_buffer==-1){ //scenario 1- not in cache or stream buffer
                            writebacks++;
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache but hit in buffer
                            writebacks++;
                            pref_reads++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                    else { //prefetch isn't enabled
                        writebacks++;
                    }
                    //Write the contents to memory
                    // Then read the contents of the current address from memory
                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=0;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;
                }

                if(cache_level==2){
                    if(prefetch_exists==true){
                        if(found_in_stream_buffer==-1){ //scenario 1- not in cache or stream buffer
                            writebacks++;
                            prefetch.if_miss_cache_SB(inst.addr);
                        }
                        else{ //scenario 2- miss in cache but hit in buffer
                            writebacks++;
                            pref_reads++;
                            prefetch.unit[found_in_stream_buffer].sb_hit_rearrange(inst.addr);
                        }
                    }
                    else { //prefetch doesn't exist
                        writebacks++;
                    }
                    //Write the contents to memory
                    // Then read the contents of the current address from memory
                    set_arr[set_num].block_arr[current_lru_block].tag=comp_tag;
                    set_arr[set_num].block_arr[current_lru_block].dirty_bit=0;
                    set_arr[set_num].block_arr[current_lru_block].valid=1;
                    set_arr[set_num].block_arr[current_lru_block].lru_counter=set_arr[set_num].current_counter;
                    set_arr[set_num].block_arr[current_lru_block].current_inst=inst;

                }

            }

        }
    }

    //UPDATE LRU
    find_lru(set_num);
}
#endif
