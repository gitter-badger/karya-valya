#pragma once

#include "MainObject.h"

class UnsyncGenerator: public IMainObject
{
public:
    DECLARE_SAVED(UnsyncGenerator, IMainObject);
    DECLARE_GET_TYPE_ITEM(UnsyncGenerator)
    UnsyncGenerator(size_t id);
    void PerformUnsync();
private:
    std::string KV_SAVEBLE(unsync_string_);

    void SetThisAsUnsyncGenerator();
    KV_ON_LOAD_CALL(SetThisAsUnsyncGenerator);
};
ADD_TO_TYPELIST(UnsyncGenerator);

void SetUnsyncGenerator(id_ptr_on<UnsyncGenerator> generator);
id_ptr_on<UnsyncGenerator> GetUnsyncGenerator();

