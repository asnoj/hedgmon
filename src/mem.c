#include <stdio.h>
#include <string.h>
#include "mem.h"

int mem_get_info(mem_info_t *info) 
{
    FILE *fd = fopen("/proc/meminfo", "r");
    if (fd == NULL) {
        perror("fopen /proc/meminfo");
        return -1;
    }

    memset(info, 0, sizeof(mem_info_t));

    char line[2048];
    while (fgets(line, sizeof(line), fd) != NULL) 
    {
        if (sscanf(line, "MemTotal: %ld kB", &info->total_kb) == 1)
            continue;
        if (sscanf(line, "MemFree: %ld kB", &info->free_kb) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %ld kB", &info->available_kb) == 1)
            continue;
        if (sscanf(line, "SwapTotal: %ld kB", &info->swap_total_kb) == 1)
            continue;
        if (sscanf(line, "SwapFree: %ld kB", &info->swap_free_kb) == 1)
            continue;
    }

    fclose(fd);
    return 0;
}

#include "ui.h"

void mem_print(const mem_info_t *info) {
    char line[64];
    long used_kb = info->total_kb - info->available_kb;
    double used_pct = (info->total_kb > 0) ? (100.0 * used_kb / info->total_kb) : 0.0;

    snprintf(line, sizeof(line), "Mem:  %6.1f / %6.1f Mo  (%.1f%%)",
             used_kb / 1024.0, info->total_kb / 1024.0, used_pct);
    print_bordered(line, WINDOW_WIDTH);

    long swap_used_kb = info->swap_total_kb - info->swap_free_kb;
    double swap_pct = (info->swap_total_kb > 0) ? (100.0 * swap_used_kb / info->swap_total_kb) : 0.0;

    snprintf(line, sizeof(line), "Swap: %6.1f / %6.1f Mo  (%.1f%%)",
             swap_used_kb / 1024.0, info->swap_total_kb / 1024.0, swap_pct);
    print_bordered(line, WINDOW_WIDTH);
}
