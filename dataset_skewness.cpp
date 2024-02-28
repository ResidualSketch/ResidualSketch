#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <bits/stdc++.h>
using namespace std;

string path = "./datasets/caida.dat";
string dataset_name = "caida";

#define TOPK 1000
#define SRCorDST
#define THRESHOLD 8000
#define THRESHOLD2 10000
#define THRESHOLD_UPPER 8


#define TUPLES_LEN 13
class TUPLES
{
public:
    uint8_t data[TUPLES_LEN];
    uint64_t timestamp;

    TUPLES(){ memset(data, 0, TUPLES_LEN); }
    TUPLES(const char* t){
        for (int i = 0; i < TUPLES_LEN; i++)
            data[i] = t[i];
    }
    TUPLES(const TUPLES& t){
        for (int i = 0; i < TUPLES_LEN; i++)
            data[i] = t.data[i];
    }
    uint32_t SrcIP() const{
        return *((uint32_t*)data);
    }
    uint32_t* SrcIP_inf(){
        return ((uint32_t*)data);
    }
    uint32_t DstIP() const{
        return *((uint32_t*)(data + 4));
    }
    uint32_t* DstIP_inf(){
        return ((uint32_t*)(data + 4));
    }
    uint16_t SrcPort() const{
        return *((uint16_t*)(data + 8));
    }
    uint16_t DstPort() const{
        return *((uint16_t*)(data + 10));
    }
    uint8_t Protocal() const{
        return data[TUPLES_LEN-1];
    }
};

class D1Pair{
public:
    uint32_t IP;
    uint32_t count;
    D1Pair(){};
    D1Pair(uint32_t _IP, uint32_t cnt):IP(_IP),count(cnt){};
    bool operator<(const D1Pair &t) const {
        if(count != t.count)
            return count < t.count;
        return 0;
    }
};
bool operator == (const D1Pair& a, const D1Pair& b){
    return (a.IP == b.IP) && (a.count == b.count);
}

vector<TUPLES> datastream;
map<uint32_t, uint32_t> mp;
vector<D1Pair> flow;
set<uint32_t> majorflow;

void topk_ratio(vector<TUPLES> &datastream){
    map<uint32_t, uint32_t> mp;
    vector<D1Pair> flow;
    set<uint32_t> majorflow;
    for(auto& tuples : datastream){
#ifdef SRCorDST
        uint32_t IP = tuples.SrcIP();
#else 
        uint32_t IP = tuples.DstIP();
#endif
        mp[IP] += 1;
    }

    for(auto [IP, count] : mp){
        flow.push_back(D1Pair(IP, count));
    }
    
    sort(flow.rbegin(),flow.rend(),[&](const D1Pair&a,const D1Pair&b)->bool{
    return (a.count < b.count);
    });

    uint32_t major_pkts = 0;
    for(int i=0; i<TOPK; ++i){
        majorflow.insert(flow[i].IP);
        major_pkts += flow[i].count;
    }

    cout<<"Have " << datastream.size() <<" packets\n";
    cout<<"Major(top " <<TOPK<< ") flows have " <<major_pkts<< " packets("
        << (double)major_pkts/datastream.size() <<")\n";
}

int main()
{
    FILE *data = fopen(path.c_str(), "rb");
    TUPLES item;
    char tmp[25];
    while (fread(tmp, 21, 1, data) > 0) {
        for (int i = 0; i < TUPLES_LEN; i++)
            item.data[i] = tmp[i];
        item.timestamp = *(uint64_t*)(tmp+13);
        datastream.push_back(item);
#ifdef SRCorDST
        mp[item.SrcIP()] += 1;
#else
        mp[item.DstIP()] += 1;
#endif
	}

    fclose(data);
    cout<<"Read " << datastream.size() <<" packets\n";
    cout<<"Read " << mp.size() <<" flows(by 1D-IP)\n";

    for(auto [IP, count] : mp){
        flow.push_back(D1Pair(IP, count));
    }
    
    sort(flow.rbegin(),flow.rend(),[&](const D1Pair&a,const D1Pair&b)->bool{
    return (a.count < b.count);
    });
    
    uint32_t major_pkts = 0;
    for(int i=0; i< TOPK; ++i){
        majorflow.insert(flow[i].IP);
        major_pkts += flow[i].count;
    }

    cout<<"Major(top " <<TOPK<< ") flows have " <<major_pkts<< " packets("
        << (double)major_pkts/datastream.size() <<")\n";
//  ...90.dat is a temp file
    for(int fraction = 90, j = 0; fraction <= 100, j < 1; fraction +=10, j++){
        string name = "00.dat"; name[0] += fraction/10;
#ifdef SRCorDST
        name = "src_" + name;
#else
        name = "dst_" + name;
#endif
        FILE *output = fopen(("./datasets/skewness/" + dataset_name+"_1D_"+name).c_str(), "wb");
        
        double frac = (double)fraction/100.0;
        // double thres = (major_pkts - frac*datastream.size())/(double)major_pkts;
        double thres = 0.693 ; // for thres settings, see the below comments.
        cout <<"thres = "<<thres << endl;
        srand(1898);
        mt19937 gen(rand());
        uniform_real_distribution<double> random_real;
        uniform_int_distribution<uint32_t> random_int(0, 4294967294);

        poisson_distribution<> random_int_cnt(THRESHOLD_UPPER);

        uint32_t prefix = (uint32_t)(random_real(gen)/0.20) + 20;
        
        uint32_t key_prefix_generate = (random_int(gen) % (1<<prefix)) << (32 - prefix);
        
        uint32_t key_cnt = (THRESHOLD + 800 * random_int_cnt(gen)) + random_int(gen) % 200;
        while(key_cnt <= THRESHOLD2) {
            key_cnt = (THRESHOLD + 800 * random_int_cnt(gen)) + random_int(gen) % 200;
        }
        for(auto& tuples : datastream){
#ifdef SRCorDST
            uint32_t IP = tuples.SrcIP();
#else 
            uint32_t IP = tuples.DstIP();
#endif      
            if((majorflow.find(IP) != majorflow.end()) &&
                (random_real(gen) <= thres)){
                IP = key_prefix_generate + (random_int(gen) % (1 << (32 - prefix)));
                key_cnt --;
#ifdef SRCorDST
                *(tuples.SrcIP_inf()) = IP; 
#else
                *(tuples.DstIP_inf()) = IP; 
#endif
                if(key_cnt == 0){
                    prefix = (uint32_t)(random_real(gen)/0.2) + 20;
                    key_prefix_generate = (random_int(gen) % (1<<prefix)) << (32 - prefix);
                    key_cnt = (THRESHOLD + 800 * random_int_cnt(gen)) + random_int(gen) % 200;
                    while(key_cnt <= THRESHOLD2) {
                        key_cnt = (THRESHOLD + 800 * random_int_cnt(gen)) + random_int(gen) % 200;
                    }
                }
            }
            for (int i = 0; i < TUPLES_LEN; i++)
                fwrite(&(tuples.data[i]), 1, 1, output);
            fwrite(&(tuples.timestamp), 8, 1, output);
        }
        topk_ratio(datastream);
        fclose(output);
    }

    return 0;
}

/*
random threshold variable, thres, can be set with the following value to satisfy the top-1000 ratio's requirements.

caida_src

original：
Major(top 1000) flows have 30318956 packets(0.564366)

top1000 ratio；corresponding thres
10 1 （topk, 1290）
20 0.693
30 0.462
40 0.256
50 0.055





mawi_src
Major(top 1000) flows have 44558070 packets(0.827509)
10 0.911155
20 0.777311
30 0.648466
40 0.523621
50 0.399777
60 0.276932
70 0.154088
80 0.033243

*/
