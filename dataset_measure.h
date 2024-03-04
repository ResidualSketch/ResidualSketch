#ifndef HHHMEASURE_H
#define HHHMEASURE_H

#include <vector>
#include <fstream>
#include <iostream>

#include "assert.h"

#include <iomanip>

#include "MMap.h"

class HHHMeasure{
public:
    TUPLES *oridataset;
    uint32_t *dataset; 
    uint64_t length;
    int LAYER;
    std::string outpath; 
    std::unordered_map<uint32_t, COUNT_TYPE> mp[33];
    std::unordered_map<uint32_t, COUNT_TYPE> conditionalHHMp[33];
    uint32_t *MASK;
    uint32_t MASK_bit[33] = {0x0, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000, 0xfc000000,
                                   0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 0xfff80000,
                                   0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
                                   0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
                                   0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};
    uint32_t MASK_byte[5] = {0x0, 0xff000000, 0xffff0000, 0xffffff00, 0xffffffff};

        
    HHHMeasure(std::string PATH, std::string name, int layer, int SrcOrDst){
        LAYER = layer;
        std::string typee, Src;
        if(layer == 33){
            MASK = MASK_bit;
            typee = "1D_bit/";
        }    
        else if(layer == 5){
            MASK = MASK_byte;    
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
        std::cout << "dataset: " << name <<", " << Src <<  " length: " << length << std::endl;

        outpath = "./output/datasets/" + typee + name + "/"+ Src +"data.txt";
        std::ofstream Fout(outpath, std::ios::trunc | std::ios::out);
        Fout.close();

        for (uint64_t i = 0; i < length; ++i)
        {
            mp[LAYER - 1][dataset[i]] += 1;
        }

        for (auto it = mp[LAYER - 1].begin(); it != mp[LAYER - 1].end(); ++it)
            for (uint32_t i = 0; i < LAYER - 1; ++i)
                mp[i][it->first & MASK[i]] += it->second;
    }

    void CalculateConditionalHHMp(COUNT_TYPE threshold)
    {
        for (int i = LAYER - 1; i >= 0; --i)
            conditionalHHMp[i].clear();
        
        for (int i = LAYER - 1; i >= 0; --i){
            for (auto it = mp[i].begin(); it != mp[i].end(); ++it){
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

    void OutputHHH(COUNT_TYPE threshold)
    {
        CalculateConditionalHHMp(threshold);
        std::ofstream Fout(outpath, std::ios::app | std::ios::out);
        Fout << "================" <<std::endl;
        Fout << "threshold = "<< threshold <<std::endl;

        uint32_t realHH = 0, realHH_previous = 0;
        for (uint32_t i = 0; i < LAYER; ++i)
        {
            for (auto it = mp[i].begin(); it != mp[i].end(); ++it)
            {
                bool real = (conditionalHHMp[i].count(it->first) != 0);
                realHH += real;
            }
            // print the aae, HH number of each layer
            Fout <<"i: "<< i <<", HHH: " << realHH - realHH_previous << std::endl;
            realHH_previous = realHH;
        }
        Fout.close();
    }

    ~HHHMeasure()
    {
        delete oridataset;
        delete dataset;
    }
};

#endif
