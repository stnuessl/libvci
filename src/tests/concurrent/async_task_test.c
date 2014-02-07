#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <libvci/async_task.h>
#include <libvci/subject.h>
#include <libvci/observer.h>
#include <libvci/macro.h>

#define MAX_EPOLL_EVENTS 1000

struct object {
    struct async_task task;
    struct observer obs;
    
    int epoll_fd;
};

void observer_call(struct observer *obs, void *arg)
{
    struct object *obj;
    int ok, fd, err;
    eventfd_t n;
    
    obj = container_of(obs, struct object, obs);
    
    fd = async_task_event_fd(&obj->task);
    
    err = eventfd_read(fd, &n);
    assert(err == 0 && n == 1);
    
    err = epoll_ctl(obj->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    assert(err == 0);
    
    ok = (long) async_task_finish(&obj->task);
    assert(ok == 1);
    
    async_task_destroy(&obj->task);
    subject_remove_observer(arg, &obj->obs);
    free(obj);
}

void *async_task_do(void *arg)
{
    int fd, rand, err;
    volatile int i;
    
    fd = open(arg, O_RDONLY);
    if(fd < 0)
        return NULL;
    
    err = read(fd, &rand, sizeof(rand));
    
    rand &= 0x0000ffff;
    
    close(fd);
    
    if(err != sizeof(rand))
        return NULL;
    
    for(i = 0; i < rand; ++i)
        ;
    
    return (void *)(long) 1;
}

int main(int argc, char *argv[])
{
    struct epoll_event ev;
    static struct epoll_event epoll_events[MAX_EPOLL_EVENTS];
    struct object *obj;
    struct subject *subject;
    int fd, fds, i, epoll_fd, err;
    
    subject = subject_new(MAX_EPOLL_EVENTS);
    assert(subject);
    
    epoll_fd = epoll_create(1);
    assert(epoll_fd >= 0);
    
    /* 
     * Initialize all of the memory inside the union.
     * This surpresses a valgrind warning.
     */
    memset(&ev.data, 0, sizeof(ev.data));
    
    for(i = 0; i < MAX_EPOLL_EVENTS; ++i) {
        obj = malloc(sizeof(*obj));
        assert(obj);
        
        obj->epoll_fd = epoll_fd;
        async_task_init(&obj->task, &async_task_do);
        
        fd = async_task_event_fd(&obj->task);
        
        observer_set_function(&obj->obs, &observer_call);
        observer_set_event_id(&obj->obs, fd);
        
        err = subject_add_observer(subject, &obj->obs);
        assert(err == 0);

        ev.data.fd = fd;
        ev.events  = EPOLLIN;
        
        err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
        assert(err == 0);
        
        err = async_task_start(&obj->task, "/dev/urandom");
        assert(err == 0);
    }
    
    while(i) {
        fds = epoll_wait(epoll_fd, epoll_events, MAX_EPOLL_EVENTS, -1);
        
        i -= fds;
        
        fprintf(stdout, "%d asynchronous tasks completed.\n", fds);
        
        while(fds--) {
            fd = epoll_events[fds].data.fd;
            
            subject_notify_arg(subject, fd, subject);
        }
    }
    
    subject_delete(subject);
    close(epoll_fd);
    
    return EXIT_SUCCESS;
}