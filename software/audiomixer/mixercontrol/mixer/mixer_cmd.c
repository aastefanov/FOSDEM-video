#include <stddef.h>
#include <unistd.h>
#include <termios.h> // tcflush

#include "mixer_conn.h"
#include "mixer_cmd.h"

ssize_t mixer_read(const struct mixer_connection *conn, const char *buf, const size_t len) {
    return read(conn->fd, (void *) buf, len);
}

void mixer_flush_buffers(const struct mixer_connection *conn, const char *buf, const size_t len) {
    // Note: buf and len are not used right now, but if tcflush doesn't work with the usb->serial,
    // we could just discard everything in the file read buffer by changing it to something like:
    // while(0 != mixer_read(conn->df, (void*) buf, len)) {}
    tcflush(conn->fd, TCIOFLUSH);
}

ssize_t mixer_write(const struct mixer_connection *conn, const char *const buf, const size_t len) {
    return write(conn->fd, buf, len);
}
