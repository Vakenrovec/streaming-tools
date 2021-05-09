#pragma once

#include "DataProcessor.h"
#include <array>
#include <cstdint>
#include <memory>

template<std::uint8_t Nm>
class ForkDataProcessor: public DataProcessor
{
public:
    ForkDataProcessor()
    {
    }

    void Init() override
    {
        for (const auto& processor: m_processors)
        {
            if (processor) {
                processor->Init();
            }
        }
        DataProcessor::Init();
    }

    void Destroy() override
    {
        for (const auto& processor: m_processors)
        {
            if (processor) {
                processor->Destroy();
            }
        }
        DataProcessor::Destroy();
    }

    void SetNextProcessors(const std::array<data_processor_ptr, Nm>& processors)
    {
        m_processors = processors;
    }

public:
    std::array<data_processor_ptr, Nm> m_processors;
    int xxx;
};
