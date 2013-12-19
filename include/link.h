#ifndef _LINK_H_
#define _LINK_H_

struct link {
    struct link *prev;
    struct link *next;
};

#endif