/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
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
