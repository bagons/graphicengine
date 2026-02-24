#include "graphicengine.hpp"

template<typename T>
geRef<T>::geRef() {
  id = -1;
  ge = nullptr;
}

template<typename T>
geRef<T>::geRef(const unsigned int _id, Engine* _ge){
  id = _id;
  ge = _ge;
}

template<typename T>
T* geRef<T>::get() {
  return static_cast<T*>(ge->get_thing(id));
};

template<typename T>
T* geRef<T>::operator->(){
  return get();
}

template<typename T>
void geRef<T>::free() {
  ge->remove_thing(id);
  id = -1;
  ge = nullptr;
}

template<typename T>
geRendRef<T>::geRendRef() {
  id = -1;
  ge = nullptr;
};

template<typename T>
geRendRef<T>::geRendRef(const int _id, Engine* _ge){
  id = _id;
  ge = _ge;
};

template<typename T>
T* geRendRef<T>::get() {
  return static_cast<T*>(ge->get_render_layer(id));
};

template<typename T>
T* geRendRef<T>::operator->() {
  return get();
};