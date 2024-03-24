#ifndef _MIXER_CONN_H_
#define _MIXER_CONN_H_

#include <termios.h>

#include "mixer_constants.h"

struct mixer_props {
    char *port;
    unsigned int baud;
};

struct mixer_connection {
    int fd;
    char read_buffer[MIXER_READ_SIZE];
    char write_buffer[MIXER_WRITE_SIZE];
    struct termios original_tty;
};

struct mixer_connection *mixer_connect(const struct mixer_props *, struct mixer_connection *);

void mixer_disconnect(const struct mixer_connection *);

#endif // _MIXER_CONN_H_
