#include "AM_CacheItem.h"

#include <stdlib.h>
#include <string.h>

AM_CacheItem::AM_CacheItem()
{
}

AM_CacheItem::~AM_CacheItem()
{
}

char *AM_CacheItem::getName()
{
    return name;
}

void AM_CacheItem::setName(const char *namex)
{
    strcpy(this->name, namex);
}

void AM_CacheItem::setType(AM_ValueType type)
{
    this->type = type;
}

int AM_CacheItem::getIntValue()
{
    return this->intValue;
}

float AM_CacheItem::getFloatValue()
{
    return this->floatValue;
}

void AM_CacheItem::setValue(int value)
{
    this->intValue = value;
}

void AM_CacheItem::setValue(float value)
{
    this->floatValue = value;
}

void AM_CacheItem::setValue(char *value)
{
    strcpy(this->textValue, value);
}
