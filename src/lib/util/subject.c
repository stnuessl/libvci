#include <stdlib.h>
#include <errno.h>

#include "list.h"
#include "map.h"
#include "subject.h"
#include "observer.h"

#include "macro.h"

static unsigned int _int_hash(const void *key)
{
    return (long) key;
}

static int _int_compare(const void *a, const void *b)
{
    return (long) a - (long) b;
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

    err = map_init(&sub->map, observers, &_int_compare, &_int_hash);
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
    struct link *list;
    
    list = map_retrieve(&sub->map, (void *)(long) obs->event_id);
    if(!list)
        return;
    
    list_take(&obs->link_event);
    list_take(&obs->link_all);
    
    if(list_empty(list)) {
        map_take(&sub->map, (void *)(long) obs->event_id);
        list_delete(list, NULL);
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
    struct link *list, *link;
    struct observer *obs;
    
    list = map_retrieve(&sub->map, (void *)(long) event_id);
    if(!list)
        return;
    
    list_for_each(list, link) {
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
    struct link *link;
    struct observer *obs;
    
    list_for_each(&sub->list, link) {
        obs = container_of(link, struct observer, link_all);
        obs->func(obs, arg);
    }
}