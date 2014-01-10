#include <stdlib.h>
#include <errno.h>

#include "list.h"
#include "map.h"
#include "vector.h"
#include "subject.h"
#include "observer.h"

#include "macros.h"

static unsigned int _int_hash(const void *key)
{
    return (long) key;
}

static int _int_compare(const void *a, const void *b)
{
    return (long) a - (long) b;
}

static void _observer_unlink(void *obs)
{
    list_take(&((struct observer *)obs)->link);
}

static void _vector_delete(void *vec)
{
    vector_delete(vec, &_observer_unlink);
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

    err = map_init(&sub->map, observers, &_int_compare, &_int_hash);
    if(err < 0)
        return err;
    
    map_set_data_delete(&sub->map, &_vector_delete);
    
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
    struct vector *vec;
    int err;

    vec = map_retrieve(&sub->map, (void *)(long) obs->event_id);
    if(!vec) {
        vec = vector_new(0);
        
        if(!vec)
            return -errno;
        
        err = map_insert(&sub->map, (void *)(long) obs->event_id, vec);
        if(err < 0) {
            vector_delete(vec, NULL);
            return err;
        }
    }

    err = vector_insert_back(vec, obs);
    if(err < 0)
        return err;
    
    list_insert_front(&sub->list, &obs->link);
    
    return 0;
}

void subject_remove_observer(struct subject *__restrict sub,
                             struct observer *obs)
{
    struct vector *vec;
    unsigned int i;
    
    vec = map_retrieve(&sub->map, (void *)(long) obs->event_id);
    
    for(i = 0; i < vector_size(vec); ++i) {
        if(*vector_at(vec, i) == obs) {
            vector_take_at(vec, i);
            list_take(&obs->link);
            break;
        }
    }
}

void subject_clear_event(struct subject *__restrict sub,
                         unsigned int event_id)
{
    struct vector *vec;
    
    vec = map_take(&sub->map, (void *)(long) event_id);
    if(!vec)
        return;

    vector_delete(vec, &_observer_unlink);
}

void subject_notify(struct subject* sub, unsigned int event_id)
{
    subject_notify_arg(sub, event_id, NULL);
}

void subject_notify_arg(struct subject *__restrict sub, 
                        unsigned int event_id, 
                        void *arg)
{
    struct vector *vec;
    struct observer **obs;
    
    vec = map_retrieve(&sub->map, (void *)(long) event_id);
    if(!vec)
        return;
    
    vector_for_each(vec, obs)
        (*obs)->func(*obs, arg);
}

void subject_notify_all(struct subject *__restrict sub)
{
    subject_notify_all_arg(sub, NULL);
}

void subject_notify_all_arg(struct subject *__restrict sub, void *arg)
{
    struct link *link;
    struct observer *obs;
    
    list_for_each(&sub->list, link) {
        obs = container_of(link, struct observer, link);
        
        obs->func(obs, arg);
    }
}