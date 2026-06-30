#pragma once

#include <new>
#include <stdint.h>
#include <utility>

template <typename T>
class StaticObject {
  public:
    StaticObject() : object(nullptr)
    {
    }

    ~StaticObject()
    {
      reset();
    }

    StaticObject(const StaticObject &) = delete;
    StaticObject &operator=(const StaticObject &) = delete;

    template <typename... Args>
    T *create(Args &&...args)
    {
      reset();
      object = new (storage) T(std::forward<Args>(args)...);
      return object;
    }

    void reset()
    {
      if (object != nullptr) {
        object->~T();
        object = nullptr;
      }
    }

    T *get()
    {
      return object;
    }

    const T *get() const
    {
      return object;
    }

  private:
    alignas(T) uint8_t storage[sizeof(T)];
    T *object;
};
