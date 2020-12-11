#include <emscripten.h>
#include "tinydomxx.hpp"

using namespace tinydomxx;

EMSCRIPTEN_KEEPALIVE
extern "C"
int test_alloc()
{

#define N 32
    jsobj *a[N];

    for (int i=0; i<N; i++) {
        a[i] = new jsobj(jsobj::alloc());
    }

    for (int i=N/2; i<N; i++) {
        delete a[i];
    }

    for (int i=0;i<N/2; i++) {
        a[i] = new jsobj(jsobj::alloc());
    }

    for (int i=0; i<N; i++) {
        delete a[i];
    }

    return 500;
}


EMSCRIPTEN_KEEPALIVE
extern "C"
void
test_arg(jsobj *a, jsobj *b)
{
    printf("%p %p\n", a, b);
}

