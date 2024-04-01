#include "../mongoose/mongoose.h"
#include "../mixer/mixer.h"

#define MAX_URI_PARAMS 100

#define SCRATCH_BUFFER_LENGTH 1024

#include <stdio.h>


uint8_t gains[MIXER_BUS_COUNT][MIXER_CHANNEL_COUNT];
struct mg_str request_params[MAX_URI_PARAMS];

struct mg_mgr mgr;
struct mixer_connection mixer_conn;

static int signo = 0;

char scratch[SCRATCH_BUFFER_LENGTH];

int setup_web_server(struct mg_mgr *conmgr, char *url);

static void handle_request(struct mg_connection *c, int ev, void *ev_data);

static void on_close(int i);


int main(int argc, char **argv) {
    char url[] = "http://localhost:8080";

    signal(SIGINT, on_close);
    signal(SIGTERM, on_close);

    struct mixer_props props = {.port = "../pty", .baud = 9600};

    if (mixer_connect(&props, &mixer_conn) < 0) {
        fprintf(stderr, "cannot connect to mixer\n");
        return 1;
    }
    if (setup_web_server(&mgr, url) < 0) {
        return 1;
    }

    fprintf(stderr, "Listening on: %s\n", url);

    while (signo == 0) {
        mg_mgr_poll(&mgr, 1000);
    }

    on_close(0);

    return 0;
}

int setup_web_server(struct mg_mgr *conmgr, char *url) {
    mg_mgr_init(conmgr); // Initialise event manager
    return mg_http_listen(conmgr, url, handle_request, NULL) == NULL ? -1 : 0; // Setup listener
}


static void route(struct mg_connection *c, struct mg_http_message *message) {
    if (mg_match(message->uri, mg_str("/api/gains/*/*"), request_params) &&
        mg_strcmp(message->method, mg_str("POST")) == 0) {
        char *ch;

        strncpy(scratch, request_params[0].ptr, request_params[0].len + 1);
        ch = NULL;
        int bus = (int) strtol(scratch, &ch, 10);

        if (ch != scratch + request_params[0].len)
            return mg_http_reply(c, 400, "", "Bus is an integer\r\n");

        strncpy(scratch, request_params[1].ptr, request_params[1].len + 1);
        ch = NULL;
        int channel = (int) strtol(scratch, &ch, 10);

        if (ch != scratch + request_params[1].len)
            return mg_http_reply(c, 400, "", "Channel is an integer\r\n");

        strncpy(scratch, message->body.ptr, message->body.len + 1);
        ch = NULL;
        uint8_t gain = (uint8_t) strtol(scratch, &ch, 10);
        if (ch != scratch + message->body.len)
            return mg_http_reply(c, 400, "", "Gain is an integer\r\n");

        if (mixer_set_gain(&mixer_conn, bus, channel, gain) < 0)
            return mg_http_reply(c, 500, "", "Failed setting gain\n");

        return mg_http_reply(c, 200, "", scratch);
    }

    if (mg_match(message->uri, mg_str("/api/gains/*.json"), request_params)) {
        strncpy(scratch, request_params[0].ptr, request_params[0].len + 1);
        char *ch = NULL;
        int bus = (int) strtol(scratch, &ch, 10);
        if (ch != scratch + request_params[0].len)
            return mg_http_reply(c, 400,
                                 "Content-Type: application/json\r\n",
                                 "{%m: %m}", MG_ESC("error"), MG_ESC("Bus is an integer"));

        if (mixer_get_bus_gains(&mixer_conn, bus, gains[bus]) < 0) {
            return mg_http_reply(c, 500,
                                 "Content-Type: application/json\r\n",
                                 "{%m: %m}", MG_ESC("error"), MG_ESC("Failed polling mixer"));
        }
        size_t pos = 0;
        pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "[");
        for (int i = 0; i < MIXER_BUS_COUNT; i++) {
            pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "%d,", gains[bus][i]);
        }
        pos += mg_snprintf(scratch + pos - 1, SCRATCH_BUFFER_LENGTH - pos - 1, "]\n") - 1; // overwrite last comma
        scratch[pos] = 0; // terminate
        return mg_http_reply(c, 200, "", scratch);
    }

    if (mg_match(message->uri, mg_str("/api/gains/*"), request_params)) {
        strncpy(scratch, request_params[0].ptr, request_params[0].len + 1);
        char *ch = NULL;
        int bus = (int) strtol(scratch, &ch, 10);
        if (ch != scratch + request_params[0].len)
            return mg_http_reply(c, 400, "", "Bus is an integer\n");

        if (mixer_get_bus_gains(&mixer_conn, bus, gains[bus]) < 0) {
            return mg_http_reply(c, 500, "", "Failed getting info from mixer\n");
        }
        size_t pos = 0;
        for (int i = 0; i < MIXER_BUS_COUNT; i++) {
            pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "%d\r\n", gains[bus][i]) - 1;
        }
        scratch[pos] = 0; // terminate
        return mg_http_reply(c, 200, "", scratch);
    }

    if (mg_match(message->uri, mg_str("/api/gains.json"), request_params)) {
        if (mixer_get_all_gains(&mixer_conn, gains) < 0) {
            return mg_http_reply(c, 500,
                                 "Content-Type: application/json\r\n",
                                 "{%m: %m}", MG_ESC("error"), MG_ESC("Failed polling mixer"));
        }
        size_t pos = 0;

        pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "[");
        for (int i = 0; i < MIXER_BUS_COUNT; i++) {
            pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "[");
            for (int j = 0; j < MIXER_CHANNEL_COUNT; j++) {
                pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "%d,", gains[i][j]);
            }
            pos += mg_snprintf(scratch + pos - 1, SCRATCH_BUFFER_LENGTH - pos - 1, "],") - 1; // overwrite last comma
        }
        pos += mg_snprintf(scratch + pos - 1, SCRATCH_BUFFER_LENGTH - pos - 1, "]\r\n") - 1; // overwrite last comma
        scratch[pos] = 0; // terminate string
        return mg_http_reply(c, 200, "", scratch);
    }


    if (mg_match(message->uri, mg_str("/api/gains"), request_params)) {
        if (mixer_get_all_gains(&mixer_conn, gains) < 0) {
            return mg_http_reply(c, 500, "", "Failed getting info from mixer");
        }
        size_t pos = 0;
        for (int i = 0; i < MIXER_BUS_COUNT; i++) {
            for (int j = 0; j < MIXER_CHANNEL_COUNT; j++) {
                pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, " %d", gains[i][j]);
            }
            pos += mg_snprintf(scratch + pos, SCRATCH_BUFFER_LENGTH - pos - 1, "\r\n");
        }
        scratch[pos] = 0; // terminate string
        return mg_http_reply(c, 200, "", scratch);
    }

    // if no route matched
    return mg_http_reply(c, 404, "", "Not found");
}

static void handle_request(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) return route(c, ev_data);
}

static void on_close(int i) {
    signo = i;
    mixer_disconnect(&mixer_conn);
    mg_mgr_free(&mgr);
}
