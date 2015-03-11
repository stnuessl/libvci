/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Steffen Nuessle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "filesystem.h"

static int remove_dir_entries(const char *__restrict path)
{
    struct dirent entry, *result;
    char *buffer;
    DIR *dir;
    size_t len;
    int err;
    
    dir = opendir(path);
    if (!dir)
        return -errno;
    
    len = strlen(path);
    buffer = malloc(len + 1 + sizeof(entry.d_name));
    if (!buffer) {
        closedir(dir);
        return -ENOMEM;
    }
    
    buffer = strcpy(buffer, path);
    if (buffer[len] != '/')
        buffer = strcat(buffer, "/");
    
    while (1) {
        err = readdir_r(dir, &entry, &result);
        if (err || !result)
            break;
        
        buffer = strcpy(buffer + len + 1, entry.d_name);
        
        if (entry.d_type == DT_DIR) {
            err = remove_dir_entries(buffer);
            if (err < 0)
                break;
            
            err = rmdir(buffer);
            if (err < 0)
                break;
        } else {
            err = unlink(buffer);
            if (err < 0)
                break;
        }
    }
    
    free(buffer);
    closedir(dir);
    
    return err;
}

int path_create(char *path, mode_t mode)
{
    struct stat st;
    char *p;
    int err;
    
    p = path;
    
    while(*p == '/')
        p += 1;
    
    for(p = strchrnul(p, '/'); *p != '\0'; p = strchrnul(p + 1, '/')) {
        
        while(*++p == '/')
            ;
        
        *--p = '\0';
        
        err = stat(path, &st);
        if(err < 0) {
            if(errno != ENOENT) {
                err = -errno;
                goto fail;
            }
        } else {
            if(S_ISDIR(st.st_mode)) {
                *p = '/';
                continue;
            }
        }
        
        err = mkdir(path, mode);
        
        *p++ = '/';
        
        if(err < 0) {
            if(errno != EEXIST) {
                err = -errno;
                goto fail;
            }
        }
    }

    return 0;
    
fail:
    return err;
}

int path_remove(char *__restrict path)
{
    int err;
    
    if (path_is_dir(path)) {
        err = remove_dir_entries(path);
        if (err < 0)
            return err;
        
        err = rmdir(path);
        if (err < 0)
            return err;
    } else {
        err = unlink(path);
        if (err < 0)
            return -errno;
    }
    
    return 0;
}

bool path_exists(const char *__restrict path)
{
    struct stat st;
    
    return stat(path, &st) == 0 || errno != ENOENT;
}

bool path_is_reg(const char *__restrict path)
{
    struct stat st;
    
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool path_is_dir(const char *__restrict path)
{
    struct stat st;
    
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

bool path_is_sock(const char *__restrict path)
{
    struct stat st;
    
    return stat(path, &st) == 0 && S_ISSOCK(st.st_mode);
}

bool path_is_fifo(const char *__restrict path)
{
    struct stat st;
    
    return stat(path, &st) == 0 && S_ISFIFO(st.st_mode);
}

bool path_is_absolute(const char *__restrict path)
{
    return path && *path == '/';
}