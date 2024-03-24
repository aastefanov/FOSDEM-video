#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#include "mixer_conn.h"

// sadly, we have to keep a mapping table, as the termios.h constants
// are actually different than the integers
speed_t get_baud(int baud) {
    switch (baud) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            return B9600;
    }
}

ssize_t *mixer_connect(const struct mixer_props *const mixer,
                       struct mixer_connection *const conn) {
    conn->fd = open(mixer->port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (conn->fd < 0) goto cleanup;

    // we flush the buffers and actually open it as nonblocking so we don't hang
    // then we set it to blocking again
    // otherwise, if the buffers aren't empty (for example due to an unclean exit)
    // and the open(2) call hangs
    tcflush(conn->fd, TCIOFLUSH);
    int flags = fcntl(conn->fd, F_GETFL);
    fcntl(conn->fd, F_SETFL, flags & (~O_NONBLOCK));

    // https://gist.github.com/wdalmut/7480422
    struct termios tty;

    if (tcgetattr(conn->fd, &tty) != 0) goto cleanup;
    conn->original_tty = tty; // we need to restore the settings on exit

    cfsetospeed(&tty, get_baud(mixer->baud));
    cfsetispeed(&tty, get_baud(mixer->baud));

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag |= ICANON | IGNBRK | ICRNL;

    tty.c_oflag |= ICANON;

    tty.c_cflag &= ~(ECHO | CLOCAL | HUPCL | PARENB | PARODD | CSTOPB | CSIZE);
    tty.c_cflag |= CS8 | CREAD;

    if (tcsetattr(conn->fd, TCSANOW, &tty) != 0) goto cleanup;

    return 0;

cleanup:
    mixer_disconnect(conn);
    return -1;
}

void mixer_disconnect(const struct mixer_connection *conn) {
    if (conn->fd < 0) return;
    tcsetattr(conn->fd, TCSANOW, &conn->original_tty);
    close(conn->fd);
}
