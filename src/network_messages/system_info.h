#pragma once

#include "common_def.h"

#define REQUEST_SYSTEM_INFO 46

static_assert(sizeof(RequestedEntity) == 32, "Something is wrong with the struct size.");


#define RESPOND_SYSTEM_INFO 47

struct RespondSystemInfo
{
    short version;
    unsigned short epoch;
    unsigned int tick;
    unsigned int initialTick;
    unsigned int latestCreatedTick;

    unsigned short initialMillisecond;
    unsigned char initialSecond;
    unsigned char initialMinute;
    unsigned char initialHour;
    unsigned char initialDay;
    unsigned char initialMonth;
    unsigned char initialYear;

    unsigned int numberOfEntities;
    unsigned int numberOfTransactions;

    m256i randomMiningSeed;
};

static_assert(sizeof(RespondSystemInfo) == 2 + 2 + 4 + 4 + 4 + 2 + 1 + 1 + 1 + 1 + 1 + 1 + 4 + 32 + 4, "Something is wrong with the struct size of RespondSystemInfo.");
