#ifndef VANAGANDR_ERROR_ERROR_HEADER
#define VANAGANDR_ERROR_ERROR_HEADER

// make the pointer assert instead of throw

#ifndef VANAGANDR_THROW
#   define VANAGANDR_THROW 0
#endif

#ifndef VANAGANDR_NDEBUG
#   define VANAGANDR_NDEBUG 0
#endif

#if VANAGANDR_NDEBUG
#   define VASSERT(x)
#else
#   include <cassert>
#   define VASSERT(x) assert(x)
#endif

#if VANAGANDR_THROW
#   define VTHROW(predicate, error, message) if((predicate)) throw error(message)
#elif !VANAGANDR_NDEBUG
#   define VTHROW(predicate, error, message) assert(!(predicate) && message)
#else
#   define VTHROW(predicate, error, message)
#endif

#include <memory>

#include "config.h"

namespace LIBNAMESPACE
{


template<class T>
T* clone(T* ptr)
{
    return new T(*ptr);
}

// Self managing pointer
// Original Design from 'Accelerated C++' by Andrew Koenig and Babara E. Moo
template<class T>
class Pointer
{
public:
    typedef std::size_t size_type;

    Pointer():
        _ptr(nullptr), _ref_count(new size_type(1))
    {}

    Pointer(T* ptr):
        _ptr(ptr), _ref_count(new size_type(1))
    {}

    Pointer(const Pointer& ptr):
        _ptr(ptr._ptr), _ref_count(ptr._ref_count)
    {
        ++(*_ref_count);
    }

    ~Pointer()
    {
        if (--(*_ref_count) == 0){
            delete _ptr;
            delete _ref_count;
        }
    }

    Pointer& operator=(const Pointer& rhs)
    {
        ++(*rhs._ref_count);

        if (--(*_ref_count) == 0){
                delete _ptr;
                delete _ref_count;
        }

        _ref_count = rhs._ref_count;
        _ptr = rhs._ptr;

        return *this;
    }

    // make a copy of the underlying object and return a ptr to it
    void make_unique()
    {
        VTHROW(_ptr == 0, std::runtime_error, "uninitialized pointer");

        if (*_ref_count != 1)
        {
            --*_ref_count;
            _ref_count = new size_type(1);
            _ptr = _ptr ? clone(_ptr) : 0;
        }
    }

    T& operator* () const
    {
        VTHROW(_ptr == 0, std::runtime_error, "uninitialized pointer");
        return *_ptr;
    }

    T* operator->() const
    {
        VTHROW(_ptr == 0, std::runtime_error, "uninitialized pointer");
        return _ptr;
    }

    operator bool() const
    {
        return _ptr;
    }

protected:
    T* _ptr;
    size_type* _ref_count;
};

}

#endif
