#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <string>

extern "C" int tinydomxx_service(int service_code, void *p);

namespace tinydomxx {

enum {
    ALLOC_ENTRY = 0,
    FREE_ENTRY = 1,
    DOCUMENT_CREATE_ELEMENT = 2,
    ELEMENT_SETATTRIBUTE = 3,
    ELEMENT_APPENDCHILD = 4,
    DOCUMENT_CREATE_TEXT_NODE = 5
};

inline uint32_t alloc_js_entry() {
    int ret = tinydomxx_service(ALLOC_ENTRY, nullptr);
    return ret;
}

inline int free_js_entry(uint32_t e) {
    struct {
        uint32_t e;
    }a = {e};

    return tinydomxx_service(FREE_ENTRY, &a);
}

inline uint32_t document_create_element(const char *tag) {
    struct {
        uint32_t tagptr;
        uint32_t taglen;
    }a = {(uint32_t)(uintptr_t)tag, (uint32_t)strlen(tag)};

    return tinydomxx_service(DOCUMENT_CREATE_ELEMENT, &a);
}
inline uint32_t document_create_text_node(const char *strptr) {
    struct {
        uint32_t strptr;
        uint32_t strlen;
    }a = {(uint32_t)(uintptr_t)strptr, (uint32_t)strlen(strptr)};

    return tinydomxx_service(DOCUMENT_CREATE_TEXT_NODE, &a);
}
inline void element_setattribute(uint32_t entry, const char *name, const char *value) {
    struct {
        uint32_t e;
        uint32_t nameptr;
        uint32_t namelen;
        uint32_t valueptr;
        uint32_t valuelen;
    }a = {entry, (uint32_t)(uintptr_t)name, (uint32_t)strlen(name), (uint32_t)(uintptr_t)value, (uint32_t)strlen(value)};

    tinydomxx_service(ELEMENT_SETATTRIBUTE, &a);
}

inline void element_appendchild(uint32_t entry, uint32_t child_entry) {
    struct {
        uint32_t e;
        uint32_t child_entry;
    }a = {entry, child_entry};

    tinydomxx_service(ELEMENT_APPENDCHILD, &a);
}


struct jsref_refcnt {
    uint64_t v;

    uint32_t count( ) const {
        return v&0xffffffff;
    }
    uint32_t handle() const {
        return v>>32;
    }

    uint32_t inc() {
        return (++v);
    }
    uint32_t dec() {
        return (--v);
    }

    bool is1() {
        return (v&0xffffffff) == 1;
    }

    void init(uint32_t handle) {
        v = (((uint64_t)handle) << 32) | 1;
    }

};

struct jsref {
    jsref_refcnt *refcnt;

    static jsref alloc() {
        return jsref();
    }

    ~jsref() {
        this->release();
    }
    jsref(const jsref &rhs) {
        this->copy(rhs);
    }
    jsref(jsref && rhs) {
        this->move(std::move(rhs));
    }

    jsref &operator = (const jsref &rhs) {
        if (&rhs == this) {
            return *this;
        }

        this->release();
        this->copy(rhs);
        return *this;
    }

    jsref &operator = (jsref &&rhs) {
        if (&rhs == this) {
            return *this;
        }

        this->release();
        this->move(std::move(rhs));
        return *this;
    }

    uint32_t entry() const {
        return refcnt->handle();
    }

    explicit jsref(int h) {
        this->refcnt = (jsref_refcnt*)malloc(sizeof(jsref_refcnt));
        this->refcnt->init(h);
    }

private:
    jsref() {
        uint32_t h = alloc_js_entry();
        this->refcnt = (jsref_refcnt*)malloc(sizeof(jsref_refcnt));
        this->refcnt->init(h);
    }

    void copy(const jsref &rhs) {
        if (rhs.refcnt) {
            this->refcnt = rhs.refcnt;
            this->refcnt->inc();
        } else {
            this->refcnt = nullptr;
        }
    }

    void release() {
        if (this->refcnt) {
            if (this->refcnt->is1()) {
                free_js_entry(refcnt->handle());
                free(refcnt);
            } else {
                this->refcnt->dec();
            }
        }
    }

    void move(jsref &&rhs) {
        this->refcnt = rhs.refcnt;
        rhs.refcnt = nullptr;
    }
};

struct JSObj
{
    jsref obj;

    JSObj(jsref const &o)
        :obj(o)
    {}
    JSObj(jsref &&o)
        :obj(std::move(o))
    {}

    JSObj(JSObj const &e) = default;
    JSObj(JSObj && e) = default;

    void setAttribute(std::string const &name, std::string const &value) {
        element_setattribute(obj.entry(), name.c_str(), value.c_str());
    }

    void appendChild(JSObj const &child) {
        element_appendchild(this->obj.entry(), child.obj.entry());
    }
};

struct Document {
    JSObj createElement(std::string const &tag) {
        uint32_t h = document_create_element(tag.c_str());
        return JSObj(std::move(jsref(h)));
    }

    JSObj createTextNode(std::string const &str) {
        uint32_t h = document_create_text_node(str.c_str());
        return JSObj(std::move(jsref(h)));
    }
};

extern Document document;

};

