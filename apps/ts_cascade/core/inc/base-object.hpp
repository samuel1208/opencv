#ifndef SAK_PLATFORM_SAK_OBJECT_HPP__
#define SAK_PLATFORM_SAK_OBJECT_HPP__

#include "tmem.h"
#include <new>

class  Object
{
public:
    Object() {}
    virtual ~Object() {} // Which must be defined, otherwise delete will not triger destructor.

    /**
     * Overloading new/delete operators
     */

    /// normal
    void* operator new (std::size_t size) throw(std::bad_alloc) 
        { 
            void* p = TMemAlloc(_memHandle, size);
            return p;
        }
    void  operator delete (void* p) throw() 
        { 
            if (p) 
                TMemFree(_memHandle, p);
        }
    void* operator new (std::size_t size, const std::nothrow_t&) throw(std::bad_alloc) 
        { 
            void* p = TMemAlloc(_memHandle, size);
            return p;
        }
    void  operator delete (void* p, std::nothrow_t&) 
        { 
            if (p)
                TMemFree(_memHandle, p);
        }

    /// array
    void* operator new[]    (std::size_t size) throw(std::bad_alloc) 
        { 
            void* p = TMemAlloc(_memHandle, size);
            return p;
        }
    void  operator delete[] (void* p) throw() 
        { 
            if (p) 
                TMemFree(_memHandle, p);
        }
    void* operator new[]    (std::size_t size, std::nothrow_t&) throw(std::bad_alloc) 
        { 
            void* p = TMemAlloc(_memHandle, size);
            return p;
        }
    void  operator delete[] (void* p, std::nothrow_t&) throw() 
        { 
            if (p) 
                TMemFree(_memHandle, p);
        }

    /// placement new/delete
    // void* operator new    (std::size_t bytes, void* pool) throw (std::bad_alloc)
    //     {
    //         return TMemAlloc((THandle)pool, bytes);
    //     }
    // template <class T> void  operator delete (void* p, void* pool) throw()
    //     {
    //         ((T*)p)->~T();
    //         TMemFree((THandle)pool, p);
    //     }
    // void* operator new[]    (std::size_t bytes, void* pool) throw (std::bad_alloc)
    //     {
    //         return TMemArrayAlloc((THandle)pool, bytes);
    //     }
    // template <class T> void  operator delete[] (void* p, void* pool) throw()
    //     {
    //         T* q = (T*)p;
    //         long n = TMemArrayLength((THandle)pool, p)/sizeof(T);
    //         assert(n>=0);
    //         while (n-- > 0) {
    //             q->~T();
    //             ++q;
    //         }
    //         TMemArrayFree((THandle)pool, p);
    //     }

    /**
     * Properties
     */
    static void    setMemHandle(THandle memHandle) { _memHandle = memHandle; }
    static THandle memHandle() { return _memHandle; }

private:
    static THandle  _memHandle;
};


// /**
//  * Warpers for Object::new/delete operators.
//  */
// #  define sakNew(A_CLASS)             (new(sak::Object::memHandle()) A_CLASS)

// template <class T>
// void sakDelete(void* p)
// {
//     sak::Object::operator delete<T>(p, sak::Object::memHandle());
// }

// #  define sakNewArray(A_CLASS, size)  (new(sak::Object::memHandle()) A_CLASS[size])

// template <class T>
// void sakDeleteArray(void*p)
// {
//     sak::Object::operator delete[]<T>(p, sak::Object::memHandle());
// }

#endif//SAK_PLATFORM_SAK_OBJECT_HPP__
