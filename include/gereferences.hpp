#ifndef GEREFERENCES_HPP
#define GEREFERENCES_HPP
class Engine;

/// @brief A general entity reference.
template<typename T>
class geRef {
public:
    /// In engine ID
    unsigned int id;
    /// Pointer to engine object
    Engine* ge;

    /// Empty geRef constructor (id = -1)
    geRef();

    /// Creates a geRef
    geRef(unsigned int _id, Engine* _ge);

    /// Returns temporary pointer to entity, that is held inside a std::unique_ptr
    T* get();

    /// Operator substitute for get method for user friendliness
    T* operator->();

    /// Deletes the entity immediately, DO NOT DO THIS IN THE ANY ENTITY UPDATE METHOD, may lead to crashes
    /// @warning After calling this method, this reference id gets set to -1, but does not guarantee that there aren't any references that still hold the now not existing ID.
    void free();
    /// Queues the entity for deletion and is deleted in between frames. Use this in entity update method.
    /// @warning After calling this method, this reference id gets set to -1, but does not guarantee that there aren't any references that still hold the now not existing ID.
    void queue_free();
};

/// @brief A render pass reference
template<typename T>
class geRendRef {
public:
    /// ID of the render pass
    int id;
    /// Pointer to engine object
    Engine* ge;

    /// Empty ref constructor (id = -1)
    geRendRef();

    /// Creates geRendRef
    geRendRef(int _id, Engine* _ge);

    /// Returns temporary pointer the render layer, that is held inside a std::unique_ptr
    T* get();
    /// Operator substitute for get method for user friendliness
    T* operator->();
};

#endif //GEREFERENCES_HPP
