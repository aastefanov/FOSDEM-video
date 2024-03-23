#ifndef _MIXER_API_H_
#define _MIXER_API_H_

#include <stdint.h>

#include "mixer_conn.h"

uint8_t mixer_get_bus_count(const struct mixer_connection *conn);

uint8_t mixer_get_channel_count(const struct mixer_connection *conn, int bus);

ssize_t mixer_set_gain(struct mixer_connection *conn, int bus, int channel, uint8_t gain);

// uint8_t mixer_get_gain(const struct mixer_connection *conn, int bus, int channel);

ssize_t mixer_get_bus_gains(struct mixer_connection *conn,
                            int bus,
                            uint8_t out_gains[MIXER_CHANNEL_COUNT]);

ssize_t mixer_get_all_gains(struct mixer_connection *conn,
                            uint8_t out_gains[MIXER_BUS_COUNT][MIXER_CHANNEL_COUNT]);

#endif // _MIXER_API_H_
