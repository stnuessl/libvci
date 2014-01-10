#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include "link.h"

struct observer {
    struct link link;
    void (*func)(void *, void *);
  
    unsigned int event_id;
};

void observer_set_function(struct observer *__restrict obs, 
                           void (*func)(void *, void *));

void (*observer_function(struct observer *__restrict obs))(void *, void *);

void observer_set_event_id(struct observer *__restrict obs, 
                           unsigned int event_id);

unsigned int observer_event_id(const struct observer *__restrict obs);


#endif /* _OBSERVER_H_ */