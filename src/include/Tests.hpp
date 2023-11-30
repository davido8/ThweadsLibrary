
/*
    Each thread reserves an amount of memory for its stack. At the end of this 
    memory is a page with no permissions so that if they overflow they crash
    instead of running into someone else's stack. This tests to ensure that it
    properly crashes.
*/
void testMainOverflow();

/*
    Same thing but with threads.
*/
void testThreadOverflow();

/*
    Tests that setting %rip to the newly mmap'ed stack doesn't result in a
    segmentation fault. And when you don't mmap it, it does.
*/
void stackMappedCorrectly();