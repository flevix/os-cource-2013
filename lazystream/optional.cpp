#include "optional.h"
#include <stdlib.h>

optional::optional(std::nullptr_t ptr)
{
    free_data();
    flag = false;
    data = ptr;
}

optional::optional(my_vector &&value)
{
    free_data();
    data = std::move(value.data());
    flag = (data == nullptr ? false : true);
    value.buffer = nullptr;
}

bool optional::exist()
{
    return flag;
}

char *optional::get()
{
    return data;
}

optional &optional::operator=(optional const &opt)
{
    data = opt.data;
    flag = opt.flag;
    return *this;
}

void optional::free_data()
{
    if (data != nullptr)
    {
        free(data);
    }
}
