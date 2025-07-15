/** @file get_pointer.hpp
 *
 *  @brief  get_pointer(p) extracts a ->* capable pointer from p
 *          excerpted from boost/get_pointer.hpp
 *
 * Copyright Peter Dimov and David Abrahams 2002. Permission to copy,
 * use, modify, sell and distribute this software is granted provided
 * this copyright notice appears in all copies of the source. This
 * software is provided "as is" without express or implied warranty,
 * and with no claim as to its suitability for any purpose.
 *
 */
#if !defined(__GET_POINTER_HPP__INCLUDED__)
#define __GET_POINTER_HPP__INCLUDED__

namespace fd
{

template<class T> inline
T * get_pointer(T * p)
{
  return p;
}

template<class T> inline
T * get_pointer(std::auto_ptr<T> & p)
{
  return p.get();
}

}  // namespace fd

#endif  // #if !defined(__GET_POINTER_HPP__INCLUDED__)
