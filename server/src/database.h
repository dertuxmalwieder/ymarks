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

#ifndef DATABASE_H
#define DATABASE_H


/* Database: */
const char* const bookmarks_sql = /* Recreate the bookmarks table */
    "DROP TABLE IF EXISTS bookmarks;"
    "CREATE TABLE bookmarks (rowid INTEGER PRIMARY KEY AUTOINCREMENT, folderID VARCHAR, ID VARCHAR NOT NULL UNIQUE, type VARCHAR, title VARCHAR, url VARCHAR, position INTEGER, tags VARCHAR);";
const char* const bookmarks_start_sql = /* Create the bookmarks table if needed */
    "CREATE TABLE IF NOT EXISTS bookmarks (rowid INTEGER PRIMARY KEY AUTOINCREMENT, folderID VARCHAR, ID VARCHAR NOT NULL UNIQUE, type VARCHAR, title VARCHAR, url VARCHAR, position INTEGER, tags VARCHAR);";


#endif
