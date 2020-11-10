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


#ifndef NO_BACKUP

#include <stdio.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#else
#  include <unistd.h>
#endif

#include "backup.h"
#include "download.h"
#include "constants.h"


int create_backups(void) {
    /* Creates a backup of the SQLFILE in BACKUPFOLDER. Keeps two copies at most. */
    const char backupname1[200];
    const char backupname2[200];

    snprintf(backupname1, sizeof(backupname1), "%s/%s.backup.1", BACKUPFOLDER, SQLFILE);
    snprintf(backupname2, sizeof(backupname2), "%s/%s.backup.2", BACKUPFOLDER, SQLFILE);

    FILE* backupfile2 = fopen(backupname2, "w");
    if (backupfile2) {
        /* The second backup file exists. Delete it and move the first
           one into it. */
        fclose(backupfile2);
#ifdef _WIN32
        if (_unlink(backupname2) < 0) {
#  ifndef SILENT
            printf("Failed to delete your existing backups. Please check your access!\n");
#  endif
            return 1;
        }
#else
        if (unlink(backupname2) < 0) {
#  ifndef SILENT
            printf("Failed to delete your existing backups. Please check your access!\n");
#  endif
            return 1;
        }
#endif
    }

    FILE* backupfile1 = fopen(backupname1, "r");
    if (backupfile1) {
        /* Move it. */
        fclose(backupfile1);
        if (rename(backupname1, backupname2) != 0) {
#ifndef SILENT
            printf("Failed to rename your existing backups. Please check your access!\n");
#endif
            return 1;
        }
    }

    /* At this point, the databases should be closed and <backupname1> should be free. */
#ifdef _WIN32
    /* Use the Windows API copy routine. */
    return CopyFile(SQLFILE, backupname1, 1);
#else
    /* Use the boring method. */
    FILE* copyFrom = fopen(SQLFILE, "r");
    FILE* copyTo = fopen(backupname1, "w");

    if (!copyFrom || !copyTo) {
#  ifndef SILENT
        printf("Failed to copy '%s' into '%s' - have you checked your access yet?\n", SQLFILE, backupname1);
#  endif
        return 1;
    }

    for (;;) {
        int thisChar = fgetc(copyFrom);
        if (thisChar != EOF) {
            fputc(thisChar, copyTo);
        }
        else {
            break;
        }
    }
    fclose(copyFrom);
    fclose(copyTo);

    return 0;
#endif
}


cJSON* download_backup(void) {
    /* Returns the latest accessible backup object. */
    /* Copy the newest available backup over the existing database,
       then download that database as usual. */

    const char backupname1[200];
    const char backupname2[200];

    snprintf(backupname1, sizeof(backupname1), "%s/%s.backup.1", BACKUPFOLDER, SQLFILE);
    snprintf(backupname2, sizeof(backupname2), "%s/%s.backup.2", BACKUPFOLDER, SQLFILE);

    FILE* backupfile = fopen(backupname1, "w");
    if (backupfile) {
        /* Yup, that exists. */
        if (rename(backupname1, SQLFILE) != 0) {
            /* Something went wrong here. */
#ifndef SILENT
            printf("Could not rename the database. :-(\n");
#endif
            return NULL;
        }
    }

    FILE* oldbackupfile = fopen(backupname2, "w");
    if (oldbackupfile) {
        /* Make the old backup file the new one. */
        if (rename(backupname2, backupname1) != 0) {
            /* Something went wrong here. */
#ifndef SILENT
            printf("Could not rename the old backup. :-(\n");
#endif
            /* We don't need to fail here though. */
        }
    }

    return download_bookmarks();
}

#endif
