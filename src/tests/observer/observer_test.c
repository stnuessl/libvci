

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <subject.h>
#include <observer.h>
#include <clock.h>
#include <macro.h>


struct my_observer {
    int id;
    void *data;
    struct observer obs;
};

void observer_call(void *obj, void *arg)
{
    struct my_observer *obs;
    long l;
    
    obs = container_of(obj, struct my_observer, obs);
    l = (long) arg;
    
    fprintf(stdout, "Observer call by struct my_observer %d. "
                    "Event id is %u and arg is %li.\n",
                    obs->id, obs->obs.event_id, l);
}

static unsigned long incr = 0;

void observer_incr(void *obj, void *arg)
{
    incr += 1;
}

void test_performance(void)
{
#define NUM 1000000
    struct subject *sub;
    static struct my_observer obs[NUM];
    struct clock *c;
    int i, err;
    
    c = clock_new(CLOCK_PROCESS_CPUTIME_ID);
    sub = subject_new(NUM);
    assert(sub);
    assert(c);
    
    for(i = 0; i < NUM; ++i) {
        obs[i].data = NULL;
        obs[i].id = i;
        
        observer_set_event_id(&obs[i].obs, i);
        observer_set_function(&obs[i].obs, &observer_incr);
        
        err = subject_add_observer(sub, &obs[i].obs);
        assert(err == 0);
    }
    
    clock_start(c);
    
    for(i = 0; i < NUM; ++i)
        subject_notify(sub, i);
    
    clock_stop(c);
    assert(incr == NUM);
    
    fprintf(stdout, 
            "Performance test: %lu function calls in %lu us.\n",
            incr, clock_elapsed_us(c));
    
    clock_delete(c);
    subject_delete(sub);
}

int main(int argc, char *argv[])
{
#define NUM_OBSERVER 10
#define ADDITIONAL_OBSERVERS 10
    struct subject *sub;
    struct my_observer obs[NUM_OBSERVER], *obs_add[ADDITIONAL_OBSERVERS];
    int i, err;
    
    sub = subject_new(NUM_OBSERVER);
    assert(sub);
    
    for(i = 0; i < NUM_OBSERVER; ++i) {
        
        obs[i].data = NULL;
        obs[i].id = NUM_OBSERVER - i;

        observer_set_event_id(&obs[i].obs, i);
        observer_set_function(&obs[i].obs, &observer_call);
        
        err = subject_add_observer(sub, &obs[i].obs);
        assert(err == 0);
    }
    
    fprintf(stdout, "Notify half of all observers.\n");
    
    for(i = 0; i < NUM_OBSERVER / 2; ++i)
        subject_notify_arg(sub, i, (void *) 42);
    
    fprintf(stdout, "Notify all\n");
    
    subject_notify_all_arg(sub, (void *) 72);
    
    for(i = 0; i < ADDITIONAL_OBSERVERS; ++i) {
        obs_add[i] = malloc(sizeof(**obs_add));
        
        obs_add[i]->data = NULL;
        obs_add[i]->id = NUM_OBSERVER + 1 + i;
        
        observer_set_event_id(&obs_add[i]->obs, 0);
        observer_set_function(&obs_add[i]->obs, &observer_call);
        
        err = subject_add_observer(sub, &obs_add[i]->obs);
        assert(err == 0);
    }
    
    fprintf(stdout, "Notify additional events with event id 0.\n");
    subject_notify_arg(sub, 0, (void *) 77);
    
    subject_clear_event(sub, 0);
    
    fprintf(stdout, "Clearing and notifying all events with event_id 0.\n");
    subject_notify(sub, 0);
    
    fprintf(stdout, "Clearing and notifying observer %d.\n", NUM_OBSERVER / 2);
    subject_remove_observer(sub, &obs[NUM_OBSERVER / 2].obs);
    subject_notify(sub, obs[NUM_OBSERVER / 2].obs.event_id);
    
    subject_delete(sub);
    
    for(i = 0; i < ADDITIONAL_OBSERVERS; ++i)
        free(obs_add[i]);
    
    test_performance();
    
    return EXIT_SUCCESS;
}