#ifndef GEREFERENCES_HPP
#define GEREFERENCES_HPP
#include "graphicengine.hpp"

class Engine;

template<typename T>
class geRef {
public:
    unsigned int id;
    Engine* ge;

    geRef() {
        id = -1;
        ge = nullptr;
    }

    geRef(const unsigned int _id, Engine* _ge) {
        id = _id;
        ge = _ge;
    }

    T* get() {
        return static_cast<T*>(ge->get_thing(id));
    };
    T* operator->() {
        return get();
    };

    void free() {
        ge->remove_thing(id);
        id = -1;
        ge = nullptr;
    }
};


template<typename T>
class geRendRef {
public:
    int id;
    Engine* ge;

    geRendRef() {
        id = -1;
        ge = nullptr;
    };

    geRendRef(const int _id, Engine* _ge) {
        id = _id;
        ge = _ge;
    };

    T* get() {
        return static_cast<T*>(ge->get_render_layer(id));
    };
    T* operator->() {
        return get();
    };
};

#endif //GEREFERENCES_HPP
