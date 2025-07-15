#if !defined __ALLOCATOR_HPP__INCLUDED__
#define __ALLOCATOR_HPP__INCLUDED__

#include <limits>
#include <memory>

// ----------------------------------------------------------------------------------------------------
// References
// ----------------------------------------------------------------------------------------------------
// 1. C++ Standard Allocator, An Introduction and Implementation By Lai Shiaw San Kent
// ( http://www.codeproject.com/cpp/allocator.asp )
//
// 2. Enhance your dynamic memory allocation with an undocumented MFC class By lano1106
// ( http://www.codeproject.com/cpp/CFixAlloc.asp )
//
// 3. On-heap fixed allocation outside MFC By emilio_grv
// ( http://www.codeproject.com/useritems/GE_fixalloc.asp )
//

namespace fd
{

namespace util
{

#if !defined(_MSC_VER) || _MSC_VER >= 1300
  using std::size_t;
  using std::ptrdiff_t;
#endif  // #if !defined(_MSC_VER) || _MSC_VER >= 1300

  // ====================================================================================================
  //	struct object_traits
  // ====================================================================================================

  //	Traits that describes an object T
  template<typename T>
  struct object_traits
  {
    //	convert an object_traits<T> to object_traits<U>
    template<typename U>
    struct rebind { typedef object_traits<U> other; };

    //	address
    inline static T * address(T & r) { return &r; }
    inline static T const * address(T const & r) { return &r; }

    // c'tor & d'tor
    inline static void construct(T * p, T const & t) { new(p) T(t); }
    inline static void destroy(T * p) { p->~T(); }

  };	//	end of class object_traits

  // ====================================================================================================
  //	struct std_alloc_policy
  // ====================================================================================================

  //	a standard allocation policy using the free store
  template<typename T>
  struct std_alloc_policy
  {
    //	typedefs
    typedef T value_type;
    typedef value_type * pointer;
    typedef const value_type * const_pointer;
    typedef value_type & reference;
    typedef const value_type & const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    //	convert an std_alloc_policy<T> to std_alloc_policy<U>
    template<typename U>
    struct rebind { typedef std_alloc_policy<U> other; };

    //	memory allocation
    inline static pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0)
    {
      return reinterpret_cast<pointer>(::operator new(cnt * sizeof(T)));
    }
    inline static void deallocate(pointer p, size_type)
    {
      ::operator delete(p);
    }

    //	size
    inline static size_type max_size()
    {
      return std::numeric_limits<size_type>::max() / sizeof(T);
    }

  };	// struct std_alloc_policy


  // ====================================================================================================
  //	struct fixed_alloc_policy
  // ====================================================================================================

  namespace detail
  {
  
  struct fixalloc_impl
  {
    typedef size_t size_type;

    enum
    {
      ELEM_MAX_SIZE   = 512,
      MAX_PLXSIZE     = 65536, //at least 2*ELEM_MAX_SIZE
      MIN_PLEXED_OBJ  = 4,
    };

    union element_header 
    { 
      element_header * pNext; //used to form the free list
      size_type sz; //used in allocated elements to remeber their size
    };

    struct plex_header 
    { 
      plex_header * pNext; //the plxes destruction list
    };

    //the data describing each size-dedicate allocation mamanger
    class data
    {
    public:
      element_header*  freelist; //the element free list
      size_type plexedobjs; //the plex size (in contained objects)

      //initialize the menbers
      data() :freelist(), plexedobjs(MIN_PLEXED_OBJ) {}
    };

    class plex_chain
    {
    public:
      plex_header* head;

      plex_chain() :head(0) {}

      //destroy the referred plexes as raw bytes
      ~plex_chain()
      {
        while(head)
        {
          plex_header * p = head;
          head = p->pNext;
          ::operator delete(p);
        }
      }
    };

    static plex_header *& plexes()
    {
      static plex_chain plexes; //the allocated plex-es
      return plexes.head;
    }

    //the index for the size-dedicated allocators
    static data & d(size_t sz) 
    { 
      //_ASSERT(sz <= ELEM_MAX_SIZE);
      static data s[ELEM_MAX_SIZE + 1];  //requires 12*512 bytes
      return s[sz]; 
    }

    //raw allocate "sz" bytes
    inline static void * raw_allocate(size_type sz)
    {
      if(sz > ELEM_MAX_SIZE)
      {//alocate normally on OS heap
        element_header * p = (element_header *) ::operator new(sz + sizeof(element_header));
        p->sz = sz; //record the size
        return &p[1]; //return the object raw address
      }

      data & r = d(sz); //get the size referred manager
      plex_header *& pPlexes = plexes();
      if(!r.freelist)
      {//no free list: allocate a plex and create the list
        plex_header * p = (plex_header*)::operator new (sizeof(plex_header) + r.plexedobjs * (sizeof(element_header) + sz));
        p->pNext = pPlexes; pPlexes = p; //chain the plex in the plex list (the firstly allocated plex will have no "next")
        size_type n = r.plexedobjs; //the allocated objects
        element_header * pEl = (element_header *)&p[1]; //first element is after the plex header
        r.freelist = pEl; //free list begins with the first element
        while(--n) //for each of the element in the plex
        {
          //gets the "next element"
          element_header * pN = (element_header *)((char *)&pEl[1] + sz);
          pEl->pNext = pN; //chain the elements
          pEl = pN;
        }
        pEl->pNext = 0; //no "next" after the last
        r.plexedobjs *= 2; //next allocated plex will have doubled size ...
        if(r.plexedobjs > (size_t)MAX_PLXSIZE/sz)
          r.plexedobjs = (size_t)MAX_PLXSIZE/sz; //usless it is already the maximum
      }
      element_header * p = r.freelist; //the first free element
      r.freelist = p->pNext; //remove it from the list
      char * pD = (char *)&p[1]; //the object raw address
//#ifdef _DEBUG
//      for(size_type i=0; i<sz; i++) //sign it for debug
//        pD[i] = -96; /* 0xA0 : just a signature*/
//#endif

      p->sz = sz; //mark the referred size
      return pD; //return the raw address
    }

    //return a raw item to the allocator
    inline static void raw_free(void * p)
    {
      //gets the element associated header
      element_header * pEl = &((element_header*)p)[-1];
      if(pEl->sz > ELEM_MAX_SIZE)
      {//was allocated directly on heap, just return it to the OS
        ::operator delete(pEl);
        return;
      }

      size_type sz = pEl->sz;
      //put in the size-associated free list
      pEl->pNext = d(sz).freelist; 
      d(sz).freelist = pEl;
    }

  };

  } // namespace detail

  template<typename T>
  struct fixed_alloc_policy : private detail::fixalloc_impl
  {
    typedef detail::fixalloc_impl implClass;

    //	typedefs
    typedef T value_type;
    typedef value_type * pointer;
    typedef const value_type * const_pointer;
    typedef value_type & reference;
    typedef const value_type & const_reference;
    typedef typename implClass::size_type size_type;
    typedef ptrdiff_t difference_type;

    //	convert an fixed_alloc_policy<T> to fixed_alloc_policy<U>
    template<typename U>
    struct rebind { typedef fixed_alloc_policy<U> other; };

    //	memory allocation
    inline static pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0)
    { 
      return reinterpret_cast<pointer>(implClass::raw_allocate(cnt * sizeof (T)));
    }
    inline static void deallocate(pointer p, size_type)
    {
      implClass::raw_free(p);
    }
    //	size
    inline static size_type max_size()
    {
      return std::numeric_limits<size_type>::max() / sizeof(T);
    }

  };	// struct fixed_alloc_policy

  // ====================================================================================================
  //	class allocator
  // ====================================================================================================

  namespace detail
  {

  //	Policy driven allocator object
  template<typename T, typename TAllocPolicy = std_alloc_policy<T>, typename TObjTraits = object_traits<T> >
  class allocator_impl
  {
  public : 
    typedef typename TAllocPolicy::size_type size_type;
    typedef typename TAllocPolicy::difference_type difference_type;
    typedef typename TAllocPolicy::pointer pointer;
    typedef typename TAllocPolicy::const_pointer const_pointer;
    typedef typename TAllocPolicy::reference reference;
    typedef typename TAllocPolicy::const_reference const_reference;
    typedef typename TAllocPolicy::value_type value_type;

  public : 
    //	convert an allocator_impl<T> to allocator_impl<U>
    template<typename U>
    struct rebind
    {
#if !defined(_MSC_VER) || _MSC_VER >= 1300
      typedef allocator_impl<U, typename TAllocPolicy::template rebind<U>::other, typename TObjTraits::template rebind<U>::other> other;
#else
      // VC6's rebinding is broken for policy driven allocator_impl
      typedef allocator_impl<U> other;
#endif
    };

  public : 
    allocator_impl() {}
    ~allocator_impl() {}
    template <typename U, typename P2, typename T2>
      explicit allocator_impl(allocator_impl<U, P2, T2> const & rhs) {}

    //	memory allocation
    inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer hint = 0)
    {
      return TAllocPolicy::allocate(cnt, hint);
    }
    inline void deallocate(pointer p, size_type cnt)
    {
      TAllocPolicy::deallocate(p, cnt);
    }

    // max_size
    inline size_type max_size() const
    {
      return TAllocPolicy::max_size();
    }

    // address
    inline static T * address(T & r)
    {
      return TObjTraits::address(r);
    }
    inline static T const * address(T const & r)
    {
      return TObjTraits::address(r);
    }

    // c'tor & d'tor
    inline static void construct(T * p, T const & t)
    {
      TObjTraits::construct(p, t);
    }
    inline static void destroy(T * p)
    {
      TObjTraits::destroy(p);
    }

  };	// class allocator_impl

  class allocator_void_impl
  {
  public:
    typedef void value_type;
    typedef void * pointer;
    typedef void const * const_pointer;

  };  // class allocator_void_impl

  } // namespace detail

  template<typename T>
    class fixed_allocator : public detail::allocator_impl<T, fixed_alloc_policy<T>, object_traits<T> >
  {
  public:
    template<typename U> struct rebind { typedef fixed_allocator<U> other; };
    
  };  // class fixed_allocator : public allocator_impl<T, fixed_alloc_policy<T>, object_traits<T> >

  template<>
    class fixed_allocator<void> : public detail::allocator_void_impl
  {
  public:
    template<typename U> struct rebind { typedef fixed_allocator<U> other; };

  };  // class fixed_allocator<void>

  template<typename T>
    class std_allocator : public detail::allocator_impl<T, std_alloc_policy<T>, object_traits<T> >
  {
  public:
    template<typename U> struct rebind { typedef std_allocator<U> other; };

  };  // class fixed_allocator : public allocator_impl<T, fixed_alloc_policy<T>, object_traits<T> >

  template<>
    class std_allocator<void> : public detail::allocator_void_impl
  {
  public:
    template<typename U> struct rebind { typedef std_allocator<U> other; };

  };  // class fixed_allocator<void>

}	// namespace util

} // namespace fd

#endif  // #if !defined __ALLOCATOR_HPP__INCLUDED__
