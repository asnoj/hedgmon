#ifndef MEM_H
#define MEM_H

typedef struct {
    long total_kb;
    long free_kb;
    long available_kb;
    long swap_total_kb;
    long swap_free_kb;
} mem_info_t;

// fill with /proc/meminfo -> return 0 if ok and -1 if any error
int mem_get_info(mem_info_t *info);

// print
void mem_print(const mem_info_t *info);

#endif