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

#ifndef _ASYNC_TASK_H_
#define _ASYNC_TASK_H_

struct async_task {
    pthread_t id;
    void *(*func)(void *);

    void *arg;
    int event_fd;
};

struct async_task *async_task_new(void *(*func)(void *));

void async_task_delete(struct async_task *__restrict task);

int async_task_init(struct async_task *__restrict task, 
                    void *(*func)(void *));

void async_task_destroy(struct async_task *__restrict task);

int async_task_start(struct async_task *__restrict task, void *arg);

void *async_task_finish(const struct async_task *__restrict task);

int async_task_event_fd(const struct async_task *__restrict task);

#endif /* _ASYNC_TASK_H_ */