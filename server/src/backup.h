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


#ifndef BACKUP_H
#define BACKUP_H

#include "cjson/cJSON.h"

int create_backups(void);
cJSON* download_backup(void);

#endif
