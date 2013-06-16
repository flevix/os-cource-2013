#include <cstdlib>

class my_vector
{
public:
    my_vector(size_t capacity);
    my_vector(const my_vector &vector);
    my_vector(my_vector &&v);
    char &operator[](size_t id);
    ~my_vector();
private:
    size_t capacity;
    size_t size;
    char *data;
};
