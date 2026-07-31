#ifndef AM_CACHEITEM_H
#define AM_CACHEITEM_H

#include <stdlib.h>
#include <string.h>

#include "AM_SDK_PicoWiFi.h"

enum AM_ValueType
{
    AM_INT,
    AM_FLOAT,
    AM_TEXT
};

class AM_CacheItem
{

private:
    char name[VARIABLELEN];
    AM_ValueType type;

    union
    {
        int intValue;
        float floatValue;
        char textValue[32];
    };

public:
    AM_CacheItem();
    ~AM_CacheItem();

    char *getName();
    void setName(const char *name);

    void setType(AM_ValueType type);

    int getIntValue();
    float getFloatValue();
    void setValue(int value);
    void setValue(float value);
    void setValue(char *value);
};

#endif