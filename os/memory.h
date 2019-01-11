#ifndef KOZOS_MEMORY_H_
#define KOZOS_MEMORY_H_

int kzmem_init(void);
void *kzmem_alloc(int size);
void kzmem_free(void *mem);

#endif