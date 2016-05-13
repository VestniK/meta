#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>

typedef _Atomic(uint32_t) atomic_counter32;

struct ArrayControlBlock {
    volatile atomic_counter32 refcnt;
    uint32_t max_elements;
};
_Static_assert(
    sizeof(struct ArrayControlBlock) == 8,
    "ArrayControlBlock must be 64bit long to preserve array data aaligmnet"
);

struct Array {
    struct ArrayControlBlock* cb;
    void* data;
    uint32_t count;
};

bool __meta_rt_array_malloc(uint32_t elem_sz, uint32_t elems_max, struct Array* dest) {
    const size_t cbsz = sizeof(*(dest->cb));
    // Requested block of memory too large (elem_sz*elems_cnt + cntsz > SIZE_MAX)
    if ((SIZE_MAX - cbsz)/elems_max < elem_sz)
        return false;

    const size_t sz = elem_sz*elems_max + cbsz;
    void* block = malloc(sz);
    if (!block)
        return false;

    dest->cb = (struct ArrayControlBlock*)block;
    atomic_init(&dest->cb->refcnt, 1);
    dest->data = (char*)(block) + cbsz;
    dest->cb->max_elements = elems_max;
    return true;
}

void __meta_rt_array_release(struct Array* dest) {
    if (!dest->cb)
        return;
    uint32_t count = atomic_fetch_sub_explicit(&dest->cb->refcnt, 1, memory_order_acq_rel);
    if (count == 1)
        return;
    free(dest->cb);
}

void __meta_rt_array_attach(const struct Array* target, struct Array* dest) {
    dest->cb = target->cb;
    dest->data = target->data;
    dest->count = target->count;
    if (!dest->cb)
        return;
    atomic_fetch_and_explicit(&dest->cb->refcnt, 1, memory_order_relaxed);
}

uint32_t __meta_rt_array_usecount(struct Array* dest) {
    if (!dest->cb)
        return 0;
    return atomic_load_explicit(&dest->cb->refcnt, memory_order_relaxed);
}
