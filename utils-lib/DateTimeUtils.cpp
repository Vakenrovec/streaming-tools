#include "DateTimeUtils.h"
#include <chrono>

std::uint64_t DateTimeUtils::GetCurrentTimeMiliseconds() {
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );

    return static_cast<std::uint64_t>(ms.count());
}