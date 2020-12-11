#include <emscripten.h>
#include "tinydomxx.hpp"

using namespace tinydomxx;

EMSCRIPTEN_KEEPALIVE
extern "C"
int test_alloc()
{

#define N 32
    jsref *a[N];

    for (int i=0; i<N; i++) {
        a[i] = new jsref(jsref::alloc());
    }

    for (int i=N/2; i<N; i++) {
        delete a[i];
    }

    for (int i=N/2;i<N; i++) {
        a[i] = new jsref(jsref::alloc());
    }

    for (int i=0; i<N; i++) {
        delete a[i];
    }

    return 500;
}

JSObj *obj;

EMSCRIPTEN_KEEPALIVE
extern "C"
void
test_hold(jsref *a, jsref *b)
{
    obj = new JSObj(*a);
}

EMSCRIPTEN_KEEPALIVE
extern "C"
void
test_release()
{
    delete obj;
}

EMSCRIPTEN_KEEPALIVE
extern "C"
void
test_append_text(jsref *a)
{
    JSObj o(*a);
    JSObj text = document.createTextNode("hello");
    o.appendChild(text);
}

EMSCRIPTEN_KEEPALIVE
extern "C"
void
test_insert_link(jsref *p)
{
    JSObj o(*p);
    JSObj a = document.createElement("a");
    JSObj str = document.createTextNode("hello");

    a.setAttribute("href", "http://example.com");
    a.appendChild(str);

    o.appendChild(a);
}

