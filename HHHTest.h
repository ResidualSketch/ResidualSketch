#ifndef HHHTEST_H
#define HHHTEST_H

#include <vector>
#include <fstream>
#include <iostream>
#include <cstdio>
 
#include "SpaceSaving.h"
#include "FullAncestry.h"
#include "assert.h"

#include "CocoSketch.h"
#include "USS.h"
#include <iomanip>
#include "MVPipe.h"
 
#include "MMap.h"

#define HHHOtherSketch SpaceSaving
 
class HHHTEST
{
public:
    HHHTEST(std::string PATH, std::string name, int layer, int SrcOrDst){
        LAYER = layer;
        std::string typee, Src;
        if(layer == 33){
            MASK = MASK_bit;
            bucketbound = bucketbound_bit;
            typee = "1D_bit/";
        }    
        else if(layer == 5){
            MASK = MASK_byte;    
            bucketbound = bucketbound_byte;
            typee = "1D_byte/";
        }
        else{
            std::cout << "Error: wrong layer num! It should be 5 or 33." << std::endl;
            exit(0);
        }

        oridataset = read_data(PATH.c_str(), 100000000, &length);

        dataset = new uint32_t[length+1];
        if(SrcOrDst == 1){
            for (uint64_t i = 0; i < length; ++i) dataset[i] = oridataset[i].srcIP();
            Src = "src_";
        }
        else{
            for (uint64_t i = 0; i < length; ++i) dataset[i] = oridataset[i].dstIP();
            Src = "dst_";
        }
        std::cout << "dataset: " << name << ", " << Src <<  " length: " << length << std::endl;
        outpath = "./output/HHH/" + typee + name + "/"+ Src;

        for(int i=0;i<12;i++)
            flag[i]=0;

        for (uint64_t i = 0; i < length; ++i)
        {
            mp[LAYER - 1][dataset[i]] += 1;
        }
        for (auto it = mp[LAYER - 1].begin(); it != mp[LAYER - 1].end(); ++it)
            for (uint32_t i = 0; i < LAYER - 1; ++i)
                mp[i][it->first & MASK[i]] += it->second;
    }

    ~HHHTEST()
    {
        delete oridataset;
        delete dataset;
    }

    void HHHRHHH(COUNT_TYPE threshold)
    {
        std::string algo = "RHHH";
        emptyfile(algo, flag[0]);
        MultiAbstract<uint32_t> *sketch[33];

        uint32_t mem = 0;
        uint32_t needbucket = length / threshold;
        for (uint32_t i = 0; i < LAYER; ++i)
        {
            uint32_t rowbucket = needbucket;
            sketch[i] = new HHHOtherSketch<uint32_t>(StreamSummary<uint32_t,COUNT_TYPE>::Size2Memory(rowbucket)); 
            mem += rowbucket * 3 * 8 ;
        }

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            uint32_t j = randomGenerator() % LAYER;
            sketch[j]->Insert(dataset[i] & MASK[j]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];

       for(uint32_t i = 0;i < LAYER; ++i) {
            estMp[i] = sketch[i]->AllQuery();
            for(auto it = estMp[i].begin(); it != estMp[i].end();++it){
                estMp[i][it->first] = estMp[i][it->first] * LAYER;
            }
        } 

        CompareHHH(estMp, threshold, algo, mem);

        for (uint32_t i = 0; i < LAYER; ++i)
            delete sketch[i];
    }

    void HHHRHHH(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "RHHH";
        emptyfile(algo, flag[0]);
        MultiAbstract<uint32_t> *sketch[33];
        uint32_t mem = MEMORY / LAYER;

        for (uint32_t i = 0; i < LAYER; ++i)
        {
            sketch[i] = new HHHOtherSketch<uint32_t>(mem);
        }

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            uint32_t j = randomGenerator() % LAYER;
            // for(uint32_t j = 0; j < LAYER; ++j)
            sketch[j]->Insert(dataset[i] & MASK[j]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];

       for(uint32_t i = 0;i < LAYER; ++i) {
            estMp[i] = sketch[i]->AllQuery();
            for(auto it = estMp[i].begin(); it != estMp[i].end();++it){
                estMp[i][it->first] = estMp[i][it->first] * LAYER;
            }
        } 

        CompareHHH(estMp, threshold, algo, MEMORY);

        for (uint32_t i = 0; i < LAYER; ++i)
            delete sketch[i];
    }

    void HHHFullAncestry(COUNT_TYPE threshold)
    {
        std::string algo = "Full";
        emptyfile(algo, flag[1]);

        MultiAbstract<uint32_t> *sketch = new FullAncestry<uint32_t>((double)threshold / length, LAYER);
        
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch->Insert(dataset[i]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        sketch->AllQuery_allayel(estMp, threshold);
        uint32_t mem = 100;

        CompareHHH(estMp, threshold, algo, mem);

        delete sketch;
    }

    void HHHFullAncestry(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Full";
        emptyfile(algo, flag[1]);
        MultiAbstract<uint32_t> *sketch = new FullAncestry<uint32_t>(MEMORY, LAYER);
        
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch->Insert(dataset[i]);
        } 
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        sketch->AllQuery_allayel(estMp, threshold);
        
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch;
    }

    void HHH_HHH12(COUNT_TYPE threshold){
        std::string algo = "HHH12";
        emptyfile(algo, flag[2]);
                           
        MultiAbstract<uint32_t> *sketch[33];
  
        uint32_t mem = 0;
        uint32_t needbucket = length / threshold;
        for (uint32_t i = 0; i < LAYER; ++i)
        {
            uint32_t rowbucket = std::min(needbucket, bucketbound[i]);
            sketch[i] = new HHHOtherSketch<uint32_t>(StreamSummary<uint32_t,COUNT_TYPE>::Size2Memory(rowbucket)); 
            mem += rowbucket * 3 * 8 ;
        }

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
            for (uint32_t j = 0; j < LAYER; ++j)
                sketch[j]->Insert(dataset[i] & MASK[j]);
        
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        for (uint32_t i = 0; i < LAYER; ++i)
        {
            estMp[i] = sketch[i]->AllQuery();
        }
        CompareHHH(estMp, threshold, algo, mem);

        for (uint32_t i = 0; i < LAYER; ++i)
            delete sketch[i];
    }

    void HHH_HHH12(uint32_t MEMORY, COUNT_TYPE threshold){
        std::string algo = "HHH12";
        emptyfile(algo, flag[2]);                       
        MultiAbstract<uint32_t> *sketch[33];   

        uint32_t total_bucket = MEMORY / 8; //sizeof(Counter) = 8
        uint32_t row_bucket = total_bucket/LAYER;
        for (int i = 0; i < LAYER; ++i) {
            uint32_t total = bucketbound[i];
            if (total < row_bucket) {
                sketch[i] = new HHHOtherSketch<uint32_t>(total*12);
                total_bucket -= total;
                row_bucket = total_bucket/(LAYER-i-1);
            } else {
                sketch[i] = new HHHOtherSketch<uint32_t>(row_bucket*12);
            }
        }
 
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
            for (uint32_t j = 0; j < LAYER; ++j)
                sketch[j]->Insert(dataset[i] & MASK[j]);
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        for (uint32_t i = 0; i < LAYER; ++i)
        {
            estMp[i] = sketch[i]->AllQuery();
        }
        CompareHHH(estMp, threshold, algo, MEMORY);

        for (uint32_t i = 0; i < LAYER; ++i)
            delete sketch[i];
    }


    void HHHMVPipe(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "MVPipe";
        emptyfile(algo, flag[3]);
        MultiAbstract<uint32_t> *sketch = new MVPipe<uint32_t>(MEMORY, LAYER);
        
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch->Insert(dataset[i]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        sketch->AllQuery_allayel(estMp, threshold);
        
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch;
    }

    void HHHCOCO(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "coco";
        emptyfile(algo, flag[4]);
        MultiAbstract<uint32_t> *sketch = new CocoSketch<uint32_t>(MEMORY);
        
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch->Insert(dataset[i]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];

        estMp[LAYER - 1] = sketch->AllQuery();

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
            for (uint32_t i = 0; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;

        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch;
    }

    void HHHUSS(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "USS";
        emptyfile(algo, flag[5]);
        MultiAbstract<uint32_t> *sketch = new USS<uint32_t>(1.5 *MEMORY);
        
        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch->Insert(dataset[i]);
        }
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;

        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];

        estMp[LAYER - 1] = sketch->AllQuery();

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
            for (uint32_t i = 0; i < LAYER - 1; ++i){
                estMp[i][it->first & MASK[i]] += it->second;
            }

        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch;
    }



    void HHHKey(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Residual";
        emptyfile(algo, flag[6]);
         
        int key_layer, key_layer_prefix;
        if(LAYER == 33){
            key_layer = 12;    // = 24 if mawi
            key_layer_prefix = 12;
        }
        else{
            key_layer = 2;
            key_layer_prefix = 2 * BYTE_LENGTH;
        }

        CocoSketch<uint32_t> *sketch_key = new CocoSketch<uint32_t>(MEMORY / 2, 2, key_layer_prefix, threshold );
        CocoSketch<uint32_t> *sketch_32 = new CocoSketch<uint32_t>(MEMORY / 2 , 2);

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            int cnt_val = sketch_32->Insert_32(dataset[i]);

            if(cnt_val < threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer]);
             }else if(cnt_val == threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer], true);
             }
        }
        
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;
 
        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key;

        estMp[LAYER - 1] = sketch_32->AllQuery();
        estMp_key = sketch_key->AllQuery_key_layer(estMp[LAYER - 1]);
        

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
        {
            for (uint32_t i = key_layer+1; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;
        }
        
        for (auto it = estMp_key.begin(); it != estMp_key.end(); ++it)
        {
            for (uint32_t i = 0; i <= key_layer; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch_key;
        delete sketch_32;
    }

    void HHHKey_without(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Residual_without_Connection";
        emptyfile(algo, flag[7]);
         
        int key_layer, key_layer_prefix;
        if(LAYER == 33){
            key_layer = 12;
            key_layer_prefix = 12;
        }
        else{
            key_layer = 2;
            key_layer_prefix = 2* BYTE_LENGTH;
        }

        CocoSketch<uint32_t> *sketch_key = new CocoSketch<uint32_t>(MEMORY / 2, 2, key_layer_prefix, threshold );
        CocoSketch<uint32_t> *sketch_32 = new CocoSketch<uint32_t>(MEMORY / 2 , 2);

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            sketch_32->Insert(dataset[i]);
            sketch_key->Insert(dataset[i] & MASK[key_layer]);
        }
        
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;
 
        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key;

        estMp[LAYER - 1] = sketch_32->AllQuery();
        estMp_key = sketch_key->AllQuery();

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
        {
            for (uint32_t i = key_layer+1; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;
        }
        
        for (auto it = estMp_key.begin(); it != estMp_key.end(); ++it)
        {
            for (uint32_t i = 0; i <= key_layer; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch_key;
        delete sketch_32;
    }


    void HHHKey_three(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Residual_with_three_layers";
        emptyfile(algo, flag[8]);
         
        int key_layer, key_layer2;
        if(LAYER == 33){
            key_layer = 12;
            key_layer2 = 24;
        }
        else{
            std::cout << "No need to build more than 2 layers in byte condition." << std::endl;
            exit(1);
        }

        CocoSketch<uint32_t> *sketch_key = new CocoSketch<uint32_t>(MEMORY / 3, 2, key_layer, threshold );
        CocoSketch<uint32_t> *sketch_key2 = new CocoSketch<uint32_t>(MEMORY / 3, 2, key_layer2, threshold );
        CocoSketch<uint32_t> *sketch_32 = new CocoSketch<uint32_t>(MEMORY / 3 , 2);

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            int cnt_val = sketch_32->Insert_32(dataset[i]);

            if(cnt_val < threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer]);
                 sketch_key2->Insert_key_layer(dataset[i] & MASK[key_layer2]);
             }else if(cnt_val == threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer], true);
                 sketch_key2->Insert_key_layer(dataset[i] & MASK[key_layer2], true);
             }
        }
        
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;
 
        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key;
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key2;

        estMp[LAYER - 1] = sketch_32->AllQuery();
        estMp_key2 = sketch_key2->AllQuery_key_layer(estMp[LAYER -1]);
        estMp_key = sketch_key->AllQuery_key_layer(estMp[LAYER - 1]);
        

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
        {
            for (uint32_t i = key_layer2+1; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;
        }
        for (auto it = estMp_key2.begin(); it != estMp_key2.end(); ++it)
        {
            for (uint32_t i = key_layer + 1; i <= key_layer2; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        
        for (auto it = estMp_key.begin(); it != estMp_key.end(); ++it)
        {
            for (uint32_t i = 0; i <= key_layer; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch_key;
        delete sketch_key2;
        delete sketch_32;
    }


    void HHHKey_four(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Residual_with_four_layers";
        emptyfile(algo, flag[9]);
         
        int key_layer, key_layer2, key_layer3;
        if(LAYER == 33){
            key_layer = 8;
            key_layer2 = 12;
            key_layer3 = 24;
        }
        else{
            std::cout << "No need to build more than 2 layers in byte condition." << std::endl;
            exit(1);
        }

        CocoSketch<uint32_t> *sketch_key = new CocoSketch<uint32_t>(MEMORY / 4, 2, key_layer, threshold );
        CocoSketch<uint32_t> *sketch_key2 = new CocoSketch<uint32_t>(MEMORY / 4, 2, key_layer2, threshold );
        CocoSketch<uint32_t> *sketch_key3 = new CocoSketch<uint32_t>(MEMORY / 4, 2, key_layer3, threshold );
        CocoSketch<uint32_t> *sketch_32 = new CocoSketch<uint32_t>(MEMORY / 4 , 2);

        TP start = now();
        for (uint32_t i = 0; i < length; ++i)
        {
            int cnt_val = sketch_32->Insert_32(dataset[i]);

            if(cnt_val < threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer]);
                 sketch_key2->Insert_key_layer(dataset[i] & MASK[key_layer2]);
                 sketch_key3->Insert_key_layer(dataset[i] & MASK[key_layer3]);
             }else if(cnt_val == threshold){
                 sketch_key->Insert_key_layer(dataset[i] & MASK[key_layer], true);
                 sketch_key2->Insert_key_layer(dataset[i] & MASK[key_layer2], true);
                 sketch_key3->Insert_key_layer(dataset[i] & MASK[key_layer3], true);
             }
        }
        
        TP end = now();

        std::cout << algo << " " << length / (durationms(end, start)) << std::endl;
 
        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key;
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key2;
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key3;

        estMp[LAYER - 1] = sketch_32->AllQuery();
        estMp_key2 = sketch_key2->AllQuery_key_layer(estMp[LAYER -1]);
        estMp_key3 = sketch_key3->AllQuery_key_layer(estMp[LAYER -1]);
        estMp_key = sketch_key->AllQuery_key_layer(estMp[LAYER - 1]);
        

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
        {
            for (uint32_t i = key_layer3+1; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;
        }
        for (auto it = estMp_key3.begin(); it != estMp_key3.end(); ++it)
        {
            for (uint32_t i = key_layer2 + 1; i <= key_layer3; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        for (auto it = estMp_key2.begin(); it != estMp_key2.end(); ++it)
        {
            for (uint32_t i = key_layer + 1; i <= key_layer2; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        
        for (auto it = estMp_key.begin(); it != estMp_key.end(); ++it)
        {
            for (uint32_t i = 0; i <= key_layer; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }
        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch_key;
        delete sketch_key2;
        delete sketch_key3;
        delete sketch_32;
    }

    void HHHKey_USS(uint32_t MEMORY, COUNT_TYPE threshold)
    {
        std::string algo = "Residual_USS";
        emptyfile(algo, flag[10]);
         
        int key_layer, key_layer_prefix;
        if(LAYER == 33){
            key_layer = 12;
            key_layer_prefix = 12;
        }
        else{
            key_layer = 2;
            key_layer_prefix = 2 * BYTE_LENGTH;
        }

        USS<uint32_t> *sketch_key = new USS<uint32_t>(3* MEMORY / 4, key_layer_prefix, threshold );
        USS<uint32_t> *sketch_32 = new USS<uint32_t>(3*MEMORY / 4);

        for (uint32_t i = 0; i < length; ++i)
        {
            sketch_32->Insert(dataset[i]);

            sketch_key->Insert(dataset[i] & MASK[key_layer]);
        }
 
        std::unordered_map<uint32_t, COUNT_TYPE> estMp[33];
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_key;

        estMp[LAYER - 1] = sketch_32->AllQuery();
        estMp_key = sketch_key->AllQuery();

        for (auto it = estMp[LAYER - 1].begin(); it != estMp[LAYER - 1].end(); ++it)
        {
            for (uint32_t i = key_layer+1; i < LAYER - 1; ++i)
                estMp[i][it->first & MASK[i]] += it->second;
        }
        
        for (auto it = estMp_key.begin(); it != estMp_key.end(); ++it)
        {
            for (uint32_t i = 0; i <= key_layer; ++i)
            {
                estMp[i][it->first & MASK[i]] += it->second;
            }
        }

        CompareHHH(estMp, threshold, algo, MEMORY);

        delete sketch_key;
        delete sketch_32;
    }


private:
    TUPLES *oridataset;
    uint32_t *dataset;
    uint64_t length;
    int LAYER;
    std::string outpath; 
    std::unordered_map<uint32_t, COUNT_TYPE> mp[33];
    std::unordered_map<uint32_t, COUNT_TYPE> conditionalHHMp[33];
    int flag[15];
    uint32_t *MASK;
    uint32_t MASK_bit[33] = {0x0, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000, 0xfc000000,
                                   0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 0xfff80000,
                                   0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
                                   0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
                                   0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};
    uint32_t MASK_byte[5] = {0x0, 0xff000000, 0xffff0000, 0xffffff00, 0xffffffff};

    uint32_t bucketbound_bit[33] = {0x2, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
                                    0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
                                    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
                                    0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0xffffffff
                                    };
    uint32_t bucketbound_byte[5] = {0x2, 0x100, 0x10000, 0x1000000, 0xffffffff}; 
    uint32_t *bucketbound;
   
    
    void emptyfile(std::string algoname, int &flag){
        // std::cout << flag << std::endl;
        if(flag == 0){
            std::string filename = outpath + algoname +".txt";
            std::ofstream tout(filename, std::ios::trunc | std::ios::out);
            tout.close();
            flag = 1;
        }
    }

    void CalculateConditionalHHMp(COUNT_TYPE threshold)
    {
        for (int i = LAYER - 1; i >= 0; --i)
            conditionalHHMp[i].clear();        
        
        for (int i = LAYER - 1; i >= 0; --i)
        {
            for (auto it = mp[i].begin(); it != mp[i].end(); ++it)
            {
                if (it->second > threshold)
                {
                    unsigned long long conditional_cnt = it->second;
                    for (uint32_t j = i + 1; j <= LAYER - 1; ++j)
                    {
                        for (auto iter = conditionalHHMp[j].begin(); iter != conditionalHHMp[j].end(); iter++)
                        {
                            if ((iter->first & MASK[i]) == it->first)
                            {
                                conditional_cnt -= iter->second;
                            }
                        }
                    }
                    if (conditional_cnt > threshold){
                        conditionalHHMp[i][it->first] = conditional_cnt;
                    }
                }
            }
        }
    }

    void CalculateContionalEstMp(COUNT_TYPE threshold, std::unordered_map<uint32_t, COUNT_TYPE> estMp[33]){
        std::unordered_map<uint32_t, COUNT_TYPE> estMp_cp[33];
        for (int i = LAYER - 1; i >= 0; --i)
        {
            std::unordered_map<uint32_t, COUNT_TYPE> tmp(estMp[i]);
            estMp[i].clear();
            if(i == LAYER - 1) assert(tmp.size() != 0);
            for (auto it = tmp.begin(); it != tmp.end(); ++it)
            {
                if (it->second > threshold)
                {
                    long long conditional_cnt = it->second;
                    for (uint32_t j = i + 1; j <= LAYER - 1; ++j)
                    {
                        for (auto iter = estMp_cp[j].begin(); iter != estMp_cp[j].end(); iter++)
                        {
                            // std::cout << "in set: " << (iter->first & MASK[i]) << " " << it->first << std::endl; 
                            if ((iter->first & MASK[i]) == it->first)
                            {
                                conditional_cnt -= iter->second;
                            }
                        }
                    }
                    if (conditional_cnt > threshold){
                        estMp_cp[i][it->first] = conditional_cnt;
                        estMp[i][it->first] = it->second;
                        
                    }
                }
            }
        }
    }

    void CompareHHH(std::unordered_map<uint32_t, COUNT_TYPE> estMp[33], COUNT_TYPE threshold, std::string algoname, uint32_t mem)
    {
        CalculateConditionalHHMp(threshold);
        CalculateContionalEstMp(threshold, estMp);
        
        std::ofstream Fout(outpath+algoname+".txt", std::ios::app | std::ios::out);

        double realHH = 0, estHH = 0, bothHH = 0, aae = 0, are = 0, 
            wrealHH = 0, westHH = 0, wbothHH = 0, ware = 0, waae = 0;
        for (uint32_t i = 0; i < LAYER - 1; ++i)    // if the base layer is not count, LAYER - 1
        {
            for (auto it = mp[i].begin(); it != mp[i].end(); ++it)
            {
                double realF = mp[i][it->first], estF = estMp[i][it->first];

                bool est = (estF > threshold);
                bool real = (conditionalHHMp[i].count(it->first) != 0);

                realHH += real;
                estHH += est;

                if (real && est)
                {
                    bothHH += 1;
                    aae += abs(realF - estF);
                    are += abs(realF - estF) / realF;
                }
            }
        }

        std::cout << "algo: " << algoname << std::endl;
        std::cout << "threshold," << threshold << std::endl;
        if(algoname != "Full")
            std::cout << "mem: " << mem << std::endl;
        else
            std::cout << "for FULL's peak use memory, see the std output starting with real memory: %d" << std::endl;
        std::cout << "realHH," << realHH << std::endl;
        std::cout << "estHH," << estHH << std::endl;
        std::cout << "bothHH," << bothHH << std::endl;
        std::cout << "recall," << bothHH / realHH << std::endl;
        std::cout << "precision," << bothHH / estHH << std::endl;
        std::cout << "F1 score," << 2 *(bothHH/realHH * bothHH/estHH)/(bothHH/realHH + bothHH/estHH) << std::endl;
        std::cout << "aae," << aae / bothHH << std::endl;
        std::cout << "are," << are / bothHH << std::endl;
        std::cout << std::endl;

        Fout << "=================== "<< std::endl;
        Fout << "threshold = " << threshold << std::endl;
        if(algoname != "Full")
            Fout << "mem: " << mem << std::endl;
        else
            Fout << "for FULL's peak use memory, see the std output starting with real memory: %d" << std::endl;
        Fout << "realHH: " << realHH << std::endl;
        Fout << "estHH: " << estHH << std::endl;
        Fout << "bothHH: " << bothHH << std::endl;
        Fout << "recall: " << bothHH / realHH << std::endl;
        Fout << "precision: " << bothHH / estHH << std::endl;
        Fout << "F2 score: " << 2 *(bothHH/realHH * bothHH/estHH)/(bothHH/realHH + bothHH/estHH) << std::endl;
        Fout << "aae: " << aae / bothHH << std::endl;
        Fout << "are: " << are / bothHH << std::endl;
        Fout << std::endl;
        Fout.close();
    }

};

#endif
