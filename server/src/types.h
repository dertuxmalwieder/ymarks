/**
 * ymarks :: a self-hosted single-user bookmark sync service.
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.1 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * See the file COPYING in this distribution for details.
 * A copy of the CDDL is also available via the Internet at
 * https://spdx.org/licenses/CDDL-1.1.html
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the contents of the COPYING file from this
 * distribution.
 */


#ifndef YMARKS_TYPES
#define YMARKS_TYPES

/* Return value for JSON: */
struct server_reply {
    int success;
    char msg[100];
} reply;


#endif
