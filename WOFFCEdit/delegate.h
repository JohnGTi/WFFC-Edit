/** @file delegate.h
 *
 *  @brief  delegate class
 *
 *
 *  @author JaeWook Choi
 *  @version 1.10
 *
 *  @history
 *    1.10 (03.12.2006) - see history of "delegateimpl.h"
 *    1.01 (03.10.2006) - added supports for arbitary smart pointer whch supply T * get_pointer() overloads
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

// ====================================================================================================
// References
// ====================================================================================================
//
// 1. CALLBACKS IN C++ USING TEMPLATE FUNCTORS by Rich Hickey
// ( http://www.tutok.sk/fastgl/callback.html )
// - summarizes existing callback methods and their weaknesses then describes a flexible, powerful and
//   easy-to-use callback technique based on template functors. ('1994)
//
// 2. Callbacks in C++
// ( http://bpeers.com/articles/callback/ )
// - The article based on Rich Hickey's article to illustrate the concept and techniques used to implement callbacks
//
// 3. Member Function Pointers and the Fastest Possible C++ Delegates by Don Clugston
// ( http://codeproject.com/cpp/delegate.asp )
// - A comprehensive tutorial on member function pointers, and an implementation of
//   delegates that generates only two ASM opcodes!
//
// 4. The Impossibly Fast C++ Delegates by Sergey Ryazanov
// ( http://www.codeproject.com/cpp/ImpossiblyFastCppDelegate.asp )
// - A implementation of a delegate library which can work faster than "the Fastest
//   Possible C++ Delegates" and is completely compatible with the C++ Standard.
//
// 5. Yet Another Generalized Functors Implementation in C++ By Aleksei Trunov
// ( http://www.codeproject.com/cpp/genfunctors.asp )
// - An article on generalized functors implementation in C++. Generalized functor requirements,
//   existing implementation problems and disadvantages are considered. Several new ideas and problem
//   solutions together with the compete implementation are suggested.
//
// ====================================================================================================

#if !defined(__DELEGATE_H__INCLUDED__)
#define __DELEGATE_H__INCLUDED__

#include <malloc.h>
#include <algorithm>
#include <stdexcept>

// --------------------------------------------------

#include "config.hpp"

// --------------------------------------------------
//
// To recognize and support any smart pointer which supply the appropriate T * get_pointer() overloads
// ( http://boost.org/libs/bind/mem_fn.html )
//
#include "get_pointer.hpp"

// ====================================================================================================

#if defined(_MSC_VER)

#pragma warning(disable:4522) // warning C4522: '' : multiple assignment operators specified

#if _MSC_VER >= 1300
#define FD_MS_VC
#else // #if _MSC_VER >= 1300
#define FD_MS_VC6
#pragma warning(disable:4786) // warning C4786: identifier was truncated to '255' characters in the debug information
#endif  // #if _MSC_VER >= 1300

#endif  // #if defined(_MSC_VER)

// ====================================================================================================

#if defined(__GNUC__)
#define FD_GNUC
#endif  // #if defined(__GNUC__)

// ====================================================================================================

// ====================================================================================================
// Calling conventions
// ====================================================================================================
#if defined(FD_MS_VC) || defined(FD_MS_VC6)
//
// http://blogs.msdn.com/oldnewthing/archive/2004/01/02/47184.aspx
// http://blogs.msdn.com/oldnewthing/archive/2004/01/08/48616.aspx
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore98/html/_core_results_of_calling_example.asp
//
#define FD_STDCALL_   __stdcall
#define FD_FASTCALL_  __fastcall
#define FD_CDECL_     __cdecl
#define FD_PASCAL_

#undef FD_FN_ENABLE_PASCAL

#endif  // #if defined(FD_MS_VC) || defined(FD_MS_VC6)

#if defined(FD_GNUC)
//
// todo: I couldn't figure out how to use calling convention for G++ at the moment
//
// http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html#Function-Attributes
//

//#define FD_STDCALL_   __attribute__ ((stdcall))
//#define FD_FASTCALL_  __attribute__ ((fastcall))
//#define FD_CDECL_     __attribute__ ((cdecl))
//#define FD_PASCAL_

#define FD_STDCALL_
#define FD_FASTCALL_
#define FD_CDECL_
#define FD_PASCAL_

#undef FD_MEM_FN_ENABLE_STDCALL
#undef FD_MEM_FN_ENABLE_FASTCALL
#undef FD_MEM_FN_ENABLE_CDECL
#undef FD_FN_ENABLE_STDCALL
#undef FD_FN_ENABLE_FASTCALL
#undef FD_FN_ENABLE_PASCAL

#endif  // #if defined(FD_GNUC)


// ====================================================================================================
// Platform specific workarounds
// ====================================================================================================

// VC71+
#if defined(FD_MS_VC)
#endif  // #if defined(FD_MS_VC)

// VC6
#if defined(FD_MS_VC6)
#define FD_NEED_SHADOW_TEMPLATE
#define FD_NO_VOID_RETURN
#define FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#define FD_NO_REBIND_STD_ALLOCATOR
#endif  // #if defined(FD_MS_VC6)

// DEV-C++ 4.9.9.2 (Mingw/gcc 3.4.2))
#if defined(FD_GNUC)
#define FD_NO_FULL_SPECIALIZATION_IN_NESTED_CLASS
#endif  // #if defined(FD_GNUC)

#if defined(FD_NO_FULL_SPECIALIZATION_IN_NESTED_CLASS)
//
// g++: Full VS Partial Specialization of Nested Templates
// ( http://lists.debian.org/debian-gcc/2004/09/msg00015.html )
//
#define FD_NESTED_PARTIAL_TPARM_MAIN(N)     , typename dummy##N = bool
#define FD_NESTED_PARTIAL_TPARM_PARTIAL(N)  typename dummy##N
#define FD_NESTED_PARTIAL_TARG(N)           , dummy##N

#else // #if defined(FD_NO_FULL_SPECIALIZATION_IN_NESTED_CLASS)
#define FD_NESTED_PARTIAL_TPARM_MAIN(N)
#define FD_NESTED_PARTIAL_TPARM_PARTIAL(N)
#define FD_NESTED_PARTIAL_TARG(N)

#endif  // #if defined(FD_NO_FULL_SPECIALIZATION_IN_NESTED_CLASS)

#if defined(FD_NEED_SHADOW_TEMPLATE)
#define FD_SHADOW_TEMPLATE(T)  template<typename T>
#else // #if defined(FD_NEED_SHADOW_TEMPLATE)
#define FD_SHADOW_TEMPLATE(T)
#endif  // #if defined(FD_NEED_SHADOW_TEMPLATE)

// ====================================================================================================

//
// Repeater macro to generate template/function parameters/arguments
//
// AGM::LibReflection: A reflection library for C++.
// ( http://codeproject.com/library/libreflection.asp )
//

#define __REPEAT0(M, C)
#define __REPEAT1(M, C)         M(1)
#define __REPEAT2(M, C)         __REPEAT1(M, C)  _##C M(2)
#define __REPEAT3(M, C)         __REPEAT2(M, C)  _##C M(3)
#define __REPEAT4(M, C)         __REPEAT3(M, C)  _##C M(4)
#define __REPEAT5(M, C)         __REPEAT4(M, C)  _##C M(5)
#define __REPEAT6(M, C)         __REPEAT5(M, C)  _##C M(6)
#define __REPEAT7(M, C)         __REPEAT6(M, C)  _##C M(7)
#define __REPEAT8(M, C)         __REPEAT7(M, C)  _##C M(8)
#define __REPEAT9(M, C)         __REPEAT8(M, C)  _##C M(9)
#define __REPEAT10(M, C)        __REPEAT9(M, C)  _##C M(10)
#define __REPEAT11(M, C)        __REPEAT10(M, C) _##C M(11)
#define __REPEAT12(M, C)        __REPEAT11(M, C) _##C M(12)
#define __REPEAT13(M, C)        __REPEAT12(M, C) _##C M(13)
#define __REPEAT14(M, C)        __REPEAT13(M, C) _##C M(14)
#define __REPEAT15(M, C)        __REPEAT14(M, C) _##C M(15)
#define __REPEAT16(M, C)        __REPEAT15(M, C) _##C M(16)
#define __REPEAT17(M, C)        __REPEAT16(M, C) _##C M(17)
#define __REPEAT18(M, C)        __REPEAT17(M, C) _##C M(18)
#define __REPEAT19(M, C)        __REPEAT18(M, C) _##C M(19)
#define __REPEAT20(M, C)        __REPEAT19(M, C) _##C M(20)
#define __REPEAT(N, M, C)       __REPEAT##N(M, C)

//various defs needed for parameters
#define ___COMMA__              ,
#define __TEMPLATE_PARAM__(N)   typename T##N
#define __TEMPLATE_ARG__(N)     T##N
#define __FUNCTION_PARAM__(N)   T##N p##N
#define __FUNCTION_ARG__(N)     p##N

#define __TEMPLATE_PARAM2__(N)  typename U##N
#define __TEMPLATE_ARG2__(N)    U##N
#define __FUNCTION_PARAM2__(N)  U##N q##N
#define __FUNCTION_ARG2__(N)    q##N

// ----------------------------------------------------------------------

//
// example
//
// macro expansion for " __REPEAT(2, __TEMPLATE_PARAM__, __COMMA__) "
//
// __REPEAT(3, M, C)
// => __REPEAT3(M, C)
// => __REPEAT2(M, C) _##C M(3)
// => __REPEAT1(M, C) _##C M(2) _##C M(3)
// => M(1) _##C M(2) _##C M(3)
// => __TEMPLATE_PARAM__(1) __COMMA__ __TEMPLATE_PARAM__(2) __COMMA__ __TEMPLATE_PARAM__(3)
// => typename T1 , typename T2, typename T3
//
// macro expansion for " __REPEAT(3, __FUNCTION_ARG__, __COMMA__) "
//
// => M(1) _##C M(2) _##C M(3)
// => __FUNCTION_ARG__(1) __COMMA__ __FUNCTION_ARG__(2) __COMMA__ __FUNCTION_ARG__(3)
// => p1, p2, p3
//

// ====================================================================================================

#define FD_TPARMS(N)        __REPEAT(N, __TEMPLATE_PARAM__, __COMMA__)
// typename T1, typename T2, ... , typename Tn
#define FD_TARGS(N)         __REPEAT(N, __TEMPLATE_ARG__, __COMMA__)
// T1, T2, ... , Tn
#define FD_FPARMS(N)        __REPEAT(N, __FUNCTION_PARAM__, __COMMA__)
// T1 p1, T2 p2, ... , Tn pn
#define FD_FARGS(N)         __REPEAT(N, __FUNCTION_ARG__, __COMMA__)
// p1, p2, ... , pn

#define FD_TPARMS2(N)       __REPEAT(N, __TEMPLATE_PARAM2__, __COMMA__)
// typename U1, typename U2, ... , typename Un
#define FD_TARGS2(N)        __REPEAT(N, __TEMPLATE_ARG2__, __COMMA__)
// U1, U2, ... , Un
#define FD_FPARMS2(N)       __REPEAT(N, __FUNCTION_PARAM2__, __COMMA__)
// U1 q1, U2 q2, ... , Un qn
#define FD_FARGS2(N)        __REPEAT(N, __FUNCTION_ARG2__, __COMMA__)
// q1, q2, ... , qn

// ----------------------------------------------------------------------

//
// for member function call adapter
//
#define __REPEAT_SP0(M, C)
#define __REPEAT_SP1(M, C)
#define __REPEAT_SP2(M, C)      M(2)
#define __REPEAT_SP3(M, C)      __REPEAT_SP2(M, C)  _##C M(3)
#define __REPEAT_SP4(M, C)      __REPEAT_SP3(M, C)  _##C M(4)
#define __REPEAT_SP5(M, C)      __REPEAT_SP4(M, C)  _##C M(5)
#define __REPEAT_SP6(M, C)      __REPEAT_SP5(M, C)  _##C M(6)
#define __REPEAT_SP7(M, C)      __REPEAT_SP6(M, C)  _##C M(7)
#define __REPEAT_SP8(M, C)      __REPEAT_SP7(M, C)  _##C M(8)
#define __REPEAT_SP9(M, C)      __REPEAT_SP8(M, C)  _##C M(9)
#define __REPEAT_SP10(M, C)     __REPEAT_SP9(M, C)  _##C M(10)
#define __REPEAT_SP11(M, C)     __REPEAT_SP10(M, C) _##C M(11)
#define __REPEAT_SP12(M, C)     __REPEAT_SP11(M, C) _##C M(12)
#define __REPEAT_SP13(M, C)     __REPEAT_SP12(M, C) _##C M(13)
#define __REPEAT_SP14(M, C)     __REPEAT_SP13(M, C) _##C M(14)
#define __REPEAT_SP15(M, C)     __REPEAT_SP14(M, C) _##C M(15)
#define __REPEAT_SP16(M, C)     __REPEAT_SP15(M, C) _##C M(16)
#define __REPEAT_SP17(M, C)     __REPEAT_SP16(M, C) _##C M(17)
#define __REPEAT_SP18(M, C)     __REPEAT_SP17(M, C) _##C M(18)
#define __REPEAT_SP19(M, C)     __REPEAT_SP18(M, C) _##C M(19)
#define __REPEAT_SP20(M, C)     __REPEAT_SP19(M, C) _##C M(20)
#define __REPEAT_SP(N, M, C)    __REPEAT_SP##N(M, C)

#define FD_TPARMS_SP(N)     __REPEAT_SP(N, __TEMPLATE_PARAM__, __COMMA__)
// typename T2, typename T3, ... , typename Tn
#define FD_TARGS_SP(N)      __REPEAT_SP(N, __TEMPLATE_ARG__, __COMMA__)
// T2, T3, ... , Tn
#define FD_FPARMS_SP(N)     __REPEAT_SP(N, __FUNCTION_PARAM__, __COMMA__)
// T2 p2, T3 p3, ... , Tn pn
#define FD_FARGS_SP(N)      __REPEAT_SP(N, __FUNCTION_ARG__, __COMMA__)
// p2, p3, ... , pn

#define FD_TPARMS_SP2(N)     __REPEAT_SP(N, __TEMPLATE_PARAM2__, __COMMA__)
// typename U2, typename U3, ... , typename Un
#define FD_TARGS_SP2(N)      __REPEAT_SP(N, __TEMPLATE_ARG2__, __COMMA__)
// U2, U3, ... , Un
#define FD_FPARMS_SP2(N)     __REPEAT_SP(N, __FUNCTION_PARAM2__, __COMMA__)
// U2 q2, U3 q3, ... , Un qn
#define FD_FARGS_SP2(N)      __REPEAT_SP(N, __FUNCTION_ARG2__, __COMMA__)
// q2, q3, ... , qn

// ====================================================================================================

#define FD_JOIN(a, b)        FD_DO_JOIN(a, b)
#define FD_DO_JOIN(a, b)     FD_DO_JOIN2(a, b)
#define FD_DO_JOIN2(a, b)    a##b

#define COUNTOF_PVOID_OF_FUNCTION(fn)   (sizeof(fn) / sizeof(void *))

#undef  FD_NDEBUG
#ifdef  NDEBUG
#define FD_NDEBUG
#endif

#define FD_STATIC_ASSERT(expr)  FD_JOIN(typedef typename fd::util::t_assert<(bool)(expr)>::assertion failed, __LINE__)

#ifdef FD_NDEBUG
#define FD_PARAM_TYPE_CHK(type1, type2) ((void)0)
#else
#define FD_PARAM_TYPE_CHK(type1, type2) type1 param_chk = (type2)0; param_chk
#endif

#ifdef FD_NDEBUG
#define FD_PARAM_TYPE_CHK_OBJ_PTR(type1, typed_obj) ((void)0)
#else
#define FD_PARAM_TYPE_CHK_OBJ_PTR(type1, typed_obj) type1 param_chk = typed_obj; param_chk
#endif

#ifdef FD_NDEBUG
#define FD_PARAM_TYPE_CHK_OBJ_REF(type1, typed_obj) ((void)0)
#else
#define FD_PARAM_TYPE_CHK_OBJ_REF(type1, typed_obj) type1 param_chk = detail::select_obj_type_<T, U>::type::get_pointer_(typed_obj); param_chk
#endif

#if defined(ASSERT)
#define FD_ASSERT               ASSERT
#elif defined(ATLASSERT)
#define FD_ASSERT               ATLASSERT
#else
#include <cassert>
#define FD_ASSERT(expr)         assert(expr)
#endif

// ====================================================================================================

namespace fd
{

namespace util
{

//
// for FD_STATIC_ASSERT()
//
// restricting constant template arguments
// ( http://cpptips.hyperformix.com/cpptips/restr_templ_args )
//
template<bool Condition> struct t_assert;
template<> struct t_assert<true>  { typedef bool assertion; };
template<> struct t_assert<false> { };

// ====================================================================================================

//
// A gentle introduction to Template Metaprogramming with C++ By moliate
// ( http://www.codeproject.com/cpp/crc_meta.asp )
//

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<bool t_condition, typename Then, typename Else> struct If;
template<typename Then, typename Else> struct If<true, Then, Else> { typedef Then Result; };
template<typename Then, typename Else> struct If<false, Then, Else> { typedef Else Result; };

#else // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template <bool t_condition, typename Then, typename Else>
struct If
{
  template<bool t_condition_inner> struct selector;
  template<> struct selector<true> { typedef Then Result; };
  template<> struct selector<false> { typedef Else Result; };

  typedef typename selector<t_condition>::Result Result;

};  // template <bool t_condition, typename Then, typename Else> If

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

// ====================================================================================================

template<typename T, typename U>
struct Is_pointer
{
  static U testee_;
  static int tester_(...);        // false condition
  static char tester_(T *);       // true condition, typename 'U' matches with typename 'T *' or 'T-derived *'

  enum { value = (sizeof(char) == sizeof(tester_(testee_))) };

};  // template<typename T, typename U> struct Is_pointer

template<typename T, typename U, typename Then, typename Else>
struct If_pointer
{
  // typename 'U' matches with typename 'T *', 'T const *', 'T-derived *' or 'T-derived const *'
  enum { Condition = Is_pointer<T, U>::value || Is_pointer<T const, U>::value };

  typedef typename If< Condition, Then, Else >::Result Result;

};  // template<typename T, typename U, typename Then, typename Else> struct If_pointer

// ====================================================================================================

template<typename T, typename U>
struct Is_reference
{
  static U testee_;
  static int tester_(...);        // false condition
  static char tester_(T &);       // true condition, typename 'U' matches with typename 'T &' or 'T-derived &'

  enum { value = (sizeof(char) == sizeof(tester_(testee_))) };

};  // template<typename T, typename U> struct Is_reference

template<typename T, typename U, typename Then, typename Else>
struct If_reference
{
  // typename 'U' matches with typename 'T &', 'T const &', 'T-derived &' or 'T-derived const &'
  enum { Condition = Is_reference<T, U>::value || Is_reference<T const, U>::value };

  typedef typename If< Condition, Then, Else >::Result Result;

};  // template<typename T, typename U, typename Then, typename Else> struct If_reference

// ====================================================================================================

template<typename T, typename U>
struct Is_pointer_or_reference
{
  // value is true, if typename 'U' matches with T' or 'T-derived' 's pointer or reference
  enum { value = Is_pointer<T, U>::value || Is_reference<T, U>::value };

};  // template<typename T, typename U> struct Is_pointer_or_reference

// ====================================================================================================

#if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<typename T, typename U> struct Is_same_type { enum { value = false }; };
template<typename T> struct Is_same_type<T, T> { enum { value = true }; };

#else // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template<typename T, typename U>
struct Is_same_type
{
  template<typename R> struct selector { enum { value = false }; };
  template<> struct selector<T> { enum { value = true }; };

  enum { value = selector<U>::value };

};  // template<typename T, typename U> struct Is_same_type

#endif  // #if !defined(FD_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

// ====================================================================================================

template<typename T> struct Is_integral
{
  static T testee_;

  // false conditions
  static int tester_(...);
  static int tester_(void);
  static int tester_(float);
  static int tester_(double);
  static int tester_(long double);

  // true conditions
  static unsigned char tester_(signed char);
  static unsigned char tester_(signed short);
  static unsigned char tester_(signed int);
  static unsigned char tester_(signed long);
  static unsigned char tester_(unsigned char);
  static unsigned char tester_(unsigned short);
  static unsigned char tester_(unsigned int);
  static unsigned char tester_(unsigned long);

  enum { value = (sizeof(unsigned char) == sizeof(tester_(testee_))) };

};  // template<typename T> struct Is_integral

// ====================================================================================================

template<typename T> struct Is_floating
{
  static T testee_;

  // false conditions
  static int tester_(...);
  static int tester_(void);
  static int tester_(signed char);
  static int tester_(signed short);
  static int tester_(signed int);
  static int tester_(signed long);
  static int tester_(unsigned char);
  static int tester_(unsigned short);
  static int tester_(unsigned int);
  static int tester_(unsigned long);

  // true conditions
  static unsigned char tester_(float);
  static unsigned char tester_(double);
  static unsigned char tester_(long double);

  enum { value = (sizeof(unsigned char) == sizeof(tester_(testee_))) };

};  // template<typename T> struct Is_floating

// ====================================================================================================

} // namespace util

// ====================================================================================================

namespace detail
{

// ====================================================================================================

//
// DefaultVoid - a workaround for 'void' templates in VC6.
//
//  - VC6 and earlier do not allow you to return 'void' from a function
//    => return 'void const *' instead
//
// Member Function Pointers and the Fastest Possible C++ Delegates By Don Clugston
// ( http://codeproject.com/cpp/delegate.asp )
//
//
#if defined(FD_NO_VOID_RETURN)
// VC6 workaround
typedef void const * DefaultVoid;
#else // #if defined(FD_NO_VOID_RETURN)
// On any other compiler, just use a normal void.
typedef void DefaultVoid;
#endif  // #if defined(FD_NO_VOID_RETURN)

// Translate from 'void' into 'DefaultVoid'
// Everything else is unchanged
template <class T>
struct VoidToDefaultVoid { typedef T type; };

template <>
struct VoidToDefaultVoid<void> { typedef DefaultVoid type; };


// ====================================================================================================

//
// The Safe Bool Idiom by Bjorn Karlsson
// ( http://www.artima.com/cppsource/safebool.html )
//
class safe_bool_base
{
public:
  typedef void (safe_bool_base::*bool_type)() const;
  void this_type_does_not_support_comparisons() const {}

protected:
  safe_bool_base() {}
  safe_bool_base(const safe_bool_base &) {}
  safe_bool_base& operator=(const safe_bool_base &) {return *this;}
  ~safe_bool_base() {}

};  // class safe_bool_base

template<typename T>
class safe_bool : public safe_bool_base
{
public:
  operator bool_type() const
  {
    T const * pT = static_cast<T const *>(this); pT;
    return !pT->empty()
      ? &safe_bool_base::this_type_does_not_support_comparisons
      : 0;
  }
protected:
  ~safe_bool() {}

};  // template<typename T> class safe_bool : public safe_bool_base

template<typename T, typename U>
void operator == (const safe_bool<T> & lhs, const safe_bool<U> & rhs)
{
  lhs.this_type_does_not_support_comparisons();
  return false;
}

template<typename T,typename U>
void operator != (const safe_bool<T> & lhs,const safe_bool<U> & rhs)
{
  lhs.this_type_does_not_support_comparisons();
  return false;
}

// ====================================================================================================

//
// tag for template specialization for free function (non-member function)
//
struct free_fn_tag { };

// ====================================================================================================

#define NO_OBJECT ((void *)-1)

// ====================================================================================================

template<typename TMFn, typename TMFn_const, typename TFtorOp> inline
void chk_functor(TMFn *, TMFn_const *, TFtorOp const & ftor_op)
{
  ftor_op;
  enum { value = util::Is_same_type<TMFn, TFtorOp>::value || util::Is_same_type<TMFn_const, TFtorOp>::value };
  FD_STATIC_ASSERT(value);
}

template<typename TMFn_const, typename TFtorOp> inline
void chk_functor_const(TMFn_const *, TFtorOp const & ftor_op)
{
  ftor_op;
  enum { value = util::Is_same_type<TMFn_const, TFtorOp>::value };

  FD_STATIC_ASSERT(value);  // functor do not have const version of operator ()
}

// ====================================================================================================

template<typename T>
struct self_ref_type_
{
  inline static T * get_pointer_(T const & obj)
  {
    return const_cast<T *>(&obj);
  }
};

template<typename T>
struct smart_ptr_type_
{
  inline static typename T::element_type * get_pointer_(T & obj)
  {
    return get_pointer(obj);
  }
};

template<typename T, typename U>
struct select_obj_type_
{
  enum { Condition = util::Is_reference<T, U>::value || util::Is_reference<U, T>::value };
  typedef self_ref_type_<T> Then;
  typedef smart_ptr_type_<T> Else;

  typedef typename util::If< Condition, Then, Else >::Result type;

};  // template<typename T, typename U> struct select_obj_type_

// ====================================================================================================

#if defined(FD_NO_REBIND_STD_ALLOCATOR)

//
// Workaround : std::allocator in VC6 (Dinkumware) does not implemented rebinding
//

template<typename T>
class std_allocator_fixed : public std::allocator<T>
{
public:
  template<typename U> struct rebind { typedef std_allocator_fixed<U> other; };

};

template<typename Allocator>
struct fix_if_std_allocator
{
  enum { Condition = util::Is_same_type<std::allocator<Allocator::value_type>, Allocator>::value };

  typedef std_allocator_fixed<Allocator::value_type> Then;
  typedef Allocator Else;

  // if std::allocator then fix it
  typedef typename util::If< Condition, Then, Else >::Result type;

};

#endif  // #if defined(FD_NO_REBIND_STD_ALLOCATOR)

template<typename Allocator>
class allocator_base
{

#if defined(FD_NO_REBIND_STD_ALLOCATOR)
  typedef typename fix_if_std_allocator<Allocator>::type alloc_;
#else
  typedef Allocator alloc_;
#endif

public:

  inline static void * allocate_(size_t n, void const * pInitHint = 0)
  {
    static typename alloc_::template rebind<unsigned char>::other alloc;
    return alloc.allocate(n, pInitHint);
  }

  inline static void deallocate_(void * p, size_t n)
  {
    static typename alloc_::template rebind<unsigned char>::other alloc;
    alloc.deallocate(static_cast<unsigned char *>(p), n);
  }

};  // template<typename Allocator> class allocator_base

} // namespace detail

// ====================================================================================================

//
// The bad_member_function_call exception class is thrown when a delegate object is invoked
//
class bad_member_function_call : public std::runtime_error
{
public:
  bad_member_function_call() : std::runtime_error("member function call on no object") {}

};  // class bad_member_function_call : public std::runtime_error

class bad_function_call : public std::runtime_error
{
public:
  bad_function_call() : std::runtime_error("call to empty delegate") {}

};  // class bad_function_call : public std::runtime_error

// ====================================================================================================

enum { DefaultCountOfVoid = FD_BUF_SIZE_IN_COUNTOF_PVOID };

// ====================================================================================================

template<typename T, typename Allocator = FD_DEFAULT_ALLOCATOR , size_t t_countof_pvoid = DefaultCountOfVoid>
class delegate; // no body

} // namespace fd

// ====================================================================================================

// Specializations

// --------------------------------------------------
#define FD_COMMA
// --------------------------------------------------
#define FD_COMMA_SP
// --------------------------------------------------

#define N 0
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

// --------------------------------------------------
#undef FD_COMMA
#define FD_COMMA  ,
// --------------------------------------------------

#define N 1
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

// --------------------------------------------------
#undef FD_COMMA_SP
#define FD_COMMA_SP  ,
// --------------------------------------------------

#define N 2
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 3
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 4
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 5
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 6
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 7
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 8
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 9
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 10
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 11
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 12
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 13
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 14
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 15
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 16
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 17
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 18
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 19
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

#define N 20
#if FD_MAX_NUM_PARAMS >= N
#include "delegatetmpl.h"
#include "make_delegatetmpl.hpp"
#if defined(FD_ENABLE_BIND_HELPER_FUNCTION)
#include "bindtmpl.hpp"
#endif
#endif
#undef N

// --------------------------------------------------
#undef FD_COMMA
// --------------------------------------------------
#undef FD_COMMA_SP
// --------------------------------------------------

#endif  // #if !defined(__DELEGATE_H__INCLUDED__)

