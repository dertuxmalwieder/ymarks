/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#ifndef DATABASE_H
#define DATABASE_H


/* Database: */
const char* const bookmarks_sql = /* Recreate the bookmarks table */
    "DROP TABLE IF EXISTS bookmarks;"
    "CREATE TABLE bookmarks (rowid INTEGER PRIMARY KEY AUTOINCREMENT, folderID VARCHAR, ID VARCHAR NOT NULL UNIQUE, type VARCHAR, title VARCHAR, url VARCHAR, position INTEGER, tags VARCHAR);";
const char* const bookmarks_start_sql = /* Create the bookmarks table if needed */
    "CREATE TABLE IF NOT EXISTS bookmarks (rowid INTEGER PRIMARY KEY AUTOINCREMENT, folderID VARCHAR, ID VARCHAR NOT NULL UNIQUE, type VARCHAR, title VARCHAR, url VARCHAR, position INTEGER, tags VARCHAR);";


#endif
