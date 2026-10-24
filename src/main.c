#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "cpu.h"
#include "ui.h"
#include "proc.h"
#include "net.h"
#define CLEAR_SCREEN "\033[H\033[J"

typedef enum {
    SORT_RAM,
    SORT_CPU
} sort_mode_t;

static sort_mode_t parse_args(int argc, char *argv[])
{
    sort_mode_t sort_mode = SORT_RAM; // default

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "--sort") == 0 && i + 1 < argc)
        {
            if ((strcmp(argv[i + 1], "cpu")) == 0 || (strcmp(argv[i + 1], "CPU") == 0))
            {
                sort_mode = SORT_CPU;
            } 
            else if (strcmp(argv[i + 1], "ram") == 0)
            {
                sort_mode = SORT_RAM;
            }
        }
    }

    return sort_mode;
}

static void display_loop(sort_mode_t sort_mode)
{
    while (1)
    {
        //    dont clean screan before getting the stats
        //    (cpu_get_usage() will sleep 1 so not cool visualy)
        mem_info_t mem;
        mem_get_info(&mem);

        process_list_t procs;
        proc_list_get(&procs); // get PIDs/names first, before measuring CPU

        double cpu_usage = measure_all_cpu_usage(&procs); // fills cpu_percent for each + returns global

        if (sort_mode == SORT_CPU) 
            proc_list_sort_by_cpu(&procs);
        else
            proc_list_sort_by_rss(&procs);


        connection_list_t conns;
        net_get_tcp_connections(&conns);

        char *asciart =
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠛⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣄⣀⣀⣀⣿⣶⣤⣄⡈⣽⣄⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⢉⣠⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⣼⣅⠘⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠀⠀⠈⠿⠋⠁⠀⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣏⣠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡀⠀⠀⢠⣶⣦⡀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠸⣿⣿⣿⡿⠿⠿⠿⠿⠿⠿⠿⠟⠛⠋⠀⠀⠀⠀⠉⠋⠀⠀⠀⠼⠿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡀⠿⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣼⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣤⣶⣶⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿         ᓬ hedgemon ᕒ         ⣿⣿⣿⣿⣿⣿⣿\n"
"⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n";
    
        printf(CLEAR_SCREEN);

        printf("%s", asciart);

        char line[64];

        snprintf(line, sizeof(line), "CPU:  %5.1f%%", cpu_usage);
        print_bordered(line, WINDOW_WIDTH);

        mem_print(&mem);


        printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
        proc_list_print_top(&procs, 10);
        printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
        net_print(&conns);
        printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
        fflush(stdout);
    }
}

int main(int argc, char *argv[])
{
    sort_mode_t sort_mode = parse_args(argc, argv);

    display_loop(sort_mode);
    return 0;
}








