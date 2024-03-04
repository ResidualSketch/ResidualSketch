// used for finding the corresponding IPs of  IP prefix HHs
#include <x86intrin.h>

#include <vector>
#include <chrono>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <string.h>
#include <unordered_map>
#include "Common/Util.h"

using namespace std;

#pragma pack(1)

const uint32_t MASK[33] = {0x0, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000, 0xfc000000,
                           0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 0xfff80000,
                           0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
                           0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
                           0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};

struct HH_node
{
    uint32_t IP_prefix;
    int val;
    vector<struct HH_node*> next;

    HH_node()
    {
        IP_prefix = 0;
        val = 0;
        next.clear();
    }
    HH_node(uint32_t IP_prefix_, int val_ = 0) : IP_prefix(IP_prefix_), val(val_)
    {
        next.clear();
    }
};

bool cmp_HH_node(HH_node* a, HH_node* b){
    return a->IP_prefix < b->IP_prefix;
}


string ip_to_string(unsigned int ip_prefix){
    return to_string((ip_prefix >> 24) & 0xFF ) + "." +
            to_string((ip_prefix >> 16) & 0xFF ) + "." + 
            to_string((ip_prefix >> 8) & 0xFF ) + "." + 
            to_string((ip_prefix ) & 0xFF ); 
             
}



void dataset_analysis(std::string PATH, std::string name, uint32_t threshold){

    uint64_t cnt = 0;

    TUPLES *read_data_tuples = read_data(PATH.c_str(), 100000000, &cnt);

    unordered_map<uint32_t, HH_node *> Mp[33];
    unordered_map<uint32_t, int> conditional_HHs[33];


    for (uint32_t i = 0; i < cnt; ++i)
    {
        uint32_t srcIP_ = read_data_tuples[i].srcIP();
        if(srcIP_ == 0) continue;
        if (Mp[32].count(srcIP_) == 0)
            Mp[32][srcIP_] = new HH_node(srcIP_);
        Mp[32][srcIP_]->val += 1;
    }
    std::cout << "dataset: " << name  <<", " <<  "packet size: " << cnt << " " 
    <<  "flow size: " <<  Mp[32].size()<< std::endl;
    // aggregation at higher layer
    for (int i = 31; i >= 0; --i)
    {
        for (auto it = Mp[32].begin(); it != Mp[32].end(); it++)
        {
            if (Mp[i].count(it->first & MASK[i]) == 0)
            {
                Mp[i][it->first & MASK[i]] = new HH_node(it->first & MASK[i]);
            }

            HH_node *i_new_node = new HH_node(it->first, (it->second)->val);
            Mp[i][it->first & MASK[i]]->next.push_back(i_new_node);
            Mp[i][it->first & MASK[i]]->val += (it->second)->val;
        }
    }

    // conditional counting and find HHs
    for (int i = 32; i >= 0; --i)
    {
        cout << "layer " << i << " conditional counting..." << endl;
        for (auto it = Mp[i].begin(); it != Mp[i].end(); ++it)
        {
            sort(((it->second)->next).begin(),((it->second)->next).end(),cmp_HH_node);
            if ((it->second)->val > threshold)
            {
                // if one item's value exceeds threshold, consider if it still exceeds threshold after substracting descendants of HHs.
                // if consider all descendants of it, only subtracting conditional counts. if consider direct descendant HHs of it, subtracting
                // original counts. Here, we choose the former.
                unsigned long long conditional_cnt = (it->second)->val;
                for (uint32_t j = i + 1; j <= 32; ++j)
                {
                    for (auto iter = conditional_HHs[j].begin(); iter != conditional_HHs[j].end(); iter++)
                    {
                        // std::cout << "in set: " << (iter->first & MASK[i]) << " " << it->first << std::endl;
                        if ((iter->first & MASK[i]) == it->first)
                        {
                            conditional_cnt -= iter->second;
                            if(j != 32){
                            for(auto iter_node = (Mp[j][iter->first]->next).begin(); iter_node != (Mp[j][iter->first]->next).end(); ++iter_node){
                                int k = 0;
                                int size_ = (it->second)->next.size();
                                if((*iter_node)->IP_prefix==0)continue;
                                while(k < size_ && ((it->second)->next[k])->IP_prefix != (*iter_node)->IP_prefix){
                                    k++;
                                }
                                if(k == size_) {std::cerr << "program crashed." << std::endl; exit(1);}
                                ((it->second)->next[k])->IP_prefix = 0;
                            }
                            }else{
                                int k = 0, size_ = (it->second)->next.size();
                                while(k < size_ && ((it->second)->next[k])->IP_prefix != Mp[32][iter->first]->IP_prefix) k ++;
                                if(k == size_){std::cerr << "program crashed." << std::endl; exit(1);}
                                ((it->second)->next[k])->IP_prefix = 0;
                            }
                        }
                    }
                }
                if (conditional_cnt > threshold)
                {
                    conditional_HHs[i][it->first] = conditional_cnt;
                    // std::cout << it->first << " " << conditional_cnt << std::endl;
                }
            }
        }
        // std::cout << "i: " << i << " size: " << conditionalHHMp[i].size() << std::endl;
    }



    std::string outpath = "./output/HHH/layer_full_keys_dist"+ name + "_" + std::to_string(threshold) + ".txt";
    ofstream output_data(outpath);

    // layer 32
    output_data << " HH in layer 32: " << conditional_HHs[32].size() << endl;

    for(auto it = conditional_HHs[32].begin(); it != conditional_HHs[32].end(); it++){
        //  output_data << ip_to_string(it->first) << " "<< it->second << endl;
        output_data << 1 << ", ";

    } 
    output_data << endl;

    // HH and its corresponding IPs in 32 layer
    for (int i = 31; i >= 0; i--){
        output_data << "HH in layer " << i << ": "<< conditional_HHs[i].size() << endl;
        /// output_data << endl;
        for(auto it = conditional_HHs[i].begin(); it != conditional_HHs[i].end(); it++){
            /// output_data << ip_to_string(it->first) << "/" << i << endl;
            int HH_card = 0;
            for(auto iter = (Mp[i][it->first]->next).begin();iter != (Mp[i][it->first]->next).end();iter++){
                if((*iter)->IP_prefix != 0){
                    HH_card++;
                    /// output_data << ip_to_string((*iter)->IP_prefix) << " " << (*iter)->val << "  " << estMp[(*iter)->IP_prefix]<< endl;
                }
            }
            output_data << HH_card << ", ";
        }
        output_data << endl;
    }

    
    output_data.close();
}
