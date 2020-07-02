/**
 * ymarks :: a self-hosted single-user Xmarks alternative.
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */


#ifndef UPLOAD_H
#define UPLOAD_H

#include <sqlite3.h>
#include "3rdparty/cJSON.h"
#include "constants.h"
#include "types.h"
#include "database.h"
#include "backup.h"


static inline struct server_reply upload_bookmarks(cJSON* bookmarks) {
    /* Return a success/message pair after trying to
       store the supplied <bookmarks> object in the database.
    */
    int success = 1;

    char msg[100] = "";
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;

    /* Get a handle on the bookmarks: */
    int storedBookmarks = cJSON_GetArraySize(bookmarks);

    /* Try to open the bookmarks database: */
    int rc = sqlite3_open(SQLFILE, &db);
    if (rc != SQLITE_OK) {
        success = 0;
        snprintf(msg, sizeof(msg), "Cannot open database: %s", sqlite3_errmsg(db));
    }
    else {
#ifndef NO_BACKUP
        /* Try to create a backup: */
        sqlite3_close(db);

        if (!create_backups()) {
#  ifndef SILENT
            printf("Failed to backup '%s'.\n", SQLFILE);
#  endif
        }
        else {
#  ifndef SILENT
            printf("Successfully created a backup of the '%s' file.\n", SQLFILE);
#  endif
        }

        /* Reopen the database (which should still work at this point): */
        sqlite3_open(SQLFILE, &db);
#endif

        /* Recreate the bookmarks table: */
        char* sqlerror;
        rc = sqlite3_exec(db, bookmarks_sql, 0, 0, &sqlerror);
        if (rc != SQLITE_OK) {
            success = 0;
            snprintf(msg, sizeof(msg), "Cannot create table 'bookmarks': %s", sqlerror);
            sqlite3_free(sqlerror);
        }
        else {
            /* Traverse the bookmarks and add them step by step ... */
#ifndef SILENT
            int cnt = 0;
#endif
            const char* insertSQL = "INSERT INTO bookmarks VALUES(null, ?, ?, ?, ?, ?, ?, '');";
            sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
            sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sqlerror); /* yay performance */

            cJSON* firstbm = bookmarks->child;
            while (firstbm) {
                cJSON* folderID = cJSON_GetObjectItem(firstbm, "folderID");
                cJSON* ID = cJSON_GetObjectItem(firstbm, "ID");
                cJSON* type = cJSON_GetObjectItem(firstbm, "type");
                cJSON* title = cJSON_GetObjectItem(firstbm, "title");
                cJSON* url = cJSON_GetObjectItem(firstbm, "url");
                cJSON* position = cJSON_GetObjectItem(firstbm, "position");

                // Fun with browser interoperability:
                // - Firefox has "bookmark types".
                // - Chrome does not - it has no bookmark separators, so they don't need that.
                //
                // Workaround:
                // - Everything that has a URL is a "bookmark".
                // - Everything else is a "folder".
                const char* cfolderID = (folderID == NULL ? "''" : (cJSON_IsNumber(folderID) ? "-1" : folderID->valuestring));
                const char* cID = (ID == NULL ? "''" : ID->valuestring);
                const char* ctype = (type == NULL ? (url == NULL ? "folder" : "bookmark") : type->valuestring);
                const char* ctitle = (
                    type != NULL && strncmp(type->valuestring, "separator", strlen(type->valuestring)) == 0 ? "-" :
                    (title == NULL ? "(null)" :
                     (strlen(title->valuestring) > 0 ? title->valuestring :
                      "(null)")));
                const char* curl = (url == NULL ? "''" : (strlen(url->valuestring) > 0 ? url->valuestring : "''"));
                const double cpos = (position == NULL ? 0 : position->valuedouble);

                /* Bind the values: */
                sqlite3_bind_text(stmt, 1, cfolderID, -1, 0);
                sqlite3_bind_text(stmt, 2, cID, -1, 0);
                sqlite3_bind_text(stmt, 3, ctype, -1, 0);
                sqlite3_bind_text(stmt, 4, ctitle, -1, 0);
                sqlite3_bind_text(stmt, 5, curl, -1, 0);
                sqlite3_bind_int(stmt, 6, (int)cpos);

                /* More reliable counting: We don't need to "count" folders and separators. */
                if (strncmp(ctype, "bookmark", strlen(ctype)) != 0) {
                    storedBookmarks--;
                }

                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    success = 0;
                    snprintf(msg, sizeof(msg), "Cannot insert bookmarks into 'bookmarks': %s.", sqlite3_errmsg(db));
                    break;
                }
#ifndef SILENT
                /* See what happens: */
                printf("Wrote bookmark #%d [%s].\n", ++cnt, ctitle);
#endif
                /* Reset the SQL statement: */
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);

                /* Loop: */
                firstbm = firstbm->next;
            }
#ifndef SILENT
            printf("Done.\n");
#endif

            sqlite3_finalize(stmt);
            sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sqlerror);
            cJSON_Delete(firstbm);

            sqlite3_close(db);
        }
    }

    if (success) {
        /* Everything was fine up to this point. Build the success message! */
        if (storedBookmarks > 0) {
            snprintf(msg, sizeof(msg), "Successfully uploaded %d bookmarks.", storedBookmarks);
        }
        else {
            snprintf(msg, sizeof(msg), "Successfully uploaded no bookmarks.");
        }
    }

    struct server_reply ret;
    ret.success = success;
    ret.msg = msg;
    return ret;
}



#endif
