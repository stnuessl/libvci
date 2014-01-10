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