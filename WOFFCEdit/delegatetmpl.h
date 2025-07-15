/** @file delegatetmpl.h
 *
 *  @brief  delegate interfaces (Preferred Syntax and Portable Syntax)
 *
 *
 *  @author JaeWook Choi
 *  @version 1.10
 *
 *  @history
 *    1.10 (03.12.2006) - added cloning bound object support for argument binding of member function
 *                          -> bind(mfn, obj)       : clone obj internally and invoke mfn on cloned object
 *                          -> bind(mfn, obj_ptr)   : store only the pointer to the obj (obj_ptr) and invoke
 *                                                    mfn on the pointed object
 *                          -> bind(mfn, smart_ptr) : clone smart_ptr internally and invoke mfn on the object
 *                                                    which is pointed by smart_ptr
 *                                                    (assuming T * get_pointer() is seen in the qualified
 *                                                    namespace and smart_ptr expose public interface element_type)
 *                        functor assignment operator (operator <<=) are changed to be coincident with bind()
 *                          -> operator <<= (ftor)      : clone functor internally and invoke operator () on the
 *                                                        cloned functor
 *                          -> operator <<= (ftor_ptr)  : store only the pointer to the functor (ftor_ptr) and
 *                                                        invoke operator () on the pointed fucntor
 *                        added copy c'tor for functor
 *                          -> delegateN(ftor, bool dummy)      : clone version copy c'tor for functor
 *                          -> delegateN(ftor_ptr, bool dummy)  : reference storing version copy c'tor for functor
 *                        macro definition are slightly changed and source codes are updated according to changes
 *                          -> FD_TYPE_RELAXATION changed to FD_DISABLE_TYPE_RELAXATION
 *                          -> FD_DISABLE_CLONING_BOUND_OBJECT added
 *                             if FD_DISABLE_CLONING_BOUND_OBJECT is not defined (default), cloning version of member
 *                             are enabled,
 *                             othereise, only reference storing version of member function are available but
 *                             4 bytes less per delegate in its size
 *                          -> therefore default is enabling both type-check relaxation mode and cloning bound object
 *                        added support for the custom allocator (the second last template argument)
 *    1.01 (03.10.2006) - seperated bind & make_delegate implementation into separate files
 *                        added supports for arbitary smart pointer whch supply T * get_pointer() overloads
 *                        ( copied from boost::mem_fn and, this version only recognize smart pointers
 *                          but do not trnasfer ownership nor increase refernce count )
 *                        removed the definition of fd::arg<> & fd::_1, fd::_2, ... fd::_9
 *                        removed make_delegate for type-check relaxation mode
 *    1.00 (03.01.2006) - Initial public release
 *
 *
 * This software is provided "as is" without express or implied warranty, and with
 * no claim as to its suitability for any purpose.
 *
 */

// Note: this header is a header template and must NOT have multiple-inclusion
// protection.

//
// --------------------------------------------------
// Preferred syntax
// --------------------------------------------------
// fd::delegate<R (T1, T2, ..., Tn), Allocator, t_countof_pvoid>;
//
// --------------------------------------------------
// Portable syntax
// --------------------------------------------------
// fd::delegateN<R, T1, T2, ... , Tn, Allocator, t_countof_pvoid>;
//
// --------------------------------------------------
// size of delegate
// --------------------------------------------------
// sizeof(delegate) = sizeof(delegateN) = t_countof_pvoid * 4 + 12 + alpha (bytes)
//                                      = 2 (default) * 4 + 12 = 20 + alpha (bytes)
//
// , where alpha is the size of the allocated memory if and only if it is required to be allocated
// , otherwise 0
//

#include "delegatetmpl.hpp"

namespace fd
{

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<typename R FD_COMMA FD_TPARMS(N), typename Allocator, size_t t_countof_pvoid>
class FD_JOIN(delegate,N); // forward reference

// ====================================================================================================
//
// ====================================================================================================
//
// Preferred syntax
//
// class delegate<R (T1, T2, ..., Tn), Allocator, t_countof_pvoid>
//
// ====================================================================================================
//
// ====================================================================================================
template<typename R FD_COMMA FD_TPARMS(N), typename Allocator, size_t t_countof_pvoid>
class delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>
  : public detail::safe_bool<delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> >
{
  typedef delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> thisClass;
  // ====================================================================================================
  // data members
  // ====================================================================================================
private:

  typedef detail::FD_JOIN(delegateImpl,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> implClass;
  implClass impl_;

public:

  typedef typename implClass::result_type result_type;

#if N == 1
  // std::unary_function compatible
  typedef T1 argument_type;
#endif  // #if N == 1

#if N == 2
  // std::binary_function compatible
  typedef T1 first_argument_type;
  typedef T2  second_argument_type;
#endif  // #if N == 2

  struct friend__
  {
    static inline implClass &
      impl_of_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & friend_delegate)
    {
      return friend_delegate.impl_;
    }
    static inline const implClass &
      impl_of_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> const & friend_delegate)
    {
      return friend_delegate.impl_;
    }

  };  // friend__
  friend struct friend__;

  // ====================================================================================================
  // c'tors & d'tor
  // ====================================================================================================
public:


  // default c'tor
  delegate()
    : impl_()
  {
  }

  // to allow zero delegate constructions
  //
  // fd::delegate1<void, int> dg1(0);           // ok
  // fd::delegate<int (float, double)> dg2 = 0; // ok
  // fd::delegate0<void> dg3(1234);             // error
  // fd::delegate<int (int, int)> dg4 = 3456;   // error
  //
  delegate(typename implClass::clear_type const *)
    : impl_()
  {
  }

  // copy c'tor
  delegate(thisClass const & other)
    : impl_(other.impl_)
  {
  }

   // copy c'tor
  delegate(const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other)
    : impl_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other))
  {
  }

  //
  // copy c'tor with member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // copy c'tor with member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // copy c'tor with member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // copy c'tor with member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    delegate(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  // ----------------------------------------------------------------------

  // copy c'tor

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    delegate(UR (*fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (*TFn1)(FD_TARGS(N));
    typedef UR (*TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    delegate(UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_STDCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_STDCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    delegate(UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_FASTCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_FASTCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    delegate(UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_PASCAL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_PASCAL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

#if N > 0

  // copy c'tor
  // specialization for member function call adapter
  //  template<typename U>
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // copy c'tor
  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // copy c'tor
  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // copy c'tor
  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // copy c'tor
  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    delegate(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename F>
    delegate(F & functor, bool/* dummy*/)
    : impl_()
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_cloned_(functor, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename F>
    delegate(F * functor, bool/* dummy*/)
    : impl_()
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_ptr_(functor, false);
  }

  // d'tor
  ~delegate() { }

  // ====================================================================================================
  // operations
  // ====================================================================================================
public:

  //
  // swap()
  //
  inline void swap(thisClass & other)
  {
    impl_.swap_(other.impl_);
  }

  inline void swap(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other)
  {
    impl_.swap_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }

  //
  // clear()
  //
  inline void clear()
  {
    impl_.reset_();
  }

  //
  // empty()
  //
  inline bool empty() const
  {
    return impl_.empty_();
  }

  //
  // assignment operators
  //

  // ----------------------------------------------------------------------

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename F>
    inline thisClass & operator <<= (F & functor)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_cloned_(functor);

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename F>
    inline thisClass & operator <<= (F * functor)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_ptr_(functor);

    return *this;
  }

  // ----------------------------------------------------------------------

  inline thisClass & operator = (thisClass const & other)
  {
    if(&this->impl_ == &other.impl_)
      return *this;

    impl_.assign_(other.impl_);

    return *this;
  }

  inline thisClass & operator = (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> const & other)
  {
    if(&this->impl_ == &FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other))
      return *this;

    impl_.assign_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));

    return *this;
  }

  // ----------------------------------------------------------------------

  // to allow the zero assignment to clear delegate
  //
  // fd::delegate1<void, int> dg1;
  // dg1 = 0;     // ok
  // dg1 = 1234;  // error
  //
  inline thisClass & operator = (typename implClass::clear_type const *)
  {
    impl_.reset_();
    return *this;
  }

  // ----------------------------------------------------------------------

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (*fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (*TFn1)(FD_TARGS(N));
    typedef UR (*TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_STDCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_STDCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_FASTCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_FASTCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_PASCAL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_PASCAL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

#if N > 0

  // specialization for member function call adapter

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

  //
  // bind()
  //

  //
  // member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
    template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
    
    impl_.bind_obj_ptr_(mfn, obj);
    
    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // function call operator
  //
  inline result_type operator () (FD_FPARMS(N)) const
  {
    return impl_.operator () (FD_FARGS(N));
  }

  //
  // compare()
  //
  inline int compare(thisClass const & other, bool check_bound_object = false) const
  {
    return impl_.compare_(other.impl_, check_bound_object);
  }

  inline int compare(const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other, bool check_bound_object = false) const
  {
    return impl_.compare_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other), check_bound_object);
  }

  //
  // comparison operators
  //

  // to allow to compare to 0
  inline bool operator == (typename implClass::clear_type const *) const
  {
    return impl_.empty_();
  }
  inline bool operator != (typename implClass::clear_type const *) const
  {
    return !impl_.empty_();
  }

  // ------------------------------------------------------------

  inline bool operator == (thisClass const & other) const
  {
    return impl_.operator == (other.impl_);
  }
  inline bool operator != (thisClass const & other) const
  {
    return impl_.operator != (other.impl_);
  }
  inline bool operator <= (thisClass const & other) const
  {
    return impl_.operator <= (other.impl_);
  }
  inline bool operator < (thisClass const & other) const
  {
    return impl_.operator < (other.impl_);
  }
  inline bool operator > (thisClass const & other) const
  {
    return impl_.operator > (other.impl_);
  }
  inline bool operator >= (thisClass const & other) const
  {
    return impl_.operator >= (other.impl_);
  }

  inline bool operator == (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator == (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator != (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator != (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator <= (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator <= (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator < (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator < (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator > (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator > (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator >= (const FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator >= (FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }

};  // template<typename R, typename T, size_t t_countof_pvoid> class delegate

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

// ====================================================================================================
//
// ====================================================================================================
//
// Portable syntax
//
// class delegateN<R, T1, T2, ..., Tn, Allocator, t_countof_pvoid>;
//
// ====================================================================================================
//
// ====================================================================================================
template<typename R FD_COMMA FD_TPARMS(N), typename Allocator = FD_DEFAULT_ALLOCATOR , size_t t_countof_pvoid = DefaultCountOfVoid>
class FD_JOIN(delegate,N)
  : public detail::safe_bool<FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> >
{
  typedef FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> thisClass;

  // ====================================================================================================
  // data members
  // ====================================================================================================
private:

  typedef detail::FD_JOIN(delegateImpl,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> implClass;
  implClass impl_;

public:

  typedef typename implClass::result_type result_type;

#if N == 1
  // std::unary_function compatible
  typedef T1 argument_type;
#endif  // #if N == 1

#if N == 2
  // std::binary_function compatible
  typedef T1 first_argument_type;
  typedef T2  second_argument_type;
#endif  // #if N == 2

  struct friend__
  {
    static implClass &
      impl_of_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> & friend_delegate)
    {
      return friend_delegate.impl_;
    }
    static const implClass &
      impl_of_(FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N), Allocator, t_countof_pvoid> const & friend_delegate)
    {
      return friend_delegate.impl_;
    }

  };  // friend__
  friend struct friend__;

  // ====================================================================================================
  // c'tors & d'tor
  // ====================================================================================================
public:

  // default c'tor
  FD_JOIN(delegate,N)()
    : impl_()
  {
  }

  // to allow zero delegate constructions
  //
  // fd::delegate1<void, int> dg1(0);           // ok
  // fd::delegate<int (float, double)> dg2 = 0; // ok
  // fd::delegate0<void> dg3(1234);             // error
  // fd::delegate<int (int, int)> dg4 = 3456;   // error
  //
  FD_JOIN(delegate,N)(typename implClass::clear_type const *)
    : impl_()
  {
  }

  // copy c'tor
  FD_JOIN(delegate,N)(thisClass const & other)
    : impl_(other.impl_)
  {
  }

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  FD_JOIN(delegate,N)(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> const  & other)
    : impl_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other))
  {
  }

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  //
  // copy c'tor with member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // copy c'tor with member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // copy c'tor with member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // copy c'tor with member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T & obj)
#endif
    : impl_(mfn, obj)
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T & obj)
#endif
    : impl_(mfn, const_cast<T const &>(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)), T * obj)
#endif
    : impl_(mfn, get_pointer(obj))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_TARGS2(N)) const, T * obj)
#endif
    : impl_(mfn, const_cast<T const *>(get_pointer(obj)))
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  // ----------------------------------------------------------------------

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    FD_JOIN(delegate,N)(UR (*fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (*TFn1)(FD_TARGS(N));
    typedef UR (*TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_STDCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_STDCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_FASTCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_FASTCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  // copy c'tor
  template<typename UR FD_COMMA FD_TPARMS2(N)>
    FD_JOIN(delegate,N)(R (FD_PASCAL_ *fn)(FD_TARGS2(N)))
    : impl_(fn)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_PASCAL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_PASCAL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

#if N > 0

  // copy c'tor
  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);
  }

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    FD_JOIN(delegate,N)(R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    FD_JOIN(delegate,N)(UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
    : impl_(mfn)
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename F>
    FD_JOIN(delegate,N)(F & functor, bool/* dummy*/)
    : impl_()
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_cloned_(functor, false);
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename F>
    FD_JOIN(delegate,N)(F * functor, bool/* dummy*/)
    : impl_()
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_ptr_(functor, false);
  }

  // d'tor
  ~FD_JOIN(delegate,N)() { }

  // ====================================================================================================
  // operations
  // ====================================================================================================
public:

  //
  // swap()
  //
  inline void swap(thisClass & other)
  {
    impl_.swap_(other.impl_);
  }

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  inline void swap(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other)
  {
    impl_.swap_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  // ====================================================================================================

  //
  // clear()
  //
  inline void clear()
  {
    impl_.reset_();
  }

  // ====================================================================================================

  //
  // empty()
  //
  inline bool empty() const
  {
    return impl_.empty_();
  }

  // ====================================================================================================

  //
  // assignment operators
  //

  // ----------------------------------------------------------------------

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
  template<typename F>
    inline thisClass & operator <<= (F & functor)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_cloned_(functor);

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

  template<typename F>
    inline thisClass & operator <<= (F * functor)
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (F::*TMFn)(FD_TARGS(N));
    typedef R (F::*TMFn_const)(FD_TARGS(N)) const;
    detail::chk_functor((TMFn *)0, (TMFn_const *)0, &F::operator());
#endif

    impl_.assign_ftor_ptr_(functor);

    return *this;
  }

  // ----------------------------------------------------------------------
  
  inline thisClass & operator = (thisClass const & other)
  {
    if(&this->impl_ == &other.impl_)
      return *this;

    impl_.assign_(other.impl_);

    return *this;
  }

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  inline thisClass & operator = (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> const & other)
  {
    if(&this->impl_ == &delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other))
      return *this;

    impl_.assign_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));

    return *this;
  }

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  // ----------------------------------------------------------------------

  // to allow the zero assignment to clear delegate
  //
  // fd::delegate1<void, int> dg1;
  // dg1 = 0;     // ok
  // dg1 = 1234;  // error
  //
  inline thisClass & operator = (typename implClass::clear_type const *)
  {
    impl_.reset_();
    return *this;
  }

  // ----------------------------------------------------------------------

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (*fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (*TFn1)(FD_TARGS(N));
    typedef UR (*TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#if defined(FD_FN_ENABLE_STDCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_STDCALL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_STDCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_STDCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_FASTCALL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_FASTCALL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_FASTCALL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

  template<typename UR FD_COMMA FD_TPARMS2(N)>
    inline thisClass & operator = (UR (FD_PASCAL_ *fn)(FD_TARGS2(N)))
  {
#if !defined(FD_NDEBUG) && defined(FD_DISABLE_TYPE_RELAXATION)
    typedef R (FD_PASCAL_ *TFn1)(FD_TARGS(N));
    typedef UR (FD_PASCAL_ *TFn2)(FD_TARGS2(N));
    TFn1 fn_chk = (TFn2)0; fn_chk;
#endif

    impl_.assign_(fn);
    return *this;
  }

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

#if N > 0

  // specialization for member function call adapter
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)))
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)))
#endif
  {
    // typename 'T1' should match with typename 'U *' or typename 'U &'
    enum { value = util::Is_pointer_or_reference<U, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U>
    inline thisClass & operator = (R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)) const)
#else
  template<typename UR FD_COMMA_SP FD_TPARMS_SP2(N), typename U>
    inline thisClass & operator = (UR (FD_CDECL_ U::*mfn)(FD_FPARMS_SP2(N)) const)
#endif
  {
    // typename 'T1' should match with typename 'U const *' or typename 'U const &'
    enum { value = util::Is_pointer_or_reference<U const, T1>::value };
    FD_STATIC_ASSERT(value);

    impl_.assign_(mfn);
    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)


#endif  // #if N > 0

  // ====================================================================================================

  //
  // bind()
  //

  //
  // member function argument binding
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#if defined(FD_MEM_FN_ENABLE_STDCALL)

  //
  // member function argument binding (__stdcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_STDCALL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

  //
  // member function argument binding (__fastcall)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_FASTCALL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, const_cast<T const *>(obj));

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

  //
  // member function argument binding (__cdecl)
  //
#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)), T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

    impl_.bind_obj_cloned_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)) const, T & obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

    impl_.bind_obj_cloned_(mfn, const_cast<T const &>(obj));

    return *this;
  }
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)), T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#if defined(FD_DISABLE_TYPE_RELAXATION)
  template<typename U, typename T>
    inline thisClass & bind(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
#else
  template<typename UR FD_COMMA FD_TPARMS2(N), typename U, typename T>
    inline thisClass & bind(UR (FD_CDECL_ U::*mfn)(FD_FPARMS2(N)) const, T * obj)
#endif
  {
    // to prevent non-const member function call on const object
    FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

    impl_.bind_obj_ptr_(mfn, obj);

    return *this;
  }

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

  // ====================================================================================================

  //
  // function call operator
  //
  inline result_type operator () (FD_FPARMS(N)) const
  {
    return impl_.operator () (FD_FARGS(N));
  }

  // ====================================================================================================

  //
  // compare()
  //
  inline int compare(thisClass const & other, bool check_bound_object = false) const
  {
    return impl_.compare_(other.impl_, check_bound_object);
  }

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  inline int compare(const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other, bool check_bound_object = false) const
  {
    return impl_.compare_(delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other), check_bound_object);
  }

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  // ====================================================================================================

  //
  // comparison operators
  //

  // to allow to compare to 0
  inline bool operator == (typename implClass::clear_type const *) const
  {
    return impl_.empty_();
  }
  inline bool operator != (typename implClass::clear_type const *) const
  {
    return !impl_.empty_();
  }

  // ------------------------------------------------------------

  inline bool operator == (thisClass const & other) const
  {
    return impl_.operator == (other.impl_);
  }
  inline bool operator != (thisClass const & other) const
  {
    return impl_.operator != (other.impl_);
  }
  inline bool operator <= (thisClass const & other) const
  {
    return impl_.operator <= (other.impl_);
  }
  inline bool operator < (thisClass const & other) const
  {
    return impl_.operator < (other.impl_);
  }
  inline bool operator > (thisClass const & other) const
  {
    return impl_.operator > (other.impl_);
  }
  inline bool operator >= (thisClass const & other) const
  {
    return impl_.operator >= (other.impl_);
  }

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  inline bool operator == (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator == (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator != (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator != (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator <= (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator <= (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator > (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator > (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator < (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator < (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }
  inline bool operator >= (const delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid> & other) const
  {
    return impl_.operator >= (delegate<R (FD_TARGS(N)), Allocator, t_countof_pvoid>::friend__::impl_of_(other));
  }

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

};  // template<typename R, typename T, size_t t_countof_pvoid> class FD_JOIN(delegate,N)

} // namespace fd
