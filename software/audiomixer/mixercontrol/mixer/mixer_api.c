#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mixer_conn.h"
#include "mixer_cmd.h"
#include "mixer_api.h"

#define MIXER_BUS_COUNT 6
#define MIXER_CHANNEL_COUNT 6

uint8_t mixer_get_bus_count(const struct mixer_connection *const conn) {
    return MIXER_BUS_COUNT;
}

uint8_t mixer_get_channel_count(const struct mixer_connection *const conn,
                                int bus) {
    return MIXER_CHANNEL_COUNT;
}

ssize_t mixer_set_gain(const struct mixer_connection *const conn,
                       const int bus,
                       const int channel,
                       const uint8_t gain) {
    int bytes = snprintf(conn->write_buffer, MIXER_WRITE_SIZE, "s%d%d%d\n", bus + 1, channel + 1, gain);
    if (bytes != mixer_write(conn, conn->write_buffer, bytes)) return -1;

    mixer_read(conn, conn->read_buffer, MIXER_READ_SIZE);
    return bytes;
}

uint8_t mixer_get_gain(const struct mixer_connection *const conn,
                       const int bus,
                       const int channel) {
    uint8_t *out_gains = malloc(mixer_get_channel_count(conn, bus) * sizeof(*out_gains));
    mixer_get_bus_gains(conn, bus, out_gains, mixer_get_channel_count(conn, bus));
    return out_gains[channel];
}

ssize_t mixer_get_bus_gains(const struct mixer_connection *const conn,
                            const int bus,
                            uint8_t *out_gains,
                            const size_t out_gains_length) {
    if (out_gains == NULL) return -1;

    // TODO: Change firmware so it doesn't write on set_gain, so flushing isn't needed,
    // or just return size_t of the response there?
    mixer_flush_buffers(conn, conn->read_buffer, MIXER_READ_SIZE);

    int bytes = snprintf(conn->write_buffer, MIXER_WRITE_SIZE, "l%d\r\n", bus + 1);

    if (bytes != mixer_write(conn, conn->write_buffer, bytes)) return -1;

    if ((bytes = mixer_read(conn, conn->read_buffer, MIXER_READ_SIZE - 1)) < 0) return bytes;

    conn->read_buffer[bytes] = '\0';

    int i = 0;

    // TODO: strcpy or memcpy? benchmark?
    // otherwise trashes the input buffer
    char temp[MIXER_READ_SIZE];
    memcpy(temp, conn->read_buffer, bytes + 1);

    const char *token = strtok(temp, " ");

    while (token != NULL && i < out_gains_length) {
        out_gains[i++] = atoi(token);
        token = strtok(NULL, " ");
    }

    return i;
}

ssize_t mixer_get_all_gains(const struct mixer_connection *const conn,
                            uint8_t **out_gains,
                            const size_t out_gains_length, const size_t *out_gains_channel_lengths) {
    if (out_gains == NULL) return -1;

    int i;
    int count = 0;
    int gains;

    for (i = 0; i < out_gains_length && i < mixer_get_bus_count(conn); i++) {
        gains = mixer_get_bus_gains(conn, i, out_gains[i], out_gains_channel_lengths[i]);
        if (gains < 0) return count; // cannot read all channels
        count += gains;
    }

    return count;
}
