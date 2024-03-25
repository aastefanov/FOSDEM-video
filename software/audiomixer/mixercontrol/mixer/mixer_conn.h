#ifndef _MIXER_CONN_H_
#define _MIXER_CONN_H_

#include <termios.h>

#include "mixer_constants.h"

/**
 * @struct mixer_props
 * Defines the prameters for establishing a new \c mixer_connection
 *
 */
struct mixer_props {
    char *port; ///< Path to a tty/pty device
    unsigned int baud; ///< Baud rate to use, as an integer
};

/**
 * @struct mixer_connection
 * Tracks the connection state, used internally
 *
 * @sa mixer_connect
 */
struct mixer_connection {
    int fd; ///< File descriptor to the tty used, used internally
    char read_buffer[MIXER_READ_SIZE]; ///< Read buffer, used internally
    char write_buffer[MIXER_WRITE_SIZE]; ///< Write buffer, used internally
    struct termios original_tty; ///< Original tty settings, to restore on \c mixer_disconnect()
};


/**
 * Connect to the mixer with the specified settings
 *
 * @param[in] params The mixer properties
 * @param[out] conn The established connection
 *
 * @sa mixer_disconnect
 * @return Zero on success, -1 on error
 */
ssize_t mixer_connect(const struct mixer_props *params, struct mixer_connection *conn);

/**
 * Close an established connection
 *
 * @param[in,out] conn The connection to close
 */
void mixer_disconnect(const struct mixer_connection *conn);

#endif // _MIXER_CONN_H_
