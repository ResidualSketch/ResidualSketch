#ifndef FullAncestry_H
#define FullAncestry_H
 
#include "MultiAbstract.h"
#include "alloc.h"
#include "hash.h"
#include <iostream>

extern int maxtotal;

/*
Note: different from other sketch in HHHBench.h, 
FullAncestry's layer 0 represents 255.255.255.255, which is layer 32 in HHHBench.h
while layer 32 represents *.*.*.*, , which is layer 0 in HHHBench.h
*/
template<typename DATA_TYPE>
class FullAncestry : public MultiAbstract<DATA_TYPE>{
private:
    struct Counter_f {
        DATA_TYPE item; //an ip address
        int g; //lower bound on the count of item that is not counted elsewhere in the table
        int d; //upper bound on the count of item before it was inserted
        //g+d is upper bound on the count of item not accounted for elsewhere
        int m; //upper bound on count of any missing child of item
        Counter_f *next; //hash table linked list
        int s; //counts passed up
        int children; //number of children
    };
    Counter_f ** hashtable[33];
    int htsize;
    double epsilon;
    int N;
    const uint32_t *masks;
    int NUM_MASKS;
    const uint32_t FULL_SEED = 111;
 
    uint32_t masks_bit[33]={
    0xFFFFFFFFu << 0,0xFFFFFFFFu << 1,0xFFFFFFFFu << 2,0xFFFFFFFFu << 3,
    0xFFFFFFFFu << 4,0xFFFFFFFFu << 5,0xFFFFFFFFu << 6,0xFFFFFFFFu << 7,
    0xFFFFFFFFu << 8,0xFFFFFFFFu << 9,0xFFFFFFFFu << 10,0xFFFFFFFFu << 11,
	0xFFFFFFFFu << 12,0xFFFFFFFFu << 13,0xFFFFFFFFu << 14,0xFFFFFFFFu << 15,
	0xFFFFFFFFu << 16,0xFFFFFFFFu << 17,0xFFFFFFFFu << 18,0xFFFFFFFFu << 19,
	0xFFFFFFFFu << 20,0xFFFFFFFFu << 21,0xFFFFFFFFu << 22,0xFFFFFFFFu << 23,
	0xFFFFFFFFu << 24,0xFFFFFFFFu << 25,0xFFFFFFFFu << 26,0xFFFFFFFFu << 27,
	0xFFFFFFFFu << 28,0xFFFFFFFFu << 29,0xFFFFFFFFu << 30,0xFFFFFFFFu << 31,0x00000000u
    };
    uint32_t masks_byte[33]={
    0xFFFFFFFFu, 0xFFFFFF00u, 0xFFFF0000u, 0xFF000000u, 0x00000000u,};

    int max(int a, int b) {return (a >= b ? a : b);}
    int haschild(Counter_f * p) {return (p->children > 0);}

    void _update(DATA_TYPE item, int mask, int mupdate, int supdate, int count, int cupdate) {
        int i, hashi = ((uint32_t) hash(item, FULL_SEED)) % htsize;
        // std::cout<< hashi << " " << htsize << std::endl;

        Counter_f ** p = &hashtable[mask][hashi];
        Counter_f * q, * ni;
        // std::cout<<"here\n"<<std::endl;
        while (*p != NULL) {
            // std::cout<<"here: "<<p<<std::endl;
            /*if key(ip) match, then update and return directly */
            if ((*p)->item == item) { 
                // std::cout<<"here\n"<<std::endl;
                (*p)->g += count;
                (*p)->m = max((*p)->m, mupdate);
                (*p)->s += supdate;
                (*p)->children += cupdate;
                return;
            }
            // std::cout<<"here\n"<<std::endl;
            p = &((*p)->next);
        } 
        // std::cout<<"here\n"<<std::endl;
        //now *p = NULL so insert
        ni = (Counter_f *)(CALLOC(1, sizeof(Counter_f)));
        //insert parent first
        if (mask+1 < NUM_MASKS) _update(item & masks[mask+1], mask+1, 0, 0, 0, 1);
        ni->children = cupdate;
        ni->item = item;
        ni->g = count;
        //ni->d = ancestor(**p).m
        ni->d = (int) (epsilon*N); //value if there is no ancestor
        // std::cout<<"here\n"<<std::endl;
        
        for (i = mask+1; i < NUM_MASKS; i++) {
            //if item & masks[i] can be found use its m value
            hashi = ((uint32_t) hash(item & masks[i], FULL_SEED)) % htsize;
            q = hashtable[i][hashi];
            while (q != NULL && q->item != (item & masks[i])) q = q->next;
            //now either q = NULL or q is our item
            if (q != NULL) {
                ni->d = q->m;
                break;
            } 
        }
        ni->m = max(ni->d, mupdate);
        ni->next = NULL;
        ni->s = supdate;
        *p = ni; //inserted
    }

    void compress() {
        int i, j;
        Counter_f ** p, *q;
        for (i = 0; i < NUM_MASKS; i++) {
            for (j = 0; j < htsize; j++) {
                p = &hashtable[i][j]; 
                while (*p != NULL) {
                        if ((*p)->g + (*p)->d < epsilon*N && !haschild(*p) && i != NUM_MASKS-1) {
                        //delete *p
                        q = *p;
                        *p = q->next;
                        if (i+1 < NUM_MASKS) _update(q->item & masks[i+1], i+1, q->g + q->d, 0, q->g, -1);
                        FREE(q);		
                    } else {
                        p = &((*p)->next);
                    }
                }
            }
        }
    }
 
    void postprocess(std::unordered_map<DATA_TYPE, COUNT_TYPE> estMp[]) {
        for (int i = 1; i < NUM_MASKS; ++i){
            for (auto it = estMp[i].begin(); it != estMp[i].end(); it++){
                for (int j = i-1; j >= 0; --j){
                    for (auto iter = estMp[j].begin(); iter != estMp[j].end(); iter++)
                    {
                        if ((it->first & masks[NUM_MASKS-1-j]) == iter->first)
                        {
                            // std::cout<< "here" << std::endl;
                            estMp[j][iter->first] += it->second; //原始是加上lower，这里是加上upper，会出现问题（过估计）
                        }
                    }
                }
            }
        }
    }
   
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    FullAncestry(double _MEMORYorEPS, uint32_t _NUM_MASKS, std::string _name = "FullAncestry"){
        maxtotal = -1;
        if(_NUM_MASKS == 33)
            masks = masks_bit;    
        else if(_NUM_MASKS == 5)
            masks = masks_byte;    
        else{
            printf("Error: wrong layer num! It should be 5 or 33.\n");
            exit(0);
        }
        this->name = _name;
        this->NUM_MASKS = _NUM_MASKS;
        N = 0;

        if(_MEMORYorEPS > 1){//用内存初始化
            htsize = (uint32_t)_MEMORYorEPS * 2 / NUM_MASKS / (sizeof(Counter_f*)+sizeof(Counter_f));
            epsilon = 1 / (double)htsize;
        }
        else{//用阈值初始化
            epsilon = _MEMORYorEPS;
            htsize = (1 + (int) (1.0 / epsilon)) | 1;
        }

        int i;		
        for (i = 0; i < NUM_MASKS; i++)
            hashtable[i] = (Counter_f **)CALLOC(htsize, sizeof(Counter_f*));
        _update(0, NUM_MASKS-1, 0, 0, 0, 0);
    }

    void Insert(const DATA_TYPE& item){
        _update(item, 0, 0, 0, 1, 0);
        N+=1;
        // if (N % htsize == 0) compress();
        //only compress sometimes (not complete yet, which time to compress()? )
        if (N % ((int) (1.0/epsilon)) == 0) compress(); 
    }
 
    void AllQuery_allayel(HashMap estMp[], COUNT_TYPE threshold){
        int i, j;
        Counter_f * p;
        for (i = 0; i < NUM_MASKS; i++) 
            for (j = 0; j < htsize; j++) 
                for (p = hashtable[i][j]; p != NULL; p = p->next) 
                    p->s = 0;
        for (i = 0; i < NUM_MASKS; i++) {
            for (j = 0; j < htsize; j++) {
                for (p = hashtable[i][j]; p != NULL; p = p->next) {
                    if (p->g + p->d + p->s >= threshold) {
                        estMp[NUM_MASKS-1-i][p->item] = p->g + p->d + p->s;
                    } else if (i != NUM_MASKS - 1) { //pass up count
                        _update(p->item & masks[i+1], i + 1, 0, p->g + p->s, 0, 0);
                    }
                }
            }
        }
        compress(); //to get rid of all the garbage
        postprocess(estMp);

		int memory=maxmemusage();
        printf("real memory: %dKB\n", memory/1024);
        printf("real memory: %d\n", memory);
        return;
    }

    uint32_t query_mem(){
        return maxmemusage();
    }

    HashMap AllQuery(){
        HashMap null;
        return null;
    }
    ~FullAncestry(){
        int i, j;
        Counter_f * p,  *q;
        for (i = 0; i < NUM_MASKS; i++) {
            for (j = 0; j < htsize; j++) {
                p = hashtable[i][j];
                while (p != NULL) {
                    q = p->next;
                    FREE(p);
                    p = q;
                }
            }
            FREE(hashtable[i]);
        }
    }
};

#endif
