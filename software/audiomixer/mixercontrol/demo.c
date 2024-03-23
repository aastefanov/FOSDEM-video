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

    struct mixer_connection *conn;
    if ((conn = mixer_connect(&props)) == NULL) {
        fprintf(stderr, "cannot connect to mixer\n");
        goto cleanup;
    }

    int bus_count = 6;
    size_t *channel_counts = malloc(bus_count * sizeof(*channel_counts));
    for (i = 0; i < bus_count; i++) channel_counts[i] = 6; // all busses have 6 channels

    int t = 0;

    uint8_t **gains = malloc(bus_count * sizeof(*gains));
    for (i = 0; i < bus_count; i++) {
        gains[i] = malloc(channel_counts[i] * sizeof(**gains));
        for (j = 0; j < channel_counts[i]; j++) gains[i][j] = ++t;
    }

    for (i = 0; i < 6; i++)
        for (j = 0; j < 6; j++) mixer_set_gain(conn, i, j, gains[i][j]);

    mixer_get_all_gains(conn, gains, 6, channel_counts);
    // if (36 != cnt) {
    // fprintf(stderr, "cannot get all gains, got %d values\n", cnt);
    // goto cleanup;
    // }

    for (i = 0; i < bus_count; i++) {
        mixer_get_bus_gains(conn, i, gains[i], 6);
        for (j = 0; j < channel_counts[i]; j++) {
            printf("%d to %d: %d%%\n", i, j, gains[i][j]);
        }
        printf("\n");
    }

cleanup:
    mixer_disconnect(conn);
    return 0;
}
