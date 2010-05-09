#ifndef GC_H
#define GC_H
// ****************************************************************************
//  gc.h                                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Garbage collector managing memory for us
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <stdint.h>
#include <typeinfo>

XL_BEGIN

// ============================================================================
//
//   Class declarations
//
// ============================================================================

struct GarbageCollector;

struct AllocatorBase
// ----------------------------------------------------------------------------
//   Structure allocating data for a single data type
// ----------------------------------------------------------------------------
{
    union Chunk
    {
        Chunk *         next;           // Next in free list
        AllocatorBase * allocator;      // Allocator for this object
        uintptr_t       bits;           // Allocation bits
    };
    typedef void (*mark_fn)(void *object);

public:
    AllocatorBase(kstring name, uint objectSize, mark_fn mark,
                  GarbageCollector *gc);
    virtual ~AllocatorBase();

    void *              Allocate();
    void                Delete(void *);
    virtual void        Finalize(void *);

    void                MarkRoots();
    void                Mark(void *inUse);
    void                Sweep();

    static AllocatorBase *ValidPointer(AllocatorBase *ptr);

    uint &              Roots(void *ptr) { return roots[ptr]; }

public:
    enum ChunkBits
    {
        PTR_MASK        = 15,           // Special bits we take out of the ptr
        USE_MASK        = 7,            // Bits used as reference counter
        ALLOCATED       = 0,            // Just allocated
        LOCKED_ROOT     = 7,            // Too many references to fit in mask
        IN_USE          = 8             // Set if already marked this time
    };

protected:
    kstring             typeName;
    GarbageCollector *  gc;
    std::vector<Chunk*> chunks;
    mark_fn             mark;
    std::map<void*,uint>roots;
    Chunk *             freeList;
    uint                chunkSize;
    uint                objectSize;
    uint                alignedSize;
    uint                available;

public:
    static void *       lowestAddress;
    static void *       highestAddress;
};


template <class Object, typename ValueType=void> struct GCPtr;


template <class Object>
struct Allocator : AllocatorBase
// ----------------------------------------------------------------------------
//    Allocate objects for a given object type
// ----------------------------------------------------------------------------
{
    typedef Object  object_t;
    typedef Object *ptr_t;

public:
    Allocator(GarbageCollector *gc = NULL);

    static Allocator *  Singleton();
    static Object *     Allocate(size_t size);
    static void         Delete(Object *);
    virtual void        Finalize(void *object);
    void                RegisterPointers();
    static void         MarkObject(void *object);
};


template<class Object, typename ValueType>
struct GCPtr
// ----------------------------------------------------------------------------
//   A root pointer to an object in a garbage-collected pool
// ----------------------------------------------------------------------------
{
    typedef AllocatorBase       Base;
    typedef Allocator<Object>   Alloc;

public:
    GCPtr(): pointer(0)                         { }
    GCPtr(Object *ptr): pointer(ptr)            { Acquire(); }
    GCPtr(Object &ptr): pointer(&ptr)           { Acquire(); }
    GCPtr(const GCPtr &ptr)
        : pointer(ptr.Pointer())                { Acquire(); }
    template<class U, typename V>
    GCPtr(const GCPtr<U,V> &p)
        : pointer((U*) p.Pointer())             { Acquire(); }
    ~GCPtr()                                    { Release(); }

    operator Object* () const                   { InUse(); return pointer; }
    const Object *ConstPointer() const          { InUse(); return pointer; }
    Object *Pointer() const                     { InUse(); return pointer; }
    Object *operator->() const                  { InUse(); return pointer; }
    Object& operator*() const                   { InUse(); return *pointer; }
    operator ValueType() const                  { return pointer; }

    GCPtr &operator= (const GCPtr &o)
    {
        if (o.ConstPointer() != pointer)
        {
            Release();
            pointer = (Object *) o.ConstPointer();
            Acquire();
        }
        return *this;
    }

    template<class U, typename V>
    GCPtr& operator=(const GCPtr<U,V> &o)
    {
        if (o.ConstPointer() != pointer)
        {
            Release();
            pointer = (U *) o.ConstPointer();
            Acquire();
        }
        return *this;
    }

#define DEFINE_CMP(CMP)                                 \
    template<class U, typename V>                       \
    bool operator CMP(const GCPtr<U,V> &o) const        \
    {                                                   \
        return pointer CMP o.ConstPointer();            \
    }                                           

    DEFINE_CMP(==)
    DEFINE_CMP(!=)
    DEFINE_CMP(<)
    DEFINE_CMP(<=)
    DEFINE_CMP(>)
    DEFINE_CMP(>=)

    void        InUse() const;

protected:
    void        Acquire() const;
    void        Release() const;

    Object *    pointer;
};


struct GarbageCollector
// ----------------------------------------------------------------------------
//    Structure registering all allocators
// ----------------------------------------------------------------------------
{
    GarbageCollector();
    ~GarbageCollector();

    void        Register(AllocatorBase *a);
    void        RunCollection(bool force=false);
    void        MustRun()    { mustRun = true; }

    static GarbageCollector *   Singleton();
    static void                 Collect(bool force=false);
    static void                 CollectionNeeded() { Singleton()->MustRun(); }

    struct Listener
    {
        virtual void BeginCollection()          {}
        virtual bool CanDelete(void *)          { return true; }
        virtual void EndCollection()            {}
    };
    void AddListener(Listener *l) { listeners.insert(l); }
    bool CanDelete(void *object);

protected:
    std::vector<AllocatorBase *> allocators;
    std::set<Listener *>         listeners;
    bool mustRun;
};



// ============================================================================
//
//    Macros used to declare a garbage-collected type
//
// ============================================================================

// Define a garbage collected tree
// Intended usage : GARBAGE_COLLECT(Tree) { /* code to mark pointers */ }

#define GARBAGE_COLLECT_MARK(type)                                      \
    void *operator new(size_t size)                                     \
    {                                                                   \
        return XL::Allocator<type>::Allocate(size);                     \
    }                                                                   \
                                                                        \
    void operator delete(void *ptr)                                     \
    {                                                                   \
        XL::Allocator<type>::Delete((type *) ptr);                      \
    }                                                                   \
                                                                        \
    void Mark(XL::Allocator<type> &alloc)

#define GARBAGE_COLLECT(type)                           \
        GARBAGE_COLLECT_MARK(type) { (void) alloc; }



// ============================================================================
//
//   Inline functions for base allocator
//
// ============================================================================

inline AllocatorBase *AllocatorBase::ValidPointer(AllocatorBase *ptr)
// ----------------------------------------------------------------------------
//   Return a valid pointer from a possibly marked pointer
// ----------------------------------------------------------------------------
{
    AllocatorBase *result = (AllocatorBase *) (((uintptr_t) ptr) & ~PTR_MASK);
    assert(result && result->gc == GarbageCollector::Singleton());
    return result;
}



// ============================================================================
//
//   Definitions for template Allocator
//
// ============================================================================

template<class Object> inline
Allocator<Object>::Allocator(GarbageCollector *gc)
// ----------------------------------------------------------------------------
//   Create an allocator for the given size
// ----------------------------------------------------------------------------
    : AllocatorBase(typeid(Object).name(), sizeof (Object), MarkObject, gc)
{}


template<class Object> inline
Allocator<Object> * Allocator<Object>::Singleton()
// ----------------------------------------------------------------------------
//    Return a singleton for the allocation class
// ----------------------------------------------------------------------------
{
    static Allocator *allocator = NULL;
    if (!allocator)
    {
        // Create the singleton
        allocator = new Allocator;

        // Register the allocator with the garbage collector
        GarbageCollector::Singleton()->Register(allocator);
    }
    return allocator;
}


template<class Object> inline
Object *Allocator<Object>::Allocate(size_t size)
// ----------------------------------------------------------------------------
//   Allocate an object (invoked by operator new)
// ----------------------------------------------------------------------------
{
    assert(size == Singleton()->AllocatorBase::objectSize);
    return (Object *) Singleton()->AllocatorBase::Allocate();
}


template<class Object> inline
void Allocator<Object>::Delete(Object *obj)
// ----------------------------------------------------------------------------
//   Allocate an object (invoked by operator new)
// ----------------------------------------------------------------------------
{
    Singleton()->AllocatorBase::Delete(obj);
}


template<class Object> inline
void Allocator<Object>::Finalize(void *obj)
// ----------------------------------------------------------------------------
//   Make sure that we properly call the destructor for the object
// ----------------------------------------------------------------------------
{
    if (gc->CanDelete(obj))
    {
        Object *object = (Object *) obj;
        delete object;
    }
}


template<class Object> inline
void Allocator<Object>::MarkObject(void *object)
// ----------------------------------------------------------------------------
//   Make sure that we properly call the destructor for the object
// ----------------------------------------------------------------------------
{
    if (object)
        ((Object *) object)->Mark(*Singleton());
}



// ============================================================================
// 
//     GCPtr inline functions
//
// ============================================================================

inline bool IsAllocated(void *ptr)
// ----------------------------------------------------------------------------
//   Tell if a pointer is allocated
// ----------------------------------------------------------------------------
{
    return (ptr >= AllocatorBase::lowestAddress &&
            ptr <= AllocatorBase::highestAddress);
}


template<class Object, typename Value> inline
void GCPtr<Object,Value>::InUse() const
// ----------------------------------------------------------------------------
//   Mark the current pointer as in use, to preserve in next GC cycle
// ----------------------------------------------------------------------------
{
    if (IsAllocated(pointer))
    {
        assert (((intptr_t) pointer & Alloc::PTR_MASK) == 0);
        Base::Chunk *chunk = ((Base::Chunk *) pointer) - 1;
        chunk->bits |= Alloc::IN_USE;
    }
}


template<class Object, typename Value> inline
void GCPtr<Object, Value>::Acquire() const
// ----------------------------------------------------------------------------
//   Increment the reference counter for the given pointer
// ----------------------------------------------------------------------------
{
    if (IsAllocated(pointer))
    {
        assert (((intptr_t) pointer & Alloc::PTR_MASK) == 0);
        Base::Chunk *chunk = ((Base::Chunk *) pointer) - 1;
        uint count = chunk->bits & Alloc::USE_MASK;
        if (count < Alloc::LOCKED_ROOT)
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | ++count;
        if (count >= Alloc::LOCKED_ROOT)
        {
            Base *allocator = Base::ValidPointer(chunk->allocator);
            count = Alloc::LOCKED_ROOT + allocator->Roots(pointer)++;
        }
    }
}


template<class Object, typename Value> inline
void GCPtr<Object, Value>::Release() const
// ----------------------------------------------------------------------------
//   Decrement the reference counter for the given pointer
// ----------------------------------------------------------------------------
{
    if (IsAllocated(pointer))
    {
        assert (((intptr_t) pointer & Alloc::PTR_MASK) == 0);
        Base::Chunk *chunk = ((Base::Chunk *) pointer) - 1;
        uint count = chunk->bits & Alloc::USE_MASK;
        Base *allocator = Base::ValidPointer(chunk->allocator);
        if (count < Alloc::LOCKED_ROOT)
        {
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | --count;
            if (!count && !(chunk->bits & Alloc::IN_USE))
                allocator->Finalize(pointer);
        }
        else if (--allocator->Roots(pointer) == 0)
        {
            chunk->bits = (chunk->bits & ~Alloc::USE_MASK) | --count;
        }
    }
}

XL_END

#endif // GC_H
