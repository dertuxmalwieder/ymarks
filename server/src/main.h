/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * See the file COPYING in this distribution for details.
 * A copy of the CDDL is also available via the Internet at
 * http://www.opensource.org/licenses/cddl1.txt
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the contents of the COPYING file from this
 * distribution.
 */


#include "cjson/cJSON.h"
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
