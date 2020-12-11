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
    NODE_APPENDCHILD = 4
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

inline void document_create_element(uint32_t entry, const char *tag) {
    struct {
        uint32_t e;
        uint32_t tagptr;
    }a = {entry, (uint32_t)(uintptr_t)tag};

    tinydomxx_service(DOCUMENT_CREATE_ELEMENT, &a);
}
inline void element_setattribute(uint32_t entry, const char *name, const char *value) {
    struct {
        uint32_t e;
        uint32_t nameptr;
        uint32_t valueptr;
    }a = {entry, (uint32_t)(uintptr_t)name, (uint32_t)(uintptr_t)value};

    tinydomxx_service(DOCUMENT_CREATE_ELEMENT, &a);
}

inline void node_appendchild(uint32_t entry, uint32_t child_entry) {
    struct {
        uint32_t e;
        uint32_t child_entry;
    }a = {entry, child_entry};

    tinydomxx_service(NODE_APPENDCHILD, &a);
}


struct jsobj_refcnt {
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

struct jsobj {
    jsobj_refcnt *refcnt;

    static jsobj alloc() {
        return jsobj();
    }

    ~jsobj() {
        this->release();
    }
    jsobj(const jsobj &rhs) {
        this->copy(rhs);
    }
    jsobj(jsobj && rhs) {
        this->move(std::move(rhs));
    }

    jsobj &operator = (const jsobj &rhs) {
        if (&rhs == this) {
            return *this;
        }

        this->release();
        this->copy(rhs);
        return *this;
    }

    jsobj &operator = (jsobj &&rhs) {
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

private:
    jsobj() {
        uint32_t h = alloc_js_entry();
        this->refcnt = (jsobj_refcnt*)malloc(sizeof(jsobj_refcnt));
        this->refcnt->init(h);
    }

    void copy(const jsobj &rhs) {
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

    void move(jsobj &&rhs) {
        this->refcnt = rhs.refcnt;
        rhs.refcnt = nullptr;
    }
};

struct HTMLElement {
    jsobj obj;

    HTMLElement(jsobj const &o)
        :obj(o)
    {}
    HTMLElement(jsobj const &&o)
        :obj(o)
    {}

    HTMLElement(HTMLElement const &e) = default;

    void setAttribute(std::string const &name, std::string const &value) {
        element_setattribute(obj.entry(), name.c_str(), value.c_str());
    }

    void appendChild(HTMLElement const &child) {
        node_appendchild(this->obj.entry(), child.obj.entry());
    }
};

struct Document {
    HTMLElement createElement(std::string const &tag) {
        auto o(jsobj::alloc());
        document_create_element(o.entry(), tag.c_str());
        return HTMLElement(std::move(o));
    }
};

};

