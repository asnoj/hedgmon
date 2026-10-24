#include <stdio.h>
#include <unistd.h>
#include "cpu.h"

int cpu_read_raw(cpu_raw_t *raw)
{
    FILE *fd = fopen("/proc/stat", "r");
    if (fd == NULL) {
        perror("fopen /proc/stat");
        return -1;
    }

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    // cpu  123456 789 234567 9876543 12345 0 6789 0 0 0
    int n = fscanf(fd, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
                    &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    fclose(fd);

    if (n != 8) {
        fprintf(stderr, "cpu_read_raw: wrong format in /proc/stat :C\n");
        return -1;
    }

    raw->total = user + nice + system + idle + iowait + irq + softirq + steal;
    raw->idle  = idle + iowait;

    return 0;
}

double cpu_get_usage(void)
{
    cpu_raw_t first, second;

    if (cpu_read_raw(&first) != 0)
        return 0.0;

    sleep(1);

    if (cpu_read_raw(&second) != 0)
        return 0.0;

    unsigned long long delta_total = second.total - first.total;
    unsigned long long delta_idle  = second.idle - first.idle;

    if (delta_total == 0)
        return 0.0;

    double usage_pct = 100.0 * (double)(delta_total - delta_idle) / (double)delta_total;
    return usage_pct;
}