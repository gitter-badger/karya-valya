#pragma once

#include <string.h>

#include <map>

#include "Idptr.h"

inline unsigned int hash(const char* key)
{
    unsigned int len = strlen(key);
    unsigned int hash, i;
    for(hash = i = 0; i < len; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

inline unsigned int hash(const std::string& str)
{
    return hash(str.c_str());
}

template<class T>
unsigned int hash(const std::map<unsigned int, id_ptr_on<T>>& map)
{
    unsigned int retval = 0;
    for(auto it = map.cbegin(); it != map.cend(); ++it)
         retval += it->first;
    return retval;
};

template<class T>
unsigned int hash(const id_ptr_on<T>& h)
{
    return h.ret_id();
}

template<class T>
unsigned int hash(std::vector<id_ptr_on<T>>& content)
{
    unsigned int retval = 0;
    int i = 1;
    for (auto it = content.begin(); it != content.end(); ++it, ++i)
        retval += it->ret_id() * i;
    return retval;
}

inline unsigned int hash(unsigned int h)
{
    return h;
}