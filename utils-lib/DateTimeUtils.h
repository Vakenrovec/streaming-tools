#pragma once

#include <cstdint>

class DateTimeUtils {
public:
    DateTimeUtils() = delete;

    static std::uint64_t GetCurrentTimeMiliseconds();
};