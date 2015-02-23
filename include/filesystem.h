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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <stdbool.h>
#include <sys/stat.h>

int path_create(char *path, mode_t mode);

int path_remove(char *__restrict path);

bool path_exists(const char *__restrict path);

bool path_is_reg(const char *__restrict path);

bool path_is_dir(const char *__restrict path);

bool path_is_sock(const char *__restrict path);

bool path_is_fifo(const char *__restrict path);

bool path_is_absolute(const char *__restrict path);

#endif /* _FILESYSTEM_H_ */