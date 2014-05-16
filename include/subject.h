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

#ifndef _SUBJECT_H_
#define _SUBJECT_H_

#include <stdbool.h>

#include "link.h"
#include "map.h"
#include "observer.h"

struct subject {
    struct map map;
    struct link list;
};

struct subject *subject_new(unsigned int observers);

void subject_delete(struct subject *__restrict sub);

int subject_init(struct subject *__restrict sub, unsigned int observers);

void subject_destroy(struct subject *__restrict sub);

void subject_clear(struct subject *__restrict sub);

int subject_add_observer(struct subject *__restrict sub,
                         struct observer *obs);

void subject_remove_observer(struct subject *__restrict sub,
                             struct observer *obs);

void subject_clear_event(struct subject *__restrict sub,
                         unsigned int event_id);

void subject_notify(struct subject *__restrict sub, unsigned int event_id);

void subject_notify_arg(struct subject *__restrict sub,
                        unsigned int event_id,
                        void *arg);

void subject_notify_all(struct subject *__restrict sub);

void subject_notify_all_arg(struct subject *__restrict sub, void *arg);


#endif /* _SUBJECT_H_ */