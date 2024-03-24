#ifndef _MIXER_API_H_
#define _MIXER_API_H_

#include <stdint.h>

#include "mixer_conn.h"

/**
 * Set the gain of a bus/channel pair
 *
 * @param[in] conn An established connection
 * @param[in] bus Targeted bus to change gain to
 * @param[in] channel Targeted channel to change gain from
 * @param[in] gain The new gain as a number in 1-100
 *
 * @return Positive value on success, negative on failure
 */
ssize_t mixer_set_gain(struct mixer_connection *conn, int bus, int channel, uint8_t gain);

// uint8_t mixer_get_gain(const struct mixer_connection *conn, int bus, int channel);

/**
 * Get the gain of all channels on a specific bus
 *
 * @param conn An established connection
 * @param[in] bus The bus to get the current gains for
 * @param[out] out_gains The current gain values
 *
 * @return Count of gains pulled, negative on error
 */
ssize_t mixer_get_bus_gains(struct mixer_connection *conn,
                            int bus,
                            uint8_t out_gains[MIXER_CHANNEL_COUNT]);

/**
 * Get the gain of all channels on all busses
 *
 * @param[in] conn An established connection
 * @param[out] out_gains The current gain values
 *
 * @return Count of gains pulled, negative on error
 */
ssize_t mixer_get_all_gains(struct mixer_connection *conn,
                            uint8_t out_gains[MIXER_BUS_COUNT][MIXER_CHANNEL_COUNT]);

#endif // _MIXER_API_H_
