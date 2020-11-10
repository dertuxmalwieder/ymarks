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


/* Note: You are not expected to modify this code. You might want to
   modify constants.h though. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include "sqlite3.h"
#include "cjson/cJSON.h"

/* Fuck NIH */
#define WBY_STATIC
#define WBY_IMPLEMENTATION
#define WBY_USE_FIXED_TYPES
#define WBY_USE_ASSERT
#include "3rdparty/web.h"

#include "constants.h"
#include "download.h"
#include "upload.h"
#include "database.h"
#ifndef NO_BACKUP
#  include "backup.h"
#endif
#include "main.h"

#ifdef _WIN32
#  pragma comment(lib, "ws2_32")
#  include <WinSock2.h>
#  ifndef KEEP_VISIBLE
#    define WIN32_LEAN_AND_MEAN
#    pragma comment(lib, "shell32")
#    include <Windows.h>
#  endif
#else
#  include <unistd.h>
#endif


/* The server part retrieves and sends our bookmarks without a nested
 * structure. We leave the complicated job to the client.
 *
 * var jsonObj = {
 *     endpoint: "",
 *     pin: "",
 *     bookmarks: [ ]
 * };
 */


void intHandler(int sig) {
    /* Shut down the server and free the memory if required. */
    signal(sig, SIG_IGN);

#ifndef SILENT
    printf("Quitting ymarks ...\n");
#endif
    wby_stop(&server);
    free(memory);
#ifdef _WIN32
    WSACleanup();
#endif

    exit(0);
}


int main(void) {
    /* Make sure we have the server shut down properly. */
    signal(SIGINT, intHandler);

    /* Validate the database: */
    sqlite3* db = NULL;
    int rc = sqlite3_open(SQLFILE, &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    /* CREATE IF NOT EXISTS: */
    char* sqlerror;
    rc = sqlite3_exec(db, bookmarks_start_sql, 0, 0, &sqlerror);
    if (rc != SQLITE_OK) {
        printf("Cannot create table 'bookmarks': %s", sqlerror);
        sqlite3_free(sqlerror);
        sqlite3_close(db);

        return 1;
    }

    /* Set up the web server: */
    void *memory = NULL;
    wby_size needed_memory = 0;

    /* Provide the relevant handlers and variables: */
    struct wby_config config;
    memset(&config, 0, sizeof config);
    config.userdata = &state;
    config.address = "0.0.0.0";
    config.port = SERVERPORT;
    config.connection_max = 4;
    config.request_buffer_size = 2048;
    config.io_buffer_size = 8192;
    config.dispatch = dispatch;

#ifdef _WIN32
    WORD wsa_version = MAKEWORD(2,2);
    WSADATA wsa_data;
    if (WSAStartup(wsa_version, &wsa_data)) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

#  ifndef KEEP_VISIBLE
    /* We don't need the main window. */
    FreeConsole();
#  endif
#endif

    wby_init(&server, &config, &needed_memory);
    memory = calloc(needed_memory, 1);
    wby_start(&server, memory);

    memset(&state, 0, sizeof state);
    while (!state.quit) {
        wby_update(&server);
    }

#ifndef SILENT
    printf("Quitting ymarks ...\n");
#endif
    wby_stop(&server);
    free(memory);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}


/* ---------------[ SERVER THINGS ] --------------- */

static int dispatch(struct wby_con *connection, void *userdata) {
    /* Handle requests */
    size_t len = connection->request.content_length;
    unsigned char buffer[len];

#ifndef SILENT
    printf("Request --> %s:\n", connection->request.uri);
    printf("   Method: %s\n", connection->request.method);
    printf("   Params: %s\n", connection->request.query_params);
    printf("   ConLen: %d\n", len);
#endif

    if (len > 0) {
        wby_read(connection, buffer, len);
        buffer[sizeof(buffer)] = '\0';
    }

    if (!strcmp("/upload", connection->request.uri)) {
        /* --- UPLOAD MODE --- */
        /* <buffer> probably contains a JSON object of our bookmarks now. */
        cJSON* uploaded = cJSON_Parse(buffer);
        struct server_reply pin = pincheck(uploaded);
        if (!pin.success) {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", pin.msg);
            const char* returnvalue = cJSON_Print(root);

            write_response(connection, returnvalue);

            cJSON_Delete(root);
            return 0;
        }

        struct server_reply uploader = upload_bookmarks(cJSON_GetObjectItem(uploaded, "bookmarks"));

        cJSON* root = cJSON_CreateObject();
        if (uploader.success) {
            cJSON_AddTrueToObject(root, "success");
        }
        else {
            cJSON_AddFalseToObject(root, "success");
        }
        cJSON_AddStringToObject(root, "message", uploader.msg);

        const char* returnvalue = cJSON_Print(root);

        write_response(connection, returnvalue);

        cJSON_Delete(root);
        cJSON_Delete(uploaded);
    }
    else if (!strcmp("/download", connection->request.uri)) {
        /* --- DOWNLOAD MODE --- */
        cJSON* uploaded = cJSON_Parse(buffer);
        struct server_reply pin = pincheck(uploaded);
        if (!pin.success) {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", pin.msg);
            const char* returnvalue = cJSON_Print(root);

            write_response(connection, returnvalue);

            cJSON_Delete(root);
            return 0;
        }

        /* The PIN is valid. Get the current set of stored bookmarks and
           print them back as JSON. (We'll still try to follow the format
           from the /upload.)
        */

        cJSON* root = cJSON_CreateObject();
        cJSON* bookmarks = download_bookmarks();

        if (bookmarks == NULL) {
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", "Could not retrieve your bookmarks. Mind to try again later?");
        }
        else {
            cJSON_AddTrueToObject(root, "success");
            cJSON_AddStringToObject(root, "message", cJSON_Print(bookmarks));

            write_response(connection, cJSON_Print(root));

            cJSON_Delete(bookmarks);
        }

        cJSON_Delete(root);
        cJSON_Delete(uploaded);
    }
    else if (!strcmp("/restore", connection->request.uri)) {
        /* --- PANIC MODE --- */
        cJSON* root = cJSON_CreateObject();

#ifndef NO_BACKUP
        cJSON* uploaded = cJSON_Parse(buffer);
        struct server_reply pin = pincheck(uploaded);
        if (!pin.success) {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", pin.msg);
            const char* returnvalue = cJSON_Print(root);

            write_response(connection, returnvalue);

            cJSON_Delete(root);
            return 0;
        }

        /* Download the previous backup or the one before that. */
        cJSON* bookmarks = download_backup();
        if (bookmarks == NULL) {
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", "Could not restore your latest backup. Sorry, you're screwed.");
        }
        else {
            cJSON_AddTrueToObject(root, "success");
            cJSON_AddStringToObject(root, "message", cJSON_Print(bookmarks));

            cJSON_Delete(bookmarks);
        }
#else
        /* There is no backup functionality here. Yikes. */
        cJSON_AddFalseToObject(root, "success");
        cJSON_AddStringToObject(root, "message", "Your server does not support backups. Sorry, you're screwed.");
#endif
        write_response(connection, cJSON_Print(root));

        cJSON_Delete(root);
    }
    else if (!strcmp("/quit", connection->request.uri)) {
        /* --- REMOTE QUIT --- */
        cJSON* uploaded = cJSON_Parse(buffer);
        struct server_reply pin = pincheck(uploaded);
        if (pin.success) {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddTrueToObject(root, "success");
            cJSON_AddStringToObject(root, "message", "The ymarks service will be stopped.");
            const char* returnvalue = cJSON_Print(root);

            write_response(connection, returnvalue);

            /* Quit the ymarks server. */
            cJSON_Delete(root);
            cJSON_Delete(uploaded);
#ifndef SILENT
            printf("Quitting ymarks ...\n");
#endif
            wby_stop(&server);
            free(memory);
#ifdef _WIN32
            WSACleanup();
#endif
            exit(0);
        }

        cJSON_Delete(uploaded);
    }
    else {
        /* neither /upload nor /download nor /restore nor /quit. */
#ifdef __STDC_LIB_EXT1__
        if (strnlen_s(connection->request.uri, 25) == 0 || strncmp("/", connection->request.uri, strnlen_s(connection->request.uri, 25)) == 0) {
#else
        if (strlen(connection->request.uri) == 0 || strncmp("/", connection->request.uri, strlen(connection->request.uri)) == 0) {
#endif
            /* No endpoint chosen. Chances are that the user just tried to open the "website". */
            static const struct wby_header html_headers[] = {
                { .name = "Content-Type", .value = "text/html" }
            };

            const char* const text = "You are not expected to visit the backend in a web browser. Please just use the browser extension for communicating with me! :-)";

            wby_response_begin(connection, 200, 128, html_headers, 0);
            wby_write(connection, text, 128);
            wby_response_end(connection);
        }
        else {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddFalseToObject(root, "success");
            cJSON_AddStringToObject(root, "message", "This endpoint is unknown.");

            write_response(connection, cJSON_Print(root));

            cJSON_Delete(root);
        }
    }

    return 0;
}


void write_response(struct wby_con *connection, const char* text) {
    /* Prints <text> over <connection> as JSON. */
    static const struct wby_header json_headers[] = {
        { .name = "Content-Type", .value = "application/json" }
    };

    wby_response_begin(connection, 200, strlen(text), json_headers, 0);
    wby_write(connection, text, strlen(text));
    wby_response_end(connection);
}


struct server_reply pincheck(cJSON* uploaded) {
    /* Compare the submitted PIN to our local one: */
    struct server_reply ret;
    int success = 1;
    char msg[100];

    FILE *pinfile = fopen("PIN.txt", "r");
    if (pinfile) {
        /* We don't want to use "12345" here. */
        char pintext[10]; /* 10-digit PINs could fit into INT_MAX. */
        fgets(pintext, sizeof(pintext), pinfile);
        long PINfromFile = strtol(pintext, NULL, 10);
        if (PINfromFile <= INT_MAX && PINfromFile >= 0) {
            /* Valid PINs are positive integers. Exclusively. */
            localPIN = (int) PINfromFile;
        }
        fclose(pinfile);
    }

    cJSON* pinobj = cJSON_GetObjectItem(uploaded, "pin");
    if (!pinobj) {
        /* No PIN supplied. */
        success = 0;
        snprintf(msg, sizeof(msg), "%s", "You did not send us your PIN. Mind to check your settings?");
    }
    else {
        if (!cJSON_IsNumber(pinobj) || pinobj->valuedouble != localPIN) {
            success = 0;
            snprintf(msg, sizeof(msg), "%s", "Your PIN does not match our records.");
        }
    }

    ret.success = success;
    ret.msg = msg;

    return ret;
}
