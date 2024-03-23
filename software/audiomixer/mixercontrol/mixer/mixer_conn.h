#ifndef _MIXER_CONN_H_
#define _MIXER_CONN_H_

#define MIXER_READ_SIZE 1024
#define MIXER_WRITE_SIZE 1024

struct mixer_props {
    char *port;
    unsigned int baud;
};

struct mixer_connection {
    int fd;
    char *read_buffer;
    char *write_buffer;
};

struct mixer_connection *mixer_connect(const struct mixer_props *);

void mixer_disconnect(struct mixer_connection *);

#endif // _MIXER_CONN_H_
