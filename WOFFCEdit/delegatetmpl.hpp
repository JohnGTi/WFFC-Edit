/** @file delegatetmpl.hpp
 *
 *  @brief  delegate implementations
 *
 *
 *  @author JaeWook Choi
 *  @version 1.10
 *
 *  @history
 *    1.10 (03.12.2006) - see history of "delegateimpl.h"
 *    1.01 (03.10.2006) - member functions are renamed and rearranged
 *    1.00 (03.01.2006) - Initial public release
 *
 *
 * This software is provided "as is" without express or implied warranty, and with
 * no claim as to its suitability for any purpose.
 *
 */

// Note: this header is a header template and must NOT have multiple-inclusion
// protection.

namespace fd
{

namespace detail
{

template<typename R FD_COMMA FD_TPARMS(N), typename Allocator, size_t t_countof_pvoid>
struct FD_JOIN(delegateImpl,N) : private detail::allocator_base<Allocator>
{
  typedef FD_JOIN(delegateImpl,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> thisClass;
  typedef detail::allocator_base<Allocator> allocBase;

  // buffer size should be bigger than or matches with sizeof(void *)
  // IOW, t_countof_pvoid template parameter should be 1 or higher
  FD_STATIC_ASSERT(t_countof_pvoid > 0);

  // DefaultVoid - a workaround for 'void' templates in VC6.
#if defined(FD_NO_VOID_RETURN)
  typedef typename detail::VoidToDefaultVoid<R>::type  result_type;
#else // #if defined(FD_NO_VOID_RETURN)
  typedef R                                   result_type;
#endif  // #if defined(FD_NO_VOID_RETURN)

  enum { size_buf = t_countof_pvoid * sizeof(void *) };

  // data members

  // ----------------------------------------------------------------------

  //
  // function pointer info.
  //
  typedef struct tag_fp_info
  {
    unsigned char sz_fp;              // byte 1 : size of the (member) function pointer
                                      //
    bool by_malloc;                   // byte 2 : whether or not the (member) function pointer is stored in the heap memory
                                      //          if true, a member function pointer is stored at *(TMFn *)(fn_ptr_)
                                      //          otherwise, a function pointer (either free function or member function)
                                      //          is stored at *((TFn *)buf_)
                                      //
    bool is_const;                    // byte 3 : cv-qualifier, whether or not the member function pointer is for const object
                                      //          ***note*** do not bind 'volatile' object with member function pointer
                                      //
    unsigned char calling_convention; // byte 4 : platform specific calling conventions, only enabled by explicit macro definition
                                      //          0 for none, 1 for FD_STDCALL_, 2 for FD_FASTCALL_, 3 for FD_CDECL_, 4 for FD_PASCAL_
  } fp_info;
  fp_info fp_info_;

  enum { fcc_none = 0, fcc_stdcall = 1, fcc_fastcall = 2, fcc_cdecl = 3, fcc_pascal = 4 };

  FD_STATIC_ASSERT(sizeof(void *) >= sizeof(fp_info));

  // ----------------------------------------------------------------------

  //
  // to create a stub for type information restoring for member function invocation
  //
  // The Impossibly Fast C++ Delegates By Sergey Ryazanov
  // ( http://codeproject.com/cpp/ImpossiblyFastCppDelegate.asp )
  //
  typedef result_type (*stub_type)(thisClass const &, void * FD_COMMA FD_TARGS(N));
  typedef result_type (*stub_const_type)(thisClass const &, void const * FD_COMMA FD_TARGS(N));

  stub_type stub_ptr_;

  // ----------------------------------------------------------------------

  // to store the untyped object for the member function pointer, object's type information
  // will be restored in stub_type procedure (stub_ptr_ ) when delegate is being invoked
  void * obj_ptr_;

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  typedef void * (*obj_clone_man_type)(thisClass &, void const *, bool);
  obj_clone_man_type obj_clone_man_ptr_;

  template<typename T, typename U>
  struct obj_clone_man_t
  {
    // if retrieve_obj_ptr_only is true, return the pointer to the existing cloned bound object only
    // otherwise,
    //   delete dg.obj_ptr_ then
    //     if untyped_obj_src is not 0, copy construct bound object (cloning) then
    //       return the pointer to the newly cloned bound object
    inline static void * typed_obj_manager_(thisClass & dg, void const * untyped_obj_src, bool retrieve_obj_ptr_only)
    {
      T * typed_obj_src =const_cast<T *>(static_cast<T const *>(untyped_obj_src)); typed_obj_src;

      if(!retrieve_obj_ptr_only)
      {
        if(dg.obj_ptr_)
        {
          T * typed_obj_this = static_cast<T *>(dg.obj_ptr_);
          typed_obj_this->~T();
          deallocate_(typed_obj_this, sizeof(T));
          dg.obj_ptr_ = 0;
        }

        if(0 != typed_obj_src)
        {
          void * pv_new = allocate_(sizeof(T));
          T * obj_new = new (pv_new) T(*typed_obj_src);

          dg.obj_ptr_ = obj_new;
        }
      }

      T * typed_obj = static_cast<T *>(dg.obj_ptr_); typed_obj;
      return typed_obj ? (void *)detail::select_obj_type_<T, U>::type::get_pointer_(*typed_obj) : 0;
    }

  };  // template<typename T, typename U> struct obj_clone_man_t

#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  // ----------------------------------------------------------------------

  //
  // to avoid heap allocation for member function pointer
  //
  // Yet Another Generalized Functors Implementation in C++ By Aleksei Trunov
  // ( http://codeproject.com/cpp/genfunctors.asp )
  //
  union
  {
    void * fn_ptr_;
    unsigned char buf_[size_buf];
  };

  // ----------------------------------------------------------------------

  // to allow '0' as an argument of copy c'tor or assignment operator to indicate
  // 'clear of delegate'
  struct clear_type { };

  // ----------------------------------------------------------------------

  //
  // allocate/deallocate
  //
  inline static void * allocate_(size_t n, void const * pInitHint = 0)
  {
    return allocBase::allocate_(n, pInitHint);
  }

  inline static void deallocate_(void * p, size_t n)
  {
    allocBase::deallocate_(p, n);
  }
  
  // ====================================================================================================
  // implementations (template meta programming)
  // ====================================================================================================

  //
  // function pointer is stored in the internal buffer (buf_)
  //
  // *** No heap allocation ****
  //
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  struct fp_by_value
  {
    // DefaultVoid - a workaround for 'void' templates in VC6.
#if defined(FD_NO_VOID_RETURN)
    typedef typename detail::VoidToDefaultVoid<UR>::type  result_type2;
#else // #if defined(FD_NO_VOID_RETURN)
    typedef UR                                            result_type2;
#endif  // #if defined(FD_NO_VOID_RETURN)

    template<typename U FD_NESTED_PARTIAL_TPARM_MAIN(0)>
    struct callee_spec_
    {
      //
      // init_()
      //
      FD_SHADOW_TEMPLATE(U)
        static void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (U::*TMFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_none;

        new (dg.buf_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (U::*TMFn)(FD_TARGS2(N)) const;

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_none;

        new (dg.buf_) TMFn(mfn);
      }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

      FD_SHADOW_TEMPLATE(U)
        static void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_stdcall;

        new (dg.buf_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N)) const;

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_stdcall;

        new (dg.buf_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

      FD_SHADOW_TEMPLATE(U)
        static void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_fastcall;

        new (dg.buf_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N)) const;

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_fastcall;

        new (dg.buf_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

      FD_SHADOW_TEMPLATE(U)
        static void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_cdecl;

        new (dg.buf_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS2(N)) const;

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_cdecl;

        new (dg.buf_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

      //
      // get_fp_() & get_const_fp_()
      //
      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 (U::* const get_fp_(thisClass const & dg, U *))(FD_TARGS2(N))
      {
        typedef result_type2 (U::*TMFn)(FD_TARGS2(N));
        return *reinterpret_cast<TMFn const *>(dg.buf_);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 (U::* const get_const_fp_(thisClass const & dg, U const *))(FD_TARGS2(N)) const
      {
        typedef result_type2 (U::*TMFn)(FD_TARGS2(N)) const;
        return *reinterpret_cast<TMFn const *>(dg.buf_);
      }

      //
      // invoke_() & invoke_const_()
      //
      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA FD_FPARMS2(N))
      {
        if(0 == obj)
        {
          // member function call on no object
          throw bad_member_function_call();
        }

        if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        else if(fcc_stdcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        else if(fcc_fastcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        else if(fcc_cdecl == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        else
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = get_fp_(dg, obj);

          return (obj->*mfn)(FD_FARGS2(N));
        }
      }

      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA FD_FPARMS2(N))
      {
        if(0 == obj)
        {
          // member function call on no object
          throw bad_member_function_call();
        }

        if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        else if(fcc_stdcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        else if(fcc_fastcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        else if(fcc_cdecl == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        else
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = get_const_fp_(dg, obj);

          return (obj->*mfn)(FD_FARGS2(N));
        }
      }

      // ====================================================================================================

      //
      // template specialization for member function adapter
      // a special provision is made for pointers to member functions.
      // a pointer to member function of the form R (X::*mf)(p1, p2, ..., pn) cv-quals
      // be adapted to a function object with the following function call operator overloads
      //
      // template<typename T>
      // R operator()(cv-quals T & x, T1 p1, T2 p2, ..., Tn pn) cv-quals
      // {
      //   return (*x).*mf(p1, p2, ..., pn);
      // }
      //
      struct mfn_adapter_
      {
        //
        // init_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (U::*TMFn)(FD_TARGS_SP2(N));

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_none;

          new (dg.buf_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (U::*TMFn)(FD_TARGS_SP2(N)) const;

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_none;

          new (dg.buf_) TMFn(mfn);
        }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N));

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_stdcall;

          new (dg.buf_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_stdcall;

          new (dg.buf_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N));

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_fastcall;

          new (dg.buf_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_fastcall;

          new (dg.buf_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N));

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_cdecl;

          new (dg.buf_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = false;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_cdecl;

          new (dg.buf_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        //
        // get_fp_() & get_const_fp_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 (U::* const get_fp_(thisClass const & dg, U *))(FD_TARGS_SP2(N))
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N));
          return *reinterpret_cast<TMFn const *>(dg.buf_);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 (U::* const get_const_fp_(thisClass const & dg, U const *))(FD_TARGS_SP2(N)) const
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N)) const;
          return *reinterpret_cast<TMFn const *>(dg.buf_);
        }

        //
        // invoke_() & invoke_const_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          if(0 == obj)
          {
            // member function call on no object
            throw bad_member_function_call();
          }

          if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

          else if(fcc_stdcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

          else if(fcc_fastcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

          else if(fcc_cdecl == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

          else
          {
            typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = get_fp_(dg, obj);

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }
        }

        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          if(0 == obj)
          {
            // member function call on no object
            throw bad_member_function_call();
          }

          if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

          else if(fcc_stdcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

          else if(fcc_fastcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

          else if(fcc_cdecl == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

          else
          {
            typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = get_const_fp_(dg, obj);

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }
        }

      };  // struct mfn_adapter_

      // ====================================================================================================

    };  // template<typename U> struct callee_spec_

    // ====================================================================================================

    //
    // template specialization for free function
    //
    template<FD_NESTED_PARTIAL_TPARM_PARTIAL(0)>
    struct callee_spec_<free_fn_tag FD_NESTED_PARTIAL_TARG(0)>
    {
      //
      // init_()
      //
      static inline void init_(thisClass & dg, UR (*fn)(FD_TARGS2(N)))
      {
        typedef UR (*TFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_none;

        new (dg.buf_) TFn(fn);
      }

#if defined(FD_FN_ENABLE_STDCALL)

      static inline void init_(thisClass & dg, UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
      {
        typedef UR (FD_STDCALL_ *TFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_stdcall;

        new (dg.buf_) TFn(fn);
      }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

      static inline void init_(thisClass & dg, UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
      {
        typedef UR (FD_FASTCALL_ *TFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_fastcall;

        new (dg.buf_) TFn(fn);
      }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

      static inline void init_(thisClass & dg, UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
      {
        typedef UR (FD_PASCAL_ *TFn)(FD_TARGS2(N));

        dg.fp_info_.sz_fp = sizeof(TFn);
        dg.fp_info_.by_malloc = false;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_pascal;

        new (dg.buf_) TFn(fn);
      }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

      //
      // get_fp_()
      //
      static inline result_type2 (* const get_fp_(thisClass const & dg))(FD_TARGS2(N))
      {
        typedef result_type2 (*TFn)(FD_TARGS2(N));
        return *reinterpret_cast<TFn const *>(dg.buf_);
      }

      //
      // invoke_()
      //
      template<typename U>
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA FD_FPARMS2(N))
      {
        FD_ASSERT(NO_OBJECT == obj);

        if(false) { }

#if defined(FD_FN_ENABLE_STDCALL)

        else if(fcc_stdcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_STDCALL_ *TFn)(FD_TARGS2(N));

          TFn const fn = reinterpret_cast<TFn const>(get_fp_(dg));

          return (*fn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

        else if(fcc_fastcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_FASTCALL_ *TFn)(FD_TARGS2(N));

          TFn const fn = reinterpret_cast<TFn const>(get_fp_(dg));

          return (*fn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

        else if(fcc_pascal == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_PASCAL_ *TFn)(FD_TARGS2(N));

          TFn const fn = reinterpret_cast<TFn const>(get_fp_(dg));

          return (*fn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

        else
        {
          typedef result_type2 (*TFn)(FD_TARGS2(N));

          TFn const fn = get_fp_(dg);

          return (*fn)(FD_FARGS2(N));
        }
      }

    };  // template<typename T FD_NESTED_PARTIAL_TPARM_MAIN(0)> struct callee_spec_

  };  // template<typename UR FD_COMMA FD_TPARMS2(N), FD_NESTED_PARTIAL_TPARM_MAIN(0)> struct fp_by_value

  // ====================================================================================================

  //
  // function pointer is stored in the heap memory allocated through ::malloc()
  // the internal buffer (buf_) is used to store the pointer to the memory allocated (to save 4 bytes)
  //
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  struct fp_by_malloc
  {
    // DefaultVoid - a workaround for 'void' templates in VC6.
#if defined(FD_NO_VOID_RETURN)
    typedef typename detail::VoidToDefaultVoid<UR>::type  result_type2;
#else // #if defined(FD_NO_VOID_RETURN)
    typedef UR                                            result_type2;
#endif  // #if defined(FD_NO_VOID_RETURN)

    template<typename U>
    struct callee_spec_
    {
      //
      // init_()
      //
      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (U::*TMFn)(FD_TARGS2(N));

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_none;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (U::*TMFn)(FD_TARGS2(N)) const;

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_none;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N));

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_stdcall;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N)) const;

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_stdcall;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N));

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_fastcall;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N)) const;

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_fastcall;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)))
      {
        typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS2(N));

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = false;
        dg.fp_info_.calling_convention = fcc_cdecl;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const)
      {
        typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS2(N)) const;

        enum { szTMFn = sizeof(TMFn) };

        dg.fp_info_.sz_fp = sizeof(TMFn);
        dg.fp_info_.by_malloc = true;
        dg.fp_info_.is_const = true;
        dg.fp_info_.calling_convention = fcc_cdecl;

        dg.fn_ptr_ = allocate_(szTMFn, 0);
        new (dg.fn_ptr_) TMFn(mfn);
      }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

      //
      // get_fp_() & get_const_fp_()
      //
      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 (U::* const get_fp_(thisClass const & dg, U *))(FD_TARGS2(N))
      {
        typedef result_type2 (U::*TMFn)(FD_TARGS2(N));
        return *reinterpret_cast<TMFn const *>(dg.fn_ptr_);
      }

      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 (U::* const get_const_fp_(thisClass const & dg, U const *))(FD_TARGS2(N)) const
      {
        typedef result_type2 (U::*TMFn)(FD_TARGS2(N)) const;
        return *reinterpret_cast<TMFn const *>(dg.fn_ptr_);
      }

      //
      // invoke_() & invoke_const_()
      //
      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA FD_FPARMS2(N))
      {
        if(0 == obj)
        {
          // member function call on no object
          throw bad_member_function_call();
        }

        if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        else if(fcc_stdcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        else if(fcc_fastcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        else if(fcc_cdecl == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        else
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS2(N));

          TMFn const mfn = get_fp_(dg, obj);

          return (obj->*mfn)(FD_FARGS2(N));
        }
      }

      FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA FD_FPARMS2(N))
      {
        if(0 == obj)
        {
          // member function call on no object
          throw bad_member_function_call();
        }

        if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        else if(fcc_stdcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        else if(fcc_fastcall == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        else if(fcc_cdecl == dg.fp_info_.calling_convention)
        {
          typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

          return (obj->*mfn)(FD_FARGS2(N));
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        else
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS2(N)) const;

          TMFn const mfn = get_const_fp_(dg, obj);

          return (obj->*mfn)(FD_FARGS2(N));
        }
      }

      // ====================================================================================================

      struct mfn_adapter_
      {
        //
        // init_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (U::*TMFn)(FD_TARGS_SP2(N));

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_none;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (U::*TMFn)(FD_TARGS_SP2(N)) const;

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_none;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N));

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_stdcall;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_stdcall;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N));

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_fastcall;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_fastcall;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N));

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = false;
          dg.fp_info_.calling_convention = fcc_cdecl;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          typedef UR (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

          enum { szTMFn = sizeof(TMFn) };

          dg.fp_info_.sz_fp = sizeof(TMFn);
          dg.fp_info_.by_malloc = true;
          dg.fp_info_.is_const = true;
          dg.fp_info_.calling_convention = fcc_cdecl;

          dg.fn_ptr_ = allocate_(szTMFn, 0);
          new (dg.fn_ptr_) TMFn(mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        //
        // get_fp_() & get_const_fp_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 (U::* const get_fp_(thisClass const & dg, U *))(FD_TARGS_SP2(N))
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N));
          return *reinterpret_cast<TMFn const *>(dg.fn_ptr_);
        }

        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 (U::* const get_const_fp_(thisClass const & dg, U const *))(FD_TARGS_SP2(N)) const
        {
          typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N)) const;
          return *reinterpret_cast<TMFn const *>(dg.fn_ptr_);
        }

        //
        // invoke_() & invoke_const_()
        //
        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          if(0 == obj)
          {
            // member function call on no object
            throw bad_member_function_call();
          }

          if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

          else if(fcc_stdcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

          else if(fcc_fastcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

          else if(fcc_cdecl == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = reinterpret_cast<TMFn const>(get_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

          else
          {
            typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N));

            TMFn const mfn = get_fp_(dg, obj);

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }
        }

        FD_SHADOW_TEMPLATE(U)
          static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          if(0 == obj)
          {
            // member function call on no object
            throw bad_member_function_call();
          }

          if(false) { }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

          else if(fcc_stdcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_STDCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

          else if(fcc_fastcall == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_FASTCALL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

          else if(fcc_cdecl == dg.fp_info_.calling_convention)
          {
            typedef result_type2 (FD_CDECL_ U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = reinterpret_cast<TMFn const>(get_const_fp_(dg, obj));

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

          else
          {
            typedef result_type2 (U::*TMFn)(FD_TARGS_SP2(N)) const;

            TMFn const mfn = get_const_fp_(dg, obj);

            return (obj->*mfn)(FD_FARGS_SP2(N));
          }
        }

      };  // struct mfn_adapter_

      // ====================================================================================================

    };  // template<typename T> struct callee_spec_

  };  // template<typename UR FD_COMMA FD_TPARMS2(N)> // struct fp_by_malloc


  // ====================================================================================================

  //
  // select a proper function pointer info. according to function pointer type
  //
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  struct select_fp_
  {
    template<typename U, bool t_binding FD_NESTED_PARTIAL_TPARM_MAIN(0)>
    struct callee_bind_spec_
    {
      template<bool t_binding_inner FD_NESTED_PARTIAL_TPARM_MAIN(1)> struct bind_spec_;

      // ====================================================================================================

      template<FD_NESTED_PARTIAL_TPARM_PARTIAL(1)>
      struct bind_spec_<false FD_NESTED_PARTIAL_TARG(1)>
      {
        struct normal_
        {
          // if the size of the member function is smaller than buffer size use buffer to store it,
          // otherwise use ::malloc to store it on the heap
          enum { Condition = sizeof(UR (U::*)(FD_TARGS2(N))) <= size_buf };

          typedef typename fp_by_value<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>   Then;
          typedef typename fp_by_malloc<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>  Else;

          typedef typename util::If< Condition, Then, Else >::Result type;

        };  // struct normal_

#if N > 0

        struct mfn_adapter_
        {
          // typename T1 matches with typename U *
          // if the size of the member function is smaller than buffer size use buffer to store it,
          // otherwise use ::malloc to store it on the heap
          enum { Condition = sizeof(UR (U::*)(FD_TARGS_SP2(N))) <= size_buf };

          typedef typename fp_by_value<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>::mfn_adapter_   Then;
          typedef typename fp_by_malloc<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>::mfn_adapter_  Else;

          typedef typename util::If< Condition, Then, Else >::Result type;

        };  // struct mfn_adapter_

        // if the typename 'T1' matches with typename 'U *', 'U const *', 'U &' or
        // 'U const &', treat it as a member function adapter
        enum { Condition = util::Is_pointer_or_reference<U, T1>::value || util::Is_pointer_or_reference<U const, T1>::value };

        typedef typename mfn_adapter_::type   Then;
        typedef typename normal_::type        Else;

        typedef typename util::If< Condition, Then, Else >::Result type;

#else // #if N > 0

        typedef typename normal_::type type;

#endif  // #if N > 0

      };

      // ====================================================================================================

      template<FD_NESTED_PARTIAL_TPARM_PARTIAL(1)>
      struct bind_spec_<true FD_NESTED_PARTIAL_TARG(1)>
      {
        // if the size of the member function is smaller than buffer size use buffer to store it,
        // otherwise use ::malloc to store it on the heap
        enum { Condition = sizeof(UR (U::*)(FD_TARGS2(N))) <= size_buf };

        typedef typename fp_by_value<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>   Then;
        typedef typename fp_by_malloc<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<U>  Else;

        typedef typename util::If< Condition, Then, Else >::Result type;

      };

      // ====================================================================================================

      typedef typename bind_spec_<t_binding>::type type;

    };  // template<typename T> struct callee_bind_spec_

    //
    // template specialization for free function
    //
    template<FD_NESTED_PARTIAL_TPARM_PARTIAL(0)>
    struct callee_bind_spec_<free_fn_tag, false FD_NESTED_PARTIAL_TARG(0)>
    {
      typedef typename fp_by_value<UR FD_COMMA FD_TARGS2(N)>::template callee_spec_<free_fn_tag> type;

    };  // template<> struct callee_bind_spec_<free_fn_tag>

  };  // template<typename UR FD_COMMA FD_TPARMS2(N)> struct select_fp_

  // ====================================================================================================

  // ====================================================================================================

  //
  // function pointer selector
  //
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  struct fp_selector_
  {
    // DefaultVoid - a workaround for 'void' templates in VC6.
#if defined(FD_NO_VOID_RETURN)
    typedef typename detail::VoidToDefaultVoid<UR>::type  result_type2;
#else // #if defined(FD_NO_VOID_RETURN)
    typedef UR                                            result_type2;
#endif  // #if defined(FD_NO_VOID_RETURN)

    template<typename U, bool t_binding FD_NESTED_PARTIAL_TPARM_MAIN(0)>
    struct callee_bind_spec_
    {
      template<bool t_binding_inner FD_NESTED_PARTIAL_TPARM_MAIN(1)> struct bind_spec_;

      // ====================================================================================================

      template<FD_NESTED_PARTIAL_TPARM_PARTIAL(1)>
      struct bind_spec_<true FD_NESTED_PARTIAL_TARG(1)>
      {
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA FD_FPARMS2(N))
        {
          return select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::invoke_(dg, obj FD_COMMA FD_FARGS2(N));
        }
        FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA FD_FPARMS2(N))
        {
          return select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::invoke_const_(dg, obj FD_COMMA FD_FARGS2(N));
        }

      };  // template<> struct bind_spec_<true>

      // ====================================================================================================

      template<FD_NESTED_PARTIAL_TPARM_PARTIAL(1)>
      struct bind_spec_<false FD_NESTED_PARTIAL_TARG(1)>
      {
        //
        // specialization for member function call adapter
        //
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_STDCALL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_FASTCALL_ U::*mfn)(FD_TARGS_SP(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)))
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }
        FD_SHADOW_TEMPLATE(U)
        static inline void init_(thisClass & dg, UR (FD_CDECL_ U::*mfn)(FD_TARGS_SP2(N)) const)
        {
          select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::init_(dg, mfn);
        }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

        FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          return select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_(dg, obj FD_COMMA_SP FD_FARGS_SP2(N));
        }
        FD_SHADOW_TEMPLATE(U)
        static inline result_type2 invoke_const_(thisClass const & dg, U const * obj FD_COMMA_SP FD_FPARMS_SP2(N))
        {
          return select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_const_(dg, obj FD_COMMA_SP FD_FARGS_SP2(N));
        }

      };  // template<> struct bind_spec_<false>

      // ====================================================================================================

      typedef bind_spec_<t_binding> type;

    };  // template<typename T> struct callee_bind_spec_

    // ====================================================================================================

    //
    // template specialization for free function
    //
    template<FD_NESTED_PARTIAL_TPARM_PARTIAL(0)>
    struct callee_bind_spec_<free_fn_tag, false FD_NESTED_PARTIAL_TARG(0)>
    {
      static inline void init_(thisClass & dg, UR (*fn)(FD_TARGS2(N)))
      {
        select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, false>::type::init_(dg, fn);
      }

#if defined(FD_FN_ENABLE_STDCALL)

      static inline void init_(thisClass & dg, UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
      {
        select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, false>::type::init_(dg, fn);
      }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

      static inline void init_(thisClass & dg, UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
      {
        select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, false>::type::init_(dg, fn);
      }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

      static inline void init_(thisClass & dg, UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
      {
        select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, false>::type::init_(dg, fn);
      }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

      template<typename U>
        static inline result_type2 invoke_(thisClass const & dg, U * obj FD_COMMA FD_FPARMS2(N))
      {
        return select_fp_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, false>::type::invoke_(dg, obj FD_COMMA FD_FARGS2(N));
      }

      typedef callee_bind_spec_<free_fn_tag, false> type;

    };  // template<> struct callee_bind_spec_<free_fn_tag>

  };  // template<typename UR FD_COMMA FD_TPARMS2(N)> struct fp_selector_

  // ====================================================================================================

  //
  // a stub for type information restoring for member function invocation
  //
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  struct delegate_stub_t
  {
    // DefaultVoid - a workaround for 'void' templates in VC6.
#if defined(FD_NO_VOID_RETURN)
    typedef typename detail::VoidToDefaultVoid<UR>::type  result_type2;
#else // #if defined(FD_NO_VOID_RETURN)
    typedef UR                                            result_type2;
#endif  // #if defined(FD_NO_VOID_RETURN)

    template<typename U, bool t_binding>
    struct callee_bind_spec_
    {
      // ====================================================================================================
      //
      // RESTORE TYPE (the core of the entire delegate implementation)
      //
      // 1. the stored untyped object (callee) restores its type back here
      //
      // 2. the relaxed type relevant error/warning will occur here (when FD_ENABLE_TYPE_RELAXATION macro is defined)
      //
      //    ex) error C2664: 'delegate_stub_t<>::XXX::invoke_' : cannot convert parameter 3 from 'int' to 'char *' (MSVC)
      //    ex) warning C4244: 'argument' : conversion from 'int' to 'float', possible loss of data (MSVC)
      //    ex) 1760 delegatetmpl.hpp invalid conversion from `int' to `char*' (g++)
      //
      // ====================================================================================================
      static inline result_type typed_invoker_(thisClass const & dg, void * untyped_obj FD_COMMA FD_FPARMS(N))
      {
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
        void * untyped_obj_retrieved = dg.obj_clone_man_ptr_
          ? (*dg.obj_clone_man_ptr_)(const_cast<thisClass &>(dg), 0, true)
          : untyped_obj;
        U * typed_obj = static_cast<U *>(untyped_obj_retrieved); typed_obj;
#else
        U * typed_obj = static_cast<U *>(untyped_obj); typed_obj;
#endif

        // if FD_ENABLE_TYPE_RELAXATION is defined, all warnings/errors message will be issued here
        // to indicate that type can be trivially converted by the compiler
        return select_stub_::type::invoke_(dg, typed_obj FD_COMMA FD_FARGS(N));
      }

      static inline result_type typed_invoker_const_(thisClass const & dg, void const * untyped_const_obj FD_COMMA FD_FPARMS(N))
      {
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
        void const * untyped_const_obj_retrieved = dg.obj_clone_man_ptr_
          ? const_cast<void const *>((*dg.obj_clone_man_ptr_)(const_cast<thisClass &>(dg), 0, true))
          : untyped_const_obj;
        U const * typed_const_obj = static_cast<U const *>(untyped_const_obj_retrieved); typed_const_obj;
#else
        U const * typed_const_obj = static_cast<U const *>(untyped_const_obj); typed_const_obj;
#endif

        // if FD_ENABLE_TYPE_RELAXATION is defined, all warnings/errors message will be issued here
        // to indicate that type can be trivially converted by the compiler
        return select_stub_::type::invoke_const_(dg, typed_const_obj FD_COMMA FD_FARGS(N));
      }
      // ====================================================================================================
      //
      // ====================================================================================================

      struct select_stub_
      {
        template<bool t_binding_inner FD_NESTED_PARTIAL_TPARM_MAIN(0)> struct bind_spec_;

        // ====================================================================================================

        template<FD_NESTED_PARTIAL_TPARM_PARTIAL(0)>
        struct bind_spec_<false FD_NESTED_PARTIAL_TARG(0)>
        {
          static inline result_type2 invoke_(thisClass const & dg, U * typed_obj FD_COMMA FD_FPARMS2(N))
          {
            return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_(dg, typed_obj FD_COMMA FD_FARGS2(N));
          }

          static inline result_type2 invoke_const_(thisClass const & dg, U const * typed_obj FD_COMMA FD_FPARMS2(N))
          {
            return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_const_(dg, typed_obj FD_COMMA FD_FARGS2(N));
          }

          // ====================================================================================================

#if N > 0

          //
          // specialization for member function call adapter
          //
          struct mfn_ptr_adapter_
          {
            // typename T1 matches with typename U *
            static inline result_type2 invoke_(thisClass const & dg, U * /*typed_obj*/FD_COMMA FD_FPARMS2(N))
            {
              U * typed_obj = q1;
              return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_(dg, typed_obj FD_COMMA_SP FD_FARGS_SP2(N));
            }

            // typename T1 matches with typename U const *
            static inline result_type2 invoke_const_(thisClass const & dg, U const * /*typed_obj*/FD_COMMA FD_FPARMS2(N))
            {
              U const * typed_obj = q1;
              return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_const_(dg, typed_obj FD_COMMA_SP FD_FARGS_SP2(N));
            }

          };  // struct mfn_ptr_adapter_

          struct mfn_ref_adapter_
          {
            // typename T1 matches with typename U &
            static inline result_type2 invoke_(thisClass const & dg, U * /*typed_obj*/FD_COMMA FD_FPARMS2(N))
            {
              U * typed_obj = &q1;
              return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_(dg, typed_obj FD_COMMA_SP FD_FARGS_SP2(N));
            }

            // typename T1 matches with typename U const &
            static inline result_type2 invoke_const_(thisClass const & dg, U const * /*typed_obj*/FD_COMMA FD_FPARMS2(N))
            {
              U const * typed_obj = &q1;
              return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, false>::type::invoke_const_(dg, typed_obj FD_COMMA_SP FD_FARGS_SP2(N));
            }

          };  // struct mfn_ref_adapter_

          // if the typename 'T1' matches with typename 'U *', 'U const *', 'U &' or 'U const &',
          // treat it as a member function adapter
          enum {
            Condition1 = util::Is_pointer<U, T1>::value || util::Is_pointer<U const, T1>::value,
            Condition2 = util::Is_reference<U, T1>::value || util::Is_reference<U const, T1>::value
          };

          typedef mfn_ptr_adapter_       Then1;
          typedef bind_spec_<false>      Else1;

          typedef mfn_ref_adapter_       Then2;
          typedef typename util::If< Condition1, Then1, Else1 >::Result Else2;

          typedef typename util::If< Condition2, Then2, Else2 >::Result type;

#else // #if N > 0

          typedef bind_spec_<false> type;

#endif  // #if N > 0

          // ====================================================================================================

        };  // template<bool t_binding> struct bind_spec_

        // ====================================================================================================

        template<FD_NESTED_PARTIAL_TPARM_PARTIAL(0)>
        struct bind_spec_<true FD_NESTED_PARTIAL_TARG(0)>
        {
          static inline result_type2 invoke_(thisClass const & dg, U * typed_obj FD_COMMA FD_FPARMS2(N))
          {
            return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::invoke_(dg, typed_obj FD_COMMA FD_FARGS2(N));
          }

          static inline result_type2 invoke_const_(thisClass const & dg, U const * typed_obj FD_COMMA FD_FPARMS2(N))
          {
            return fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, true>::type::invoke_const_(dg, typed_obj FD_COMMA FD_FARGS2(N));
          }

        }; // template<> struct bind_spec_<true>

        // ====================================================================================================

        typedef bind_spec_<true>                  Then;
        typedef typename bind_spec_<false>::type  Else;

        typedef typename util::If< t_binding, Then, Else >::Result type;

      };  // struct select_stub_

    };  // template<typename U, bool t_binding> struct callee_bind_spec_

  };  // template<typename UR FD_COMMA FD_TPARMS2(N)> struct delegate_stub_t

  // ====================================================================================================
  // c'tors and d'tor
  // ====================================================================================================

  //
  // default c'tor
  //
  FD_JOIN(delegateImpl,N)() : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer
  }

  //
  // copy c'tor
  //
  FD_JOIN(delegateImpl,N)(thisClass const & other)
    : fp_info_(other.fp_info_), stub_ptr_(other.stub_ptr_), obj_ptr_(other.obj_ptr_)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(other.obj_clone_man_ptr_)
#endif
  {
    ::memset(buf_, 0, size_buf);  // zero buffer

    if(!other.empty_())
    {
      // to copy the function pointer from other

      if(other.fp_info_.by_malloc)
      {
        // function pointer is stored in the heap memory, so allocate a heap memory and deep copy
        fn_ptr_ = allocate_(other.fp_info_.sz_fp, 0);

        // copy function pointer
        ::memcpy(fn_ptr_, other.fn_ptr_, other.fp_info_.sz_fp);
      }
      else
      {
        // direct copy from other.buf_ to buf_
        ::memcpy(buf_, other.buf_, size_buf);
      }
    }
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    if(obj_clone_man_ptr_)
    {
      obj_ptr_ = 0;
      (*obj_clone_man_ptr_)(*this, other.obj_ptr_, false);
    }
#else
    obj_ptr_ = other.obj_ptr_;
#endif

  }

  //
  // copy c'tor with member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_TARGS2(N)), T & obj)
    : stub_ptr_(0), obj_ptr_(0)
    , obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_TARGS2(N)) const, T const & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_TARGS2(N)), T * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_TARGS2(N)) const, T const * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // copy c'tor with member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // copy c'tor with member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // copy c'tor with member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj)
    : stub_ptr_(0), obj_ptr_(0), obj_clone_man_ptr_(0)
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_cloned_(mfn, obj, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }


  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    bind_obj_ptr_(mfn, obj, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  // copy c'tor
  template<typename UR FD_COMMA FD_TPARMS2(N)>
  FD_JOIN(delegateImpl,N)(UR (*fn)(FD_TARGS2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    typedef UR (*TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    assign_(fn, false);
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
  FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    typedef UR (FD_STDCALL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    assign_(fn, false);
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
  FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    typedef UR (FD_FASTCALL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    assign_(fn, false);
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
  FD_JOIN(delegateImpl,N)(UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    typedef UR (FD_PASCAL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    assign_(fn, false);
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

#if N > 0

  // copy c'tor
  // specialization for member function call adapter
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_FPARMS_SP2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (U::*mfn)(FD_FPARMS_SP2(N)) const)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

  // specialization for member function call adapter
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

  // specialization for member function call adapter
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)))
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

  // specialization for member function call adapter
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegateImpl,N)(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
    : stub_ptr_(0), obj_ptr_(0)
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    , obj_clone_man_ptr_(0)
#endif
  {
    ::memset(&fp_info_, 0, sizeof(fp_info));
    ::memset(buf_, 0, size_buf);  // zero buffer

    assign_(mfn, false);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

  // d'tor
  ~FD_JOIN(delegateImpl,N)()
  {
    free_fp_malloc_();

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    if(obj_clone_man_ptr_)
    {
      FD_ASSERT(obj_ptr_);
      (*obj_clone_man_ptr_)(*this, 0, false);
    }
#endif

  }

  // ====================================================================================================
  // operations
  // ====================================================================================================

  //
  // swap_()
  //
  inline void swap_(thisClass & other)
  {
    std::swap(fp_info_, other.fp_info_);
    std::swap(stub_ptr_, other.stub_ptr_);
    std::swap(obj_ptr_, other.obj_ptr_);
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
    std::swap(obj_clone_man_ptr_, other.obj_clone_man_ptr_);
#endif

    unsigned char buf_temp[size_buf];
    ::memcpy(buf_temp,    buf_,       size_buf);
    ::memcpy(buf_,        other.buf_, size_buf);
    ::memcpy(other.buf_,  buf_temp,   size_buf);
  }

  // ====================================================================================================

  //
  // reset_()
  //
  inline void reset_()
  {
    thisClass().swap_(*this);
  }

  // ====================================================================================================

  //
  // empty_()
  //
  inline bool empty_() const
  {
    return 0 == fp_info_.sz_fp;
  }

  // ====================================================================================================

  //
  // assign_()
  //
  inline thisClass & assign_(thisClass const & other)
  {
    if(&other == this)
      return *this;

    thisClass(other).swap_(*this);

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & assign_(UR (*fn)(FD_TARGS2(N)), bool reset = true)
  {
    typedef UR (*TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::init_(*this, fn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::typed_invoker_;

    obj_ptr_ = NO_OBJECT;

    return *this;
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & assign_(UR (FD_STDCALL_ *fn)(FD_TARGS2(N)), bool reset = true)
  {
    typedef UR (FD_STDCALL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::init_(*this, fn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::typed_invoker_;

    obj_ptr_ = NO_OBJECT;

    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & assign_(UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)), bool reset = true)
  {
    typedef UR (FD_FASTCALL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::init_(*this, fn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::typed_invoker_;

    obj_ptr_ = NO_OBJECT;

    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & assign_(UR (FD_PASCAL_ *fn)(FD_TARGS2(N)), bool reset = true)
  {
    typedef UR (FD_PASCAL_ *TFn)(FD_TARGS2(N));
    // size of free function pointer is smaller than size_buf
    FD_STATIC_ASSERT(size_buf >= sizeof(TFn));

    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::init_(*this, fn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::typed_invoker_;

    obj_ptr_ = NO_OBJECT;

    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)


#if N > 0

  // specialization for member function call adapter
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (U::*mfn)(FD_FPARMS_SP2(N)), bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    return *this;
  }

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (U::*mfn)(FD_FPARMS_SP2(N)) const, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)), bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    return *this;
  }

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)), bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    return *this;
  }

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR, T1 FD_COMMA_SP  FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)), bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    return *this;
  }

  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & assign_(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = false };

    fp_selector_<UR, T1 FD_COMMA_SP FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR, T1 FD_COMMA_SP  FD_TARGS_SP2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

  // ====================================================================================================

  //
  // functor assignment
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename F>
    inline thisClass & assign_ftor_cloned_ (F & functor, bool reset = true)
  {
    bind_obj_cloned_(&F::operator(), functor, reset);

    return *this;
  }
#endif

  template<typename F>
    inline thisClass & assign_ftor_ptr_(F * functor, bool reset = true)
  {
    bind_obj_ptr_(&F::operator(), functor, reset);

    return *this;
  }

  // ====================================================================================================

  //
  // bind_obj_ptr_() & bind_obj_cloned_()
  //

  //
  // member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (U::*mfn)(FD_TARGS2(N)), T & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(obj);
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (U::*mfn)(FD_TARGS2(N)) const, T const & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(const_cast<T &>(obj));
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (U::*mfn)(FD_TARGS2(N)), T * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = obj;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (U::*mfn)(FD_TARGS2(N)) const, T const * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = const_cast<T *>(obj);

    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(obj);
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(const_cast<T &>(obj));
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = obj;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = const_cast<T *>(obj);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(obj);
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(const_cast<T &>(obj));
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = obj;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = const_cast<T *>(obj);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(obj);
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_cloned_(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T const & obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = allocate_(sizeof(T));
    new (obj_ptr_) T(const_cast<T &>(obj));
    obj_clone_man_ptr_ = &obj_clone_man_t<T, U>::typed_obj_manager_;

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = &delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_;

    obj_ptr_ = obj;

    return *this;
  }

  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind_obj_ptr_(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T const * obj, bool reset = true)
  {
    if(reset) reset_();

    enum { bind_spec = true };

    fp_selector_<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::type::init_(*this, mfn);
    stub_ptr_ = reinterpret_cast<stub_type>(&delegate_stub_t<UR FD_COMMA FD_TARGS2(N)>::template callee_bind_spec_<U, bind_spec>::typed_invoker_const_);

    obj_ptr_ = const_cast<T *>(obj);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // function call operator
  //
  inline result_type operator () (FD_FPARMS(N)) const
  {
    if(empty_())
      throw bad_function_call();

    if(NO_OBJECT == obj_ptr_)
    {
      enum { bind_spec = false };

      // free function call
#if defined(FD_DISABLE_TYPE_RELAXATION)
      // All information is available and none need to be restored from stub, so take the shortcut.
      return fp_selector_<R FD_COMMA FD_TARGS(N)>::template callee_bind_spec_<free_fn_tag, bind_spec>::invoke_(*this, NO_OBJECT FD_COMMA FD_FARGS(N));
#else
      return (*stub_ptr_)(*this, NO_OBJECT FD_COMMA FD_FARGS(N));
#endif  // #if defined(FD_DISABLE_TYPE_RELAXATION)
    }
    else
    {
      if(!fp_info_.is_const)
      {
        // bound member function call or mfn_adapter_ call
        return (*stub_ptr_)(*this, obj_ptr_ FD_COMMA FD_FARGS(N));
      }
      else
      {
        // bound const member function call or const mfn_adapter_ call
        stub_const_type stub_const_ptr = reinterpret_cast<stub_const_type>(stub_ptr_);
        void const * obj_const_ptr = const_cast<void const *>(obj_ptr_);
        return (*stub_const_ptr)(*this, obj_const_ptr FD_COMMA FD_FARGS(N));
      }
    }
  }

  // ====================================================================================================

#if defined(FD_COPARE_WITH_ENDIANESS_CORRECT)

  // to compare two function pointer regardless of their binary endianess
  inline int fpcmp_(void const * fp1, void const * fp2, size_t count) const
  {
    enum { sizeof_pvoid = sizeof(void *) };
    unsigned char * p1 = reinterpret_cast<unsigned char *>(const_cast<void *>(fp1));
    unsigned char * p2 = reinterpret_cast<unsigned char *>(const_cast<void *>(fp2));

    unsigned char const * p1s = p1;

    for( ; (p1 - p1s) <= (count - sizeof_pvoid); p1 += sizeof_pvoid, p2 += sizeof_pvoid )
    {
      void * pv1 = *reinterpret_cast<void **>(p1);
      void * pv2 = *reinterpret_cast<void **>(p2);

      if(pv1 > pv2)
        return 1;
      else if(pv1 < pv2)
        return -1;
    }

    FD_ASSERT( 0 == (count - (p1 - p1s)) );

    return 0;
  }

#else // #if defined(FD_COPARE_WITH_ENDIANESS_CORRECT)

  inline int fpcmp_(void const * fp1, void const * fp2, size_t count) const
  {
    return ::memcmp(fp1, fp2, count);
  }

#endif  // #if defined(FD_COPARE_WITH_ENDIANESS_CORRECT)

  //
  // compare_()
  //
  inline int compare_(thisClass const & other, bool check_bound_object = false) const
  {
    if(fp_info_.sz_fp == other.fp_info_.sz_fp)
    {
      if(0 == fp_info_.sz_fp)
        return 0;  // empty delegates

      void const * this_fp = fp_info_.by_malloc ? fn_ptr_ : reinterpret_cast<void const *>(buf_);
      void const * other_fp = other.fp_info_.by_malloc ? other.fn_ptr_ : reinterpret_cast<void const *>(other.buf_);

      int cmp_result = fpcmp_(this_fp, other_fp, fp_info_.sz_fp);
      if(!check_bound_object || 0 != cmp_result)
        return cmp_result;

      // function pointers are the same and check_bound_object is set as true
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
      void * this_obj_ptr = obj_clone_man_ptr_ ? (*obj_clone_man_ptr_)(const_cast<thisClass &>(*this), 0, true) : obj_ptr_;
      void * other_obj_ptr = other.obj_clone_man_ptr_ ? (*other.obj_clone_man_ptr_)(const_cast<thisClass &>(other), 0, true) : other.obj_ptr_;
#else
      void * this_obj_ptr = obj_ptr_;
      void * other_obj_ptr = other.obj_ptr_;
#endif
      return (reinterpret_cast<void **>(this_obj_ptr) - reinterpret_cast<void **>(other_obj_ptr));
    }
    else
    {
      if(0 == fp_info_.sz_fp)
        return -1;

      if(0 == other.fp_info_.sz_fp)
        return 1;

      size_t min_sz_fp = (fp_info_.sz_fp <= other.fp_info_.sz_fp)
        ? fp_info_.sz_fp
        : other.fp_info_.sz_fp;

      void const * this_fp = fp_info_.by_malloc ? fn_ptr_ : reinterpret_cast<void const *>(buf_);
      void const * other_fp = other.fp_info_.by_malloc ? other.fn_ptr_ : reinterpret_cast<void const *>(other.buf_);

      int cmp_result = fpcmp_(this_fp, other_fp, min_sz_fp);
      if(0 != cmp_result)
        return cmp_result;

      if(fp_info_.sz_fp <= other.fp_info_.sz_fp)
        return -1;
      else
        return 1;
    }
  }

  // ====================================================================================================

  //
  // comparison operators
  //
  inline bool operator == (thisClass const & other) const
  {
    return 0 == compare_(other, false);
  }
  inline bool operator != (thisClass const & other) const
  {
    return 0 != compare_(other, false);
  }
  inline bool operator <=  (thisClass const & other) const
  {
    return 0 >= compare_(other, false);
  }
  inline bool operator < (thisClass const & other) const
  {
    return 0 > compare_(other, false);
  }
  inline bool operator > (thisClass const & other) const
  {
    return 0 < compare_(other, false);
  }
  inline bool operator >= (thisClass const & other) const
  {
    return 0 <= compare_(other, false);
  }

  // ====================================================================================================

  // implementations

  inline void free_fp_malloc_()
  {
    if(fp_info_.by_malloc)
    {
      fp_info_.by_malloc = false;
      deallocate_(fn_ptr_, fp_info_.sz_fp);
      fn_ptr_ = 0;
    }
  }

}; // template<typename R FD_COMMA FD_TPARMS(N), size_t t_countof_pvoid> struct FD_JOIN(delegateImpl,N)

} // namespace detail

} // namespace fd
