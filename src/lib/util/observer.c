#include <stdlib.h>

#include "observer.h"

void observer_set_function(struct observer *__restrict obs, 
                           void (*func)(struct observer *, void *))
{
    obs->func = func;
}

void (*observer_function(struct observer *__restrict obs))
                        (struct observer *, void *)
{
    return obs->func;
}

void observer_set_event_id(struct observer *__restrict obs, 
                           unsigned int event_id)
{
    obs->event_id = event_id;
}

unsigned int observer_event_id(const struct observer *__restrict obs)
{
    return obs->event_id;
}