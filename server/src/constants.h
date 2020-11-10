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


/* You are wholeheartedly invited to modify the following constants
   before compiling your own ymarks binary. */

#ifndef CONSTANTS_H
#define CONSTANTS_H



/* The port ymarks will bind its server component to: */
static const int   SERVERPORT   = 8888;

/* The folder in which backups will be created (if enabled): */
static const char* BACKUPFOLDER = ".";

/* The name of your database file: */
static const char* SQLFILE      = "ymarks.db";



#endif
