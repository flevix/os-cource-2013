#ifndef OPTIONAL_H
#define OPTIONAL_H

#include <utility>
#include "myvector.h"

class optional
{
public:
    optional(std::nullptr_t ptr);
    optional(my_vector &&value);
    bool exist();
    char *get();
    optional &operator=(optional const &v);
private:
    void free_data();
    char *data = nullptr;
    bool flag;
};

#endif
