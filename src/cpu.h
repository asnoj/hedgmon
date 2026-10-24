#ifndef CPU_H
#define CPU_H

#include "proc.h"

typedef struct {
    unsigned long long total;
    unsigned long long idle;
} cpu_raw_t;

// read /proc/stat 1 time and fill raw with total + idle
// return 0if ok or -1 if error
int cpu_read_raw(cpu_raw_t *raw);

// call 2 time cpu_read_raw with 1 sec in between
double cpu_get_usage(void);

double measure_all_cpu_usage(process_list_t *procs);

#endif