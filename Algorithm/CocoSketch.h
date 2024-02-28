#ifndef COCOSKETCH_H
#define COCOSKETCH_H

#include "MultiAbstract.h"
#include <iostream>
#include <vector>

using std::vector;

void printIp(uint32_t ip)
{
    for (int i = 3; i >= 0; --i)
    {
        std::cout << ((ip & (0xFFu << (i * 8))) >> (i * 8));
        if (i != 0)
            std::cout << ".";
    }
    std::cout << std::endl;
}

template <typename DATA_TYPE>
class CocoSketch : public MultiAbstract<DATA_TYPE>
{
public:
    typedef typename MultiAbstract<DATA_TYPE>::Counter Counter;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    CocoSketch(uint32_t _MEMORY, uint32_t _HASH_NUM = 2, uint32_t _layer_num = 0, COUNT_TYPE _threshold = 0, std::string _name = "CocoSketch")
    {
        this->name = _name;

        HASH_NUM = _HASH_NUM;
        LENGTH = _MEMORY / _HASH_NUM / sizeof(Counter);
        layer_prefix = (~0u) ^ (~0u >> _layer_num);
        // std::cout << "layer_prefix: ";
        // for (int i = 3; i >= 0; --i)
        // {
        //     std::cout << ((layer_prefix & (0xFFu << (i * 8))) >> (i * 8));
        //     if (i != 0)
        //         std::cout << ".";
        // }
        // std::cout << std::endl;
        threshold = _threshold;

        counter = new Counter *[HASH_NUM];
        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            counter[i] = new Counter[LENGTH];
            memset(counter[i], 0, sizeof(Counter) * LENGTH);
        }
    }

    ~CocoSketch()
    {
        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            delete[] counter[i];
        }
        delete[] counter;
    }

    void Insert(const DATA_TYPE &item)
    {
        COUNT_TYPE minimum = 0x7fffffff;
        uint32_t minPos, minHash;

        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            uint32_t position = hash(item, i) % LENGTH;
            if (counter[i][position].ID == item)
            {
                counter[i][position].count += 1;
                return;
            }
            if (counter[i][position].count < minimum)
            {
                minPos = position;
                minHash = i;
                minimum = counter[i][position].count;
            }
        }

        counter[minHash][minPos].count += 1;
        if (randomGenerator() % counter[minHash][minPos].count == 0)
        {
            counter[minHash][minPos].ID = item;
        }
    }

    int Insert_32(const DATA_TYPE &item)
    {
        COUNT_TYPE minimum = 0x7fffffff;
        uint32_t minPos, minHash;

        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            uint32_t position = hash(item, i) % LENGTH;
            if (counter[i][position].ID == item)
            {
                counter[i][position].count += 1;
                return counter[i][position].count;
            }
            if (counter[i][position].count < minimum)
            {
                minPos = position;
                minHash = i;
                minimum = counter[i][position].count;
            }
        }

        counter[minHash][minPos].count += 1;
        if (randomGenerator() % counter[minHash][minPos].count == 0)
        {
            counter[minHash][minPos].ID = item;
        }
        return 0;
    }


    void Insert_key_layer(const DATA_TYPE &item, bool decrement = false)
    {
        COUNT_TYPE minimum = 0x7fffffff;
        uint32_t minPos, minHash;

        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            uint32_t position = hash(item, i) % LENGTH;
            if (counter[i][position].ID == item)
            {
                counter[i][position].count += 1;
                if(decrement) counter[i][position].count = std::max(0, counter[i][position].count - threshold);
                return;
            }
            if (counter[i][position].count < minimum)
            {
                minPos = position;
                minHash = i;
                minimum = counter[i][position].count;
            }
        }

        counter[minHash][minPos].count += 1;
        if (randomGenerator() % counter[minHash][minPos].count == 0)
        {
            counter[minHash][minPos].ID = item;
        }
    }


    HashMap AllQuery()
    {
        HashMap ret;
        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            for (uint32_t j = 0; j < LENGTH; ++j)
            {
                // srcip may be zero incidently

                ret[counter[i][j].ID] += counter[i][j].count;
            }
        }

        return ret;
    }


    HashMap AllQuery_key_layer(const HashMap & Mp_32)
    {
        HashMap ret;
        for(auto it = Mp_32.begin(); it != Mp_32.end(); ++it){
            if(it->second > threshold) ret[it->first & layer_prefix] += it->second;
        }
        for (uint32_t i = 0; i < HASH_NUM; ++i)
        {
            for (uint32_t j = 0; j < LENGTH; ++j)
            {
                // srcip may be zero incidently

                ret[counter[i][j].ID] += counter[i][j].count;
            }
        }

        return ret;
    }



private:
    uint32_t LENGTH;
    uint32_t HASH_NUM;
    uint32_t layer_prefix;
    COUNT_TYPE threshold;

    Counter **counter;
};

#endif
