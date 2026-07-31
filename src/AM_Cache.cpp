#include "AM_Cache.h"

AM_Cache::AM_Cache(/* args */)
{
    this->last_used = 0;
}

AM_Cache::~AM_Cache()
{
}

int AM_Cache::find(const char *name)
{
    for (int i = 0; i < MAX_VARIABLES; i++)
    {
        if (cache[i].getName() != nullptr && strcmp(cache[i].getName(), name) == 0)
        {
            return i;
        }
    }

    return -1;
}

void AM_Cache::add(const char *name, int value)
{
    AM_CacheItem item;

    item.setName(name);
    item.setType(AM_INT);
    item.setValue(value);

    cache[this->last_used++] = item;
}

bool AM_Cache::value_updated(const char *name, int value)
{
    int variable_idx;

    variable_idx = find(name);

    if (variable_idx == -1)
    {
        printf("\t\tVariable %s not found\n", name);
        add(name, value);
        return true;
    }

    int current_value = cache[variable_idx].getIntValue();

    if (current_value == value)
    {
        // printf("\t\tVariable %s found with value %d [NO UPDATE]\n", name, current_value);
        return false;
    }

    printf("\t\tVariable %s found with value %.5f [UPDATE]\n", name, current_value);
    cache[variable_idx].setValue(value);

    return true;
}

bool AM_Cache::value_updated(const char *name, float value)
{
    int variable_idx;

    variable_idx = find(name);

    if (variable_idx == -1)
    {
        printf("\t\tVariable %s not found\n", name);
        add(name, value);
        return true;
    }

    float current_value = cache[variable_idx].getFloatValue();

    if (current_value == value)
    {
        // printf("\t\tVariable %s found with value %d [NO UPDATE]\n", name, current_value);
        return false;
    }

    printf("\t\tVariable %s found with value %.5f [UPDATE]\n", name, current_value);
    cache[variable_idx].setValue(value);

    return true;
}

void AM_Cache::clear(void)
{
    this->last_used = 0;
    for (int i = 0; i < MAX_VARIABLES; i++)
    {
        cache[i].setName(nullptr);
    }
}