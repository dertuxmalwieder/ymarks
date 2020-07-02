/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */


#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <sqlite3.h>
#include "3rdparty/cJSON.h"
#include "constants.h"


static inline cJSON* download_bookmarks(void) {
    /* Return a JSON object, containing our bookmarks,
       as uploaded (but we only use the bookmarks[]
       part):
       { bm1, bm2, ... }
    */

    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;

    int rc = sqlite3_open(SQLFILE, &db);
    if (rc != SQLITE_OK) {
#ifndef SILENT
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
#endif
        return NULL;
    }

    /* This is pretty much unsorted (but the client can handle that): */
    char* sql = "SELECT folderID, ID, type, title, url, position, tags FROM bookmarks";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
#ifndef SILENT
        printf("Cannot prepare SELECT statement: %s\n", sqlite3_errmsg(db));
#endif
        sqlite3_close(db);
        return NULL;
    }

    /* Select was successful. */
    cJSON* root = cJSON_CreateArray();
    if (!root) {
        /* Failed to malloc(). */
#ifndef SILENT
        printf("Could not allocate the root object. Aborting.\n");
#endif
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    while (1) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            cJSON* bookmark = cJSON_CreateObject();
            if (!bookmark) {
#ifndef SILENT
                printf("Could not allocate a new bookmark object. Aborting.\n");
#endif
                cJSON_Delete(root);
                break;
            }

            /* Fill the bookmark: */
            cJSON_AddItemToObject(bookmark, "folderID", cJSON_CreateString(sqlite3_column_text(stmt, 0)));
            cJSON_AddItemToObject(bookmark, "ID", cJSON_CreateString(sqlite3_column_text(stmt, 1)));
            cJSON_AddItemToObject(bookmark, "type", cJSON_CreateString(sqlite3_column_text(stmt, 2)));
            cJSON_AddItemToObject(bookmark, "title", cJSON_CreateString(sqlite3_column_text(stmt, 3)));
            cJSON_AddItemToObject(bookmark, "url", cJSON_CreateString(sqlite3_column_text(stmt, 4)));
            cJSON_AddItemToObject(bookmark, "position", cJSON_CreateNumber((double) sqlite3_column_int(stmt, 5)));
            /* tbd: tags? */

            /* Attach a pointer on <bookmark> to <root>: */
            cJSON_AddItemToArray(root, bookmark);
        }
        else if (rc == SQLITE_DONE) {
            break;
        }
        else {
#ifndef SILENT
            printf("Cannot fetch your bookmarks (code: %d). Aborted.\n", rc);
#endif
            cJSON_Delete(root);

            break;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return root;
}

#endif
