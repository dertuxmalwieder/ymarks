/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */


#include "3rdparty/cJSON.h"
#include "types.h"


/* Misc.: */
static int localPIN = 12345;
void intHandler(int);
struct server_reply pincheck(cJSON* uploaded);


/* Web server setup: */
#define MAX_WSCONN 8

struct server_state {
    int quit;
    unsigned frame_counter;
    struct wby_con *conn[MAX_WSCONN];
    int conn_count;
} state;


void *memory = NULL;
struct wby_server server;
void write_response(struct wby_con *connection, const char* text);
static int dispatch(struct wby_con *connection, void *userdata);
