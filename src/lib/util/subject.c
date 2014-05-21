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
#include <errno.h>

#include "list.h"
#include "map.h"
#include "compare.h"
#include "subject.h"
#include "observer.h"

#include "macro.h"

static unsigned int _int_hash(const void *key)
{
    return (long) key;
}

static void _observer_unlink(struct link *link)
{
    list_take(&container_of(link, struct observer, link_event)->link_all);
}

static void _list_delete(void *list)
{
    list_delete(list, &_observer_unlink);
}

struct subject *subject_new(unsigned int observers)
{
    struct subject *sub;
    int err;
    
    sub = malloc(sizeof(*sub));
    if(!sub)
        return NULL;
    
    err = subject_init(sub, observers);
    if(err < 0) {
        free(sub);
        return NULL;
    }
    
    return sub;
}

void subject_delete(struct subject *__restrict sub)
{
    subject_destroy(sub);
    free(sub);
}

int subject_init(struct subject *__restrict sub, unsigned int observers)
{
    int err;

    err = map_init(&sub->map, observers, &compare_int, &_int_hash);
    if(err < 0)
        return err;
    
    map_set_data_delete(&sub->map, &_list_delete);
    
    list_init(&sub->list);

    return 0;
}

void subject_destroy(struct subject *__restrict sub)
{
    map_destroy(&sub->map);
}

void subject_clear(struct subject *__restrict sub)
{
    map_clear(&sub->map);
}

int subject_add_observer(struct subject *__restrict sub,
                         struct observer *obs)
{
    struct link *list;
    int err;
    
    list = map_retrieve(&sub->map, (void *)(long) obs->event_id);
    if(!list) {
        /* This will not get deleted until subject_destroy() is called. */
        list = list_new();
        
        if(!list)
            return -errno;
        
        err = map_insert(&sub->map, (void *)(long) obs->event_id, list);
        if(err < 0) {
            list_delete(list, NULL);
            return err;
        }
    }
    
    list_insert_back(list, &obs->link_event);
    list_insert_back(&sub->list, &obs->link_all);
    
    return 0;
}

void subject_remove_observer(struct subject *__restrict sub,
                             struct observer *obs)
{
    struct link *list, *link, *safe;

    list = map_retrieve(&sub->map, (void *)(long) obs->event_id);
    if(!list)
        return;
    
    list_for_each_safe(list, link, safe) {
        if(container_of(link, struct observer, link_event) == obs) {
            list_take(&obs->link_event);
            list_take(&obs->link_all);
            break;
        }
    }
}

void subject_clear_event(struct subject *__restrict sub,
                         unsigned int event_id)
{
    struct link *list;
    
    list = map_take(&sub->map, (void *)(long) event_id);
    if(!list)
        return;

    list_delete(list, &_observer_unlink);
}

void subject_notify(struct subject* sub, unsigned int event_id)
{
    subject_notify_arg(sub, event_id, NULL);
}

void subject_notify_arg(struct subject *__restrict sub, 
                        unsigned int event_id, 
                        void *arg)
{
    struct link *list, *link, *safe;
    struct observer *obs;
    
    list = map_retrieve(&sub->map, (void *)(long) event_id);
    if(!list)
        return;
    
    list_for_each_safe(list, link, safe) {
        obs = container_of(link, struct observer, link_event);
        obs->func(obs, arg);
    }
}

void subject_notify_all(struct subject *__restrict sub)
{
    subject_notify_all_arg(sub, NULL);
}

void subject_notify_all_arg(struct subject *__restrict sub, void *arg)
{
    struct link *link, *safe;
    struct observer *obs;
    
    list_for_each_safe(&sub->list, link, safe) {
        obs = container_of(link, struct observer, link_all);
        obs->func(obs, arg);
    }
}