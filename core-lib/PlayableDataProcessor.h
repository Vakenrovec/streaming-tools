#pragma once

#include "DataProcessor.h"

class PlayableDataProcessor: public DataProcessor
{
public:
    virtual int Play(int count = 0);
    inline void Stop() { stop = true; };

protected:
    volatile bool stop = false;
};
