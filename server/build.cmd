@echo off
if not exist build mkdir build
rem bake sqlite3 right into the exe because why would we want to waste space for DLLs?
clang -Weverything --std=c11 src/main.c src/backup.c 3rdparty/cJSON.c sqlite3/sqlite-snapshot-201709211311/sqlite3.c -o build/ymarks.exe -static -I. -Isqlite3/sqlite-snapshot-201709211311
