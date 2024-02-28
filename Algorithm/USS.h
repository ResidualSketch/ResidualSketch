#ifndef USS_H
#define USS_H

#include "MultiAbstract.h"
#include "StreamSummary.h"

template<typename DATA_TYPE>
class USS : public MultiAbstract<DATA_TYPE>{
public:

    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    USS(uint32_t _MEMORY,  int _layer_num = 32, int threshold_ = 0, std::string _name = "USS"){
        this->name = _name;
        layer_prefix = (~0u) ^ (~0u >> _layer_num);
        threshold = threshold_;
        summary = new StreamSummary<DATA_TYPE, COUNT_TYPE>(summary->Memory2Size(_MEMORY));
    }

    ~USS(){
        delete summary;
    }

    void Insert(const DATA_TYPE& item){
        if(summary->mp->Lookup(item))
            summary->Add_Data(item);
        else{
            if(summary->isFull()){
                if(randomGenerator() % (summary->getMin() + 1) == 0)
                    summary->SS_Replace(item);
                else
                    summary->Add_Min();
            }
            else
                summary->New_Data(item);
        }
    }

    DATA_TYPE Insert_32(const DATA_TYPE& item){
        if(summary->mp->Lookup(item)){
            summary->Add_Data(item);
            return summary->Query(item);
        }
        else{
            if(summary->isFull()){
                if(randomGenerator() % (summary->getMin() + 1) == 0)
                    summary->SS_Replace(item);
                else
                    summary->Add_Min();
                return 0;
            }
            else{
                summary->New_Data(item);
                return 1;
            }
        }
    }

    void Insert_key_layer(const DATA_TYPE& item,bool decrement = false){
        if(summary->mp->Lookup(item)){
            if(decrement)
                summary->Del_Data(item, threshold);
            else
                summary->Add_Data(item);
        }
        else{
            if(summary->isFull()){
                if(randomGenerator() % (summary->getMin() + 1) == 0)
                    summary->SS_Replace(item);
                else
                    summary->Add_Min();
            }
            else{
                summary->New_Data(item);
            }
        }
    }

    HashMap AllQuery(){
        return summary->AllQuery();
    }

    HashMap AllQuery_key_layer(HashMap& ret_32){
        HashMap ret = summary->AllQuery();
        for(auto it = ret_32.begin(); it != ret_32.end(); ++it){
            if(it->second > threshold) 
                ret[it->first & layer_prefix] += it->second;
        }
        return ret;
    }

private:
    StreamSummary<DATA_TYPE, COUNT_TYPE>* summary;
    int threshold;
    uint32_t layer_prefix;
};

#endif
