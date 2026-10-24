#include <stdio.h>
#include <string.h>
#include "ui.h"

void print_bordered(const char *text, int width) {
    int len = (int)strlen(text);
    int padding = width - len;

    if (padding < 0) {
        padding = 0; // if text > width no negativ space
    }

    printf("⣿⣿⣿%s%*s⣿⣿⣿\n", text, padding, "");
}