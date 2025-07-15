/** @file bindtmpl.h
 *
 *  @brief  helper functions (fd::make_delegate)
 *
 *
 *  @author JaeWook Choi
 *  @version 1.10
 *
 *  @history
 *    1.10 (03.12.2006) - see history of "delegateimpl.h"
 *    1.01 (03.10.2006) - make_delegate implementation is separated from delegatetmpl.h
 *                        removed type-check relaxation version of make_delegate
 *                        (make_delegate is automatic type deduction function and it is not
 *                         useful at all in type-check relaxation mode)
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

template<typename R FD_COMMA FD_TPARMS(N)> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (*fn)(FD_TARGS(N)))
{
  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(fn);
}

#if defined(FD_FN_ENABLE_STDCALL)

template<typename R FD_COMMA FD_TPARMS(N)> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_STDCALL_ *fn)(FD_TARGS(N)))
{
  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(fn);
}

#endif  // #if defined(FD_FN_ENABLE_STDCALL)

#if defined(FD_FN_ENABLE_FASTCALL)

template<typename R FD_COMMA FD_TPARMS(N)> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_FASTCALL_ *fn)(FD_TARGS(N)))
{
  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(fn);
}

#endif  // #if defined(FD_FN_ENABLE_FASTCALL)

#if defined(FD_FN_ENABLE_PASCAL)

template<typename R FD_COMMA FD_TPARMS(N)> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_PASCAL_ *fn)(FD_TARGS(N)))
{
  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(fn);
}

#endif  // #if defined(FD_FN_ENABLE_PASCAL)

// ====================================================================================================
// member function argument binding
// ====================================================================================================

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (U::*mfn)(FD_FPARMS(N)), T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (U::*mfn)(FD_FPARMS(N)) const, T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (U::*mfn)(FD_FPARMS(N)), T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (U::*mfn)(FD_FPARMS(N)) const, T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#if defined(FD_MEM_FN_ENABLE_STDCALL)

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_STDCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)), T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_FASTCALL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T & obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)), T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
make_delegate(R (FD_CDECL_ U::*mfn)(FD_FPARMS(N)) const, T * obj)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

// ====================================================================================================

#if N > 0

//
// make_delegate_adpater() for member function adapter
//
// When using make_delegate() helper function for member function adapter,
// it is impossible to tell whether member function adapter is used in the
// pointer form (1) or in the reference form (2), therefore only pointer form (1)
// of make_delegate() for member function adapter will be enabled
//

// ----------------------------------------------------------------------
//
// 1)
// template<typename T>
// R operator()(cv-quals T * px, T1 p1, T2 p2, ..., Tn pn) cv-quals
// {
//   return px->.*mf(p1, p2, ..., pn);
// }
//
// (NOTE)
//
// the first argument of the pointer to object is only used for providing explicit type
// information about the member function which is being stored
//
// (why?)
//
// class Base
// {
// public:
//   virtual int foo(int);
// };
// class Derived : public Base
// {
// };
// template<typename T>
// void bar(int (T::*mfn)(int))
// {
//   enum { assertion = fd::util::Is_same<T, Derived>::value };
//   FD_STATIC_ASSERT(assertion);
// }
// int main()
// {
//   bar(&Derived::foo);  // cause FD_STATIC_ASSERT() in bar() at compile time !!!
// }
//
// In the code snippet shown above, typename 'T' is found to be 'Base' not 'Derived'
// even though member function pointer was specified as &Derived::foo when calling bar().
// therefore, to make make_delegate() to work as the user intended, explicit type info.
// should be passed over into make_delegate() call, something like,
//
// fd::delegate2<int, Derived *, int> dg = make_delegate((Derived *)0, &Derived::foo);
//
// If explicit type info. is passed over into make_delegate() as the second argument instead,
// it will confuse the compiler as its signature become exactly same as
// the member function with binging object version of make_delegate(), So type info.
// passed over into make_delegate() as the first argument to distinguish b/w them.
//
// It isn't really nice, but I don't see any better solution for this
//

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (U::*mfn)(FD_FPARMS_SP(N)))
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK(U *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (U::*mfn)(FD_FPARMS_SP(N)) const)
{
  FD_PARAM_TYPE_CHK(U const *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

#if defined(FD_MEM_FN_ENABLE_STDCALL)

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)))
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK(U *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_STDCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
{
  FD_PARAM_TYPE_CHK(U const *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

#endif  // #if defined(FD_MEM_FN_ENABLE_STDCALL)

#if defined(FD_MEM_FN_ENABLE_FASTCALL)

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)))
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK(U *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_FASTCALL_ U::*mfn)(FD_FPARMS_SP(N)) const)
{
  FD_PARAM_TYPE_CHK(U const *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

#endif  // #if defined(FD_MEM_FN_ENABLE_FASTCALL)

#if defined(FD_MEM_FN_ENABLE_CDECL)

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)))
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK(U *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

template<typename R FD_COMMA_SP FD_TPARMS_SP(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>
make_delegate(T *, R (FD_CDECL_ U::*mfn)(FD_FPARMS_SP(N)) const)
{
  FD_PARAM_TYPE_CHK(U const *, T *);

  return FD_JOIN(delegate,N)<R, T * FD_COMMA_SP FD_TARGS_SP(N)>(mfn);
}

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

#endif  // #if N > 0

}  // namespace fd
