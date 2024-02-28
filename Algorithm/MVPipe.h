#ifndef MVPipe_H
#define MVPipe_H

#include "MultiAbstract.h"
#include "hash.h"
#include <iostream>

template<typename DATA_TYPE>
class MVPipe : public MultiAbstract<DATA_TYPE>{
private:
    typedef struct SBUCKET_type { //Total sum V(i, j) int sum;
        int sum;
        long count;
        long value;
        int kickout;
        DATA_TYPE key;
    } SBucket;
    struct MV_type {
        //Counter to count total degree
        int sum;
        //Counter table
        SBucket ***counts;
        //Outer sketch depth and width
        int depth;
        int* width;
        int hash_shift;
        uint32_t *hash, *scale, *hardner;
    };
    MV_type mv_;
    const uint32_t *masks;
    const uint32_t MV_SEED = 111;

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
    uint32_t masks_byte[5]={
    0xFFFFFFFFu, 0xFFFFFF00u, 0xFFFF0000u, 0xFF000000u, 0x00000000u,};

    void _update(int row, DATA_TYPE key, int val){
        unsigned long bucket = 0;
        // if key is not in the i-th row, then insert it in the i+1-th row
        // depth equals to the hierarchy level
        DATA_TYPE tempkey = key;
        int i = 0;
        for (i = row; i < mv_.depth; i++) {
            if (val == 0) break;
            DATA_TYPE hierar_key = tempkey & masks[i];
            if (i < mv_.hash_shift)
                bucket = hash(hierar_key, mv_.hardner[i]) % mv_.width[i];
            else {
                bucket = hierar_key >> (i*33/(mv_.depth-1));
            }
            MVPipe::SBucket *sbucket = mv_.counts[i][bucket];
            sbucket->sum += val;
            if(hierar_key == sbucket->key) {
                sbucket->count += val;
                sbucket->value += val;
                break;
            } else {
                sbucket->count -= val;
                if (sbucket->count < 0) {
                    //send the original candidate key to the next row
                    tempkey = sbucket->key;
                    long tempval = sbucket->value;
                    sbucket->value = val;
                    val = tempval;
                    sbucket->key = hierar_key;
                    sbucket->count = -sbucket->count;
                }
            }
        }
    }


public: 
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    MVPipe(uint32_t _MEMORY, int _depth, std::string _name = "FullAncestry"){
        if(_depth == 33)
            masks = masks_bit;    
        else if(_depth == 5)
            masks = masks_byte;    
        else{
            printf("Error: wrong layer num! It should be 5 or 33.\n");
            exit(0);
        }

        int *width = new int[_depth];
        int total_bucket = _MEMORY * 8 / 128;
        int row_bucket = total_bucket/_depth;
        int flag = 0;
        for (int i  = _depth; i > 0; i--) {
            long total = (long)1<<((_depth-i)*33/(_depth-1));
            if (total < (long)row_bucket) {
                flag = _depth-i;
                width[i-1] = (int)total;
                total_bucket -= (int)total;
                row_bucket = total_bucket/(i-1);
            } else {
                width[i-1] = row_bucket;
            }
        }
        
        mv_.depth = _depth;
        mv_.width = width;
        //denote the row that using hash function to hash keys to the buckets
        mv_.hash_shift = _depth-flag-1;
        mv_.sum = 0;
        mv_.counts = new SBucket **[_depth];
        for (int i = 0; i < _depth; i++) {
            mv_.counts[i] = (SBucket**)calloc(width[i], sizeof(SBucket*));
            for (int j = 0; j < width[i]; j++) {
                mv_.counts[i][j] = (SBucket*)calloc(1, sizeof(SBucket));
                memset(mv_.counts[i][j], 0, sizeof(SBucket));
            }
        }
    
        mv_.hash = new uint32_t[_depth];
        mv_.scale = new uint32_t[_depth];
        mv_.hardner = new uint32_t[_depth];
        uint32_t seed = MV_SEED;
        for (int i = 0; i < _depth; i++)
            mv_.hash[i] = hash(MV_SEED ,seed++);
        for (int i = 0; i < _depth; i++)
            mv_.scale[i] = hash(MV_SEED, seed++);
        for (int i = 0; i < _depth; i++) 
            mv_.hardner[i] = hash(MV_SEED, seed++);
    }

    void Insert(const DATA_TYPE& item){
        _update(0, item, 1);
    }

    void AllQuery_allayel(HashMap estMp[], COUNT_TYPE threshold){
        //1. push all keys in the low level to high level if its counts do not
        //exceed the threshold
        for (int i = 0; i < mv_.depth; i++) {
            for (int j = 0; j < mv_.width[i]; j++) {
                DATA_TYPE key = mv_.counts[i][j]->key;
                int resval = 0;
                resval = (mv_.counts[i][j]->sum + mv_.counts[i][j]->count)/2;
                //check false postive
                int bucket = 0;
                int addval = mv_.counts[i][j]->value;
                for (int r = 0; r < 4; r++) {
                    if (i+1+r >= mv_.depth) break;
                    DATA_TYPE rkey = key & masks[i+1+r];
                    if (i+1+r < mv_.hash_shift)
                        bucket = hash(rkey, mv_.hardner[i+1+r]) % mv_.width[i+1+r];
                    else {
                        bucket = rkey >> ((i+1+r)*33/(mv_.depth-1));
                    }
                    int est = 0;
                    if (rkey == mv_.counts[i+1+r][bucket]->key) {
                        est = (mv_.counts[i+1+r][bucket]->sum + mv_.counts[i+1+r][bucket]->count)/2 + addval;
                        addval += mv_.counts[i+1+r][bucket]->value;
                    } else {
                        est = (mv_.counts[i+1+r][bucket]->sum - mv_.counts[i+1+r][bucket]->count)/2 + addval;
                    }
                    resval = resval > est ? est : resval;
                }
 
                if (resval > threshold) {
                    //add back descendant
                    for(int di = i; di >= 0; --di)
                        for(auto it = estMp[mv_.depth-1-di].begin(); it != estMp[mv_.depth-1-di].end(); it++){
                        //if it is descendants
                        DATA_TYPE dkey = it->first;
                        int bucket = 0;
                        if ((dkey&masks[i]) == key) {
                            if (di < mv_.hash_shift)
                                bucket = hash(dkey, mv_.hardner[di]) % mv_.width[di];
                            else {
                                bucket = dkey >> (di*33/(mv_.depth-1));
                            }
                            resval += mv_.counts[di][bucket]->value;
                        }
                    }
                    estMp[mv_.depth-1-i][key] = resval;
                } else { // it is not a heavy hitter, then aggregate its value to the next level
                    _update(i+1, mv_.counts[i][j]->key, mv_.counts[i][j]->value);
                }
            }
        }
    }

    HashMap AllQuery(){
        HashMap null;
        return null;
    } 
    ~MVPipe(){
        for (int i = 0; i < mv_.depth; i++) {
            for (int j = 0; j < mv_.width[i]; j++) {
                free(mv_.counts[i][j]);
            }
            free(mv_.counts[i]);
        }
        delete [] mv_.width;
        delete [] mv_.hash;
        delete [] mv_.scale;
        delete [] mv_.hardner;
        delete [] mv_.counts;
    }
};

#endif
