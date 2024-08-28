#pragma once

namespace score_params
{
enum ParamType
{
    NR_NEURONS = 0,
    DURATIONS,
    MAX_PARAM_TYPE
};

static constexpr unsigned long long kDataLength = 256;

// Comment out when we want to reduce the number of running test
static constexpr unsigned long long kSettings[][MAX_PARAM_TYPE] = {
    {8192, 32768},
    {8192, 65536},
    {4096, 16384},
    {4096, 32768},
    {4096, 65536},
    {2048, 16384},
    {2048, 32768},
    {2048, 65536},
    {1024, 16384},
    {1024, 32768},
    {1024, 65536},
    {512, 16384},
    {512, 32768},
    {512, 65536},
    {16384, 4096},
    {16384, 2048},
    {16384, 1024},
    {16384, 512},
    {32768, 2048},
    {32768, 4096},
    {32768, 1024},
    {32768, 512}
};
}
