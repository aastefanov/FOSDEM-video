#ifndef _MIXER_CMD_H_
#define _MIXER_CMD_H_

#include <stddef.h>
#include <unistd.h>

#include "mixer_conn.h"

ssize_t mixer_read(const struct mixer_connection *conn, const char *buf, size_t len);

void mixer_flush_buffers(const struct mixer_connection *conn, const char *buf, size_t len);

ssize_t mixer_write(const struct mixer_connection *conn, const char *buf, size_t len);


#endif // _MIXER_CMD_H_
