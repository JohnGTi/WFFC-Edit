/** @file bindtmpl.h
 *
 *  @brief  fd::bind
 *
 *  (note)
 *  [obsolete] bind do not support FD_ENABLE_TYPE_RELAXATION, use make_delegate() instead
 *             make_delegate do not support FD_TYPE_REXATION neither from version 1.01
 *
 *
 *  @author JaeWook Choi
 *  @version 1.10
 *
 *  @history
 *    1.10 (03.12.2006) - see history of "delegateimpl.h"
 *    1.01 (03.10.2006) - bind implementation is separated from delegatetmpl.h
 *
 * copied from FastDelegateBind.h (http://www.codeproject.com/cpp/FastDelegate.asp)
 * Original author: Jody Hagins
 *
 * This software is provided "as is" without express or implied warranty, and with
 * no claim as to its suitability for any purpose.
 *
 */

// Note: this header is a header template and must NOT have multiple-inclusion
// protection.

namespace fd
{

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (U::*mfn)(FD_TARGS(N)), T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (U::*mfn)(FD_TARGS(N)) const, T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (U::*mfn)(FD_TARGS(N)), T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (U::*mfn)(FD_TARGS(N)) const, T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#if defined(FD_MEM_FN_ENABLE_STDCALL)

#if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)
template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)), T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_STDCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj, ...)
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
bind(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)), T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_FASTCALL_ U::*mfn)(FD_TARGS(N)) const, T * obj, ...)
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
bind(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T & obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_REF(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, obj);
}
#endif  // #if !defined(FD_DISABLE_CLONE_BOUND_OBJECT)

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)), T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

template<typename R FD_COMMA FD_TPARMS(N), typename U, typename T> inline
FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>
bind(R (FD_CDECL_ U::*mfn)(FD_TARGS(N)) const, T * obj, ...)
{
  // to prevent non-const member function call on const object
  FD_PARAM_TYPE_CHK_OBJ_PTR(U const *, obj);

  return FD_JOIN(delegate,N)<R FD_COMMA FD_TARGS(N)>(mfn, get_pointer(obj));
}

#endif  // #if defined(FD_MEM_FN_ENABLE_CDECL)

}  // namespace fd
