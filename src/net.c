#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net.h"
#include "ui.h"

static const char *state_to_string(int state_code)
{
    switch (state_code)
    {
        case 0x01: return "ESTABLISHED";
        case 0x02: return "SYN_SENT";
        case 0x03: return "SYN_RECV";
        case 0x04: return "FIN_WAIT1";
        case 0x05: return "FIN_WAIT2";
        case 0x06: return "TIME_WAIT";
        case 0x07: return "CLOSE";
        case 0x08: return "CLOSE_WAIT";
        case 0x09: return "LAST_ACK";
        case 0x0A: return "LISTEN";
        case 0x0B: return "CLOSING";
        default:   return "UNKNOWN";
    }
}

static void hex_to_ip(const char *hex_ip, char *out, size_t out_size)
{
    unsigned int ip_hex;
    sscanf(hex_ip, "%X", &ip_hex);

    unsigned int b1 = (ip_hex >> 0) & 0xFF;
    unsigned int b2 = (ip_hex >> 8) & 0xFF;
    unsigned int b3 = (ip_hex >> 16) & 0xFF;
    unsigned int b4 = (ip_hex >> 24) & 0xFF;

    snprintf(out, out_size, "%u.%u.%u.%u", b1, b2, b3, b4);
}

static void hex_to_addr(const char *hex_addr_port, char *out, size_t out_size)
{
    char ip_part[16];
    unsigned int port_hex;

    sscanf(hex_addr_port, "%15[^:]:%X", ip_part, &port_hex);

    char ip_str[IP_STR_LEN];
    hex_to_ip(ip_part, ip_str, sizeof(ip_str));

    snprintf(out, out_size, "%s:%u", ip_str, port_hex);
}

int net_get_tcp_connections(connection_list_t *conns)
{
    conns->count = 0;

    FILE *fd = fopen("/proc/net/tcp", "r");
    if (fd == NULL)
    {
        perror("fopen /proc/net/tcp");
        return -1;
    }

    char line[256];
    fgets(line, sizeof(line), fd); // skip header line

    while (fgets(line, sizeof(line), fd) != NULL)
    {
        if (conns->count >= MAX_CONNECTIONS)
        {
            break;
        }

        char local_hex[32];
        char remote_hex[32];
        unsigned int state_code;

        int n = sscanf(line, "%*d: %31s %31s %X",
                        local_hex, remote_hex, &state_code);

        if (n != 3)
        {
            continue;
        }

        connection_t *conn = &conns->list[conns->count];

        char local_ip_hex[16];
        sscanf(local_hex, "%15[^:]", local_ip_hex);
        hex_to_ip(local_ip_hex, conn->local_ip, sizeof(conn->local_ip));

        hex_to_addr(remote_hex, conn->remote_addr, sizeof(conn->remote_addr));
        snprintf(conn->state, sizeof(conn->state), "%s", state_to_string((int)state_code));

        conns->count++;
    }

    fclose(fd);
    return 0;
}

static void print_section_title(const char *title, int width)
{
    int title_len = (int)strlen(title);
    int fill_len = width - title_len;

    if (fill_len < 0)
    {
        fill_len = 0;
    }

    char line[128];
    snprintf(line, sizeof(line), "%s%.*s", title, fill_len, 
             "========================================================================");

    print_bordered(line, width);
}

// Prints every connection whose state matches the given state string.
// Returns the number of connections printed (0 means the section is skipped).
static int print_section(const connection_list_t *conns, const char *state_filter) 
{
    int found = 0;
    char line[128];

    for (int i = 0; i < conns->count; i++)
    {
        const connection_t *c = &conns->list[i];

        if (strcmp(c->state, state_filter) != 0)
        {
            continue;
        }

        if (found == 0)
        {
            print_section_title(state_filter, WINDOW_WIDTH);

            snprintf(line, sizeof(line), "%-15s %-21s", "LOCAL", "REMOTE");
            print_bordered(line, WINDOW_WIDTH);
        }

        snprintf(line, sizeof(line), "%-15s %-21s", c->local_ip, c->remote_addr);
        print_bordered(line, WINDOW_WIDTH);

        found++;
    }

    return found;
}

void net_print(const connection_list_t *conns)
{
    // order matters: this is the order sections will be displayed in
    const char *states_to_show[] = { "LISTEN", "ESTABLISHED", "TIME_WAIT" };
    int num_states = sizeof(states_to_show) / sizeof(states_to_show[0]);

    for (int s = 0; s < num_states; s++)
    {
        print_section(conns, states_to_show[s]);
    }
}