#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include "link.h"

struct observer {
    struct link link_event;
    struct link link_all;
    
    void (*func)(struct observer *, void *);
    
    unsigned int event_id;
};

void observer_set_function(struct observer *__restrict obs, 
                           void (*func)(struct observer *, void *));

void (*observer_function(struct observer *__restrict obs))
                        (struct observer *, void *);

void observer_set_event_id(struct observer *__restrict obs, 
                           unsigned int event_id);

unsigned int observer_event_id(const struct observer *__restrict obs);


#endif /* _OBSERVER_H_ */