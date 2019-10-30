#ifndef POSET_POSET_H
#define POSET_POSET_H

#include <stdbool.h>
#include <stddef.h>

#ifdef  NDEBUG
static int const debug = false;
#else
static  int const debug = true;
#endif

#ifdef __cplusplus
#include <cstddef>

namespace jnp1 {
    extern "C" {
#endif

    unsigned long poset_new();

    void poset_delete(unsigned long id);

    size_t poset_size(unsigned long id);

    bool poset_insert(unsigned long id, char const* value);

    bool poset_remove(unsigned long id, char const* value);

    bool poset_add(unsigned long id, char const* value1, char const* value2);

    bool poset_del(unsigned long id, char const* value1, char const* value2);

    bool poset_test(unsigned long id, char const* value1, char const* value2);

    void poset_clear(unsigned long id);

#ifdef __cplusplus
    }
}





#endif  //extern "C"
#endif //POSET_POSET_H
