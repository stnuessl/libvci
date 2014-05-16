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

#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>

#include "link.h"

struct link *list_new(void);

void list_delete(struct link *__restrict list,
                  void (*data_delete)(struct link *));

void list_init(struct link *__restrict list);

void list_destroy(struct link *__restrict list,
                  void (*data_delete)(struct link *));

void list_clear(struct link *__restrict list,
                void (*data_delete)(struct link *));

void list_insert(struct link *list, struct link *link);

void list_take(struct link *__restrict link);

void list_insert_front(struct link *__restrict list, struct link *link);

void list_insert_back(struct link *__restrict list, struct link *link);

struct link *list_front(struct link *__restrict list);

struct link *list_back(struct link *__restrict list);

struct link *list_take_front(struct link *__restrict list);

struct link *list_take_back(struct link *__restrict list);

struct link *list_at(struct link *__restrict list, unsigned int index);

struct link *list_at_reverse(struct link *__restrict list, unsigned int index);

bool list_empty(const struct link *__restrict list);

void list_merge(struct link *list, struct link *other);

#define list_for_each(list, link)                                              \
    for((link) = (list)->next; (link) != (list); (link) = (link)->next)

#define list_for_each_reverse(list, link)                                      \
    for((link) = (list)->prev; (link) != (list); (link) = (link)->prev)

#define list_for_each_safe(list, link, safe)                                   \
    for((link) = (list)->next, (safe) = (link)->next;                          \
        (link) != (list);                                                      \
        (link) = (safe), (safe) = (safe)->next)

#define list_for_each_reverse_safe(list, link, safe)                           \
    for((link) = (list)->prev, (safe) = (link)->prev;                          \
        (link) != (list);                                                      \
        (link) = (safe), (safe) = (safe)->prev)

#endif /* _LIST_H_ */