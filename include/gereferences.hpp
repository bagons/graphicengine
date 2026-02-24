#ifndef GEREFERENCES_HPP
#define GEREFERENCES_HPP
class Engine;

template<typename T>
class geRef {
public:
    unsigned int id;
    Engine* ge;

    geRef();

    geRef(unsigned int _id, Engine* _ge);

    T* get();
    T* operator->();

    void free();
};


template<typename T>
class geRendRef {
public:
    int id;
    Engine* ge;

    geRendRef();

    geRendRef(int _id, Engine* _ge);

    T* get();
    T* operator->();
};

#endif //GEREFERENCES_HPP
