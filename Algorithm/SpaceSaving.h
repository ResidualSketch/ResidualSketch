#ifndef SpaceSaving_H
#define SpaceSaving_H

#include "MultiAbstract.h"
#include "StreamSummary.h"

template<typename DATA_TYPE>
class SpaceSaving : public MultiAbstract<DATA_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SpaceSaving(uint32_t _MEMORY, std::string _name = "SpaceSaving"){
        this->name = _name;


        summary = new StreamSummary<DATA_TYPE, COUNT_TYPE>(summary->Memory2Size(_MEMORY));
    }

    ~SpaceSaving(){
        delete summary;
    }

    void Insert(const DATA_TYPE& item){
        if(summary->mp->Lookup(item))
            summary->Add_Data(item);
        else{
            if(summary->isFull())
                summary->SS_Replace(item);
            else
                summary->New_Data(item);
        }
        // std::cout << "size after insert: "  <<  (summary->mp)->size() << std::endl;
        // res = summary->AllQuery();
        // for(auto it = res.begin();it!=res.end(); it++)
        //     std::cout << "inserted item: " << it->first << std::endl;
        // std::cout << std::endl;
    }


    HashMap AllQuery(){
        return summary->AllQuery();
    }

private:
    StreamSummary<DATA_TYPE, COUNT_TYPE>* summary;
};

#endif
