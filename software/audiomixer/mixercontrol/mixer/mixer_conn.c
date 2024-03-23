#include <fcntl.h> // O_RDWR, O_NOCTTY
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "mixer_conn.h"

struct mixer_connection *mixer_connect(const struct mixer_props *const mixer,
                                       struct mixer_connection *const conn) {
    conn->fd = open(mixer->port, O_RDWR | O_NOCTTY);
    if (conn->fd < 0) goto cleanup;

    // https://gist.github.com/wdalmut/7480422
    struct termios tty;
    if (tcgetattr(conn->fd, &tty) != 0) goto cleanup;

    cfsetospeed(&tty, mixer->baud);
    cfsetispeed(&tty, mixer->baud);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    // tty.c_iflag &= ~IGNBRK; // disable break processing
    tty.c_lflag = 0; // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0; // no remapping, no delays
    tty.c_cc[VMIN] = 1; // read doesn't block
    tty.c_cc[VTIME] = 1; // 0.01 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(conn->fd, TCSANOW, &tty) != 0) goto cleanup;

    return conn;
cleanup:
    mixer_disconnect(conn);
    return NULL;
}

void mixer_disconnect(const struct mixer_connection *conn) {
    close(conn->fd);
}
