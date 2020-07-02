/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */


#ifndef YMARKS_TYPES
#define YMARKS_TYPES

/* Return value for JSON: */
struct server_reply {
    int success;
    char* msg;
} reply;


#endif
