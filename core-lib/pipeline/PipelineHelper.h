#pragma once

#include "../credentials-lib/MediaCredentials.h"
#include "file/FileReadRawStreamProcessor.h"
#include <memory>
#include <string>
#include <vector>

using Pipeline = std::shared_ptr<std::vector<std::shared_ptr<DataProcessor>>>;

class PipelineHelper
{
public:
    PipelineHelper() = delete;

    static Pipeline CreateMediaplayer(
        const std::string& rawStreamDir = MediaCredentials::rawStreamDir, 
        const std::string& rawStreamFilename = MediaCredentials::rawStreamFilename,
        const int width = MediaCredentials::width,
        const int height = MediaCredentials::height
    );
};
