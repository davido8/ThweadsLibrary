#include <cstdio>
#include <stdint.h>

void recurse() {
    int x;
    printf("Stack at: %p\n", &x);
    recurse();
}
/*
    Each thread reserves an amount of memory for its stack. At the end of this 
    memory is a page with no permissions so that if they overflow they crash
    instead of running into someone else's stack. This tests to ensure that it
    properly crashes.
*/
void testMainOverflow() {
    recurse();
}
