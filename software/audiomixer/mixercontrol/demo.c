#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mixer/mixer.h"

int main(int argc, char **argv) {
    int i, j;

    if (argc < 3) return 1;
    char *port = argv[1];
    unsigned int baud = atoi(argv[2]);

    if (!strlen(port) || !baud) return 1;

    struct mixer_props props = {.port = port, .baud = baud};

    struct mixer_connection conn;
    if (mixer_connect(&props, &conn) < 0) {
        fprintf(stderr, "cannot connect to mixer\n");
        goto cleanup;
    }

    uint8_t gains[MIXER_BUS_COUNT][MIXER_CHANNEL_COUNT];
    for (i = 0; i < MIXER_BUS_COUNT; i++) {
        for (j = 0; j < MIXER_CHANNEL_COUNT; j++) gains[i][j] = i * MIXER_BUS_COUNT + j + 1;
    }

    for (i = 0; i < 6; i++)
        for (j = 0; j < 6; j++) mixer_set_gain(&conn, i, j, gains[i][j]);

    mixer_get_all_gains(&conn, gains);
    // if (36 != cnt) {
    // fprintf(stderr, "cannot get all gains, got %d values\n", cnt);
    // goto cleanup;
    // }

    for (i = 0; i < MIXER_BUS_COUNT; i++) {
        mixer_get_bus_gains(&conn, i, gains[i]);
        for (j = 0; j < MIXER_CHANNEL_COUNT; j++) {
            printf("%d to %d: %d%%\n", i, j, gains[i][j]);
        }
        printf("\n");
    }

cleanup:
    mixer_disconnect(&conn);
    return 0;
}
