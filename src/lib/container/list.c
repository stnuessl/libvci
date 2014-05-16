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
#include <errno.h>
#include <stdbool.h>

#include "link.h"
#include "list.h"
/*
 * Thanks to the wayland devs for this slick list implementation.
 */

struct link *list_new(void)
{
    struct link *list;
    
    list = malloc(sizeof(*list));
    if(!list)
        return NULL;
    
    list_init(list);
    
    return list;
}

void list_delete(struct link *__restrict list,
                 void (*data_delete)(struct link *))
{
    list_destroy(list, data_delete);
    free(list);
}

void list_init(struct link *__restrict list)
{
    list->prev = list;
    list->next = list;
}

void list_destroy(struct link *__restrict list,
                  void (*data_delete)(struct link *))
{
    list_clear(list, data_delete);
}

void list_clear(struct link *__restrict list,
                void (*data_delete)(struct link *))
{
    struct link *link, *next;
    
    if(!data_delete) {
        list_init(list);
        return;
    }
    
    list_for_each_safe(list, link, next)
        data_delete(link);
}

void list_insert(struct link *list, struct link *link)
{
    link->prev = list;
    link->next = list->next;

    list->next = link;
    link->next->prev = link;
}

void list_take(struct link *__restrict link)
{
    link->prev->next = link->next;
    link->next->prev = link->prev;
}

void list_insert_front(struct link *__restrict list, struct link *link)
{
    list_insert(list, link);
}

void list_insert_back(struct link *__restrict list, struct link *link)
{
    list_insert(list->prev, link);
}

struct link *list_front(struct link *__restrict list)
{
    return list->next;
}

struct link *list_back(struct link *__restrict list)
{
    return list->prev;
}

struct link *list_take_front(struct link *__restrict list)
{
    struct link *link;
    
    link = list_front(list);
    
    list_take(link);
    
    return link;
}

struct link *list_take_back(struct link *__restrict list)
{
    struct link *link;
    
    link = list_back(list);
    
    list_take(link);
    
    return link;
}

struct link *list_at(struct link *__restrict list, unsigned int index)
{
    struct link *link;

    link = list->next;
    
    while(index--)
        link = link->next;
    
    return (link != list) ? link : list->prev;
}

struct link *list_at_reverse(struct link *__restrict list, unsigned int index)
{
    struct link *link;
    
    link = list->prev;
    
    while(index--)
        link = link->prev;
    
    return (link != list) ? link : list->next;
}

bool list_empty(const struct link *__restrict list)
{
    return list->next == list;
}

void list_merge(struct link *list, struct link *other)
{
    if(list_empty(other))
        return;
    
    other->prev->next = list;
    other->next->prev = list->next;
    
    list->next->prev = other->prev;
    list->next = other->next;
    
    list_init(other);
}