#if !defined(__CONFIG_HPP__INCLUDED__)
#define __CONFIG_HPP__INCLUDED__

// ====================================================================================================
// BEGIN of CONFIG
// ====================================================================================================

#if !defined(FD_MAX_NUM_PARAMS)
#define FD_MAX_NUM_PARAMS               8
#endif

// ====================================================================================================

#if !defined(FD_DEFAULT_ALLOCATOR)
#define FD_DEFAULT_ALLOCATOR            std::allocator<void>
#endif  // #if !defined(FD_DEFAULT_ALLOCATOR)

// ====================================================================================================

// internal buffer size of delegate in count of (void *)
// buf_size = countof_pvoid * sizeof(void *)
#if !defined(FD_BUF_SIZE_IN_COUNTOF_PVOID)
#define FD_BUF_SIZE_IN_COUNTOF_PVOID    2  // // default buf_size = 2 * 4 = 8 bytes
#endif

// ====================================================================================================

//
// to enable fd::bind helper function for easy conversion from boost::bind & boost::function to fd::delegate
// define FD_ENABLE_BIND_HELPER_FUNCTION macro before including, directly or indirectly, "delegate.h"
// 
// ====================================================================================================
// #define FD_ENABLE_BIND_HELPER_FUNCTION
// ====================================================================================================
//

// ====================================================================================================

// --------------------------------------------------
//
// to use delegate with __stdcall member functions,
// define FD_MEM_FN_ENABLE_STDCALL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_MEM_FN_ENABLE_STDCALL
// ====================================================================================================
//
// --------------------------------------------------
//
// to use delegate with __fastcall member functions,
// define FD_MEM_FN_ENABLE_FASTCALL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_MEM_FN_ENABLE_FASTCALL
// ====================================================================================================
//
// --------------------------------------------------
//
// to use delegate with __cdecl member functions,
// define FD_MEM_FN_ENABLE_CDECL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_MEM_FN_ENABLE_CDECL
// ====================================================================================================
//
// --------------------------------------------------
//
// to use delegate with __stdcall functions,
// define FD_FN_ENABLE_STDCALL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_FN_ENABLE_STDCALL
// ====================================================================================================
//
// --------------------------------------------------
//
// to use delegate with __stdcall functions,
// define FD_FN_ENABLE_FASTCALL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_FN_ENABLE_FASTCALL
// ====================================================================================================
//
// --------------------------------------------------
//
// to use delegate with pascal functions,
// define FD_FN_ENABLE_PASCAL macro before including, directly or indirectly, "delegate.h"
//
// ====================================================================================================
// #define FD_FN_ENABLE_PASCAL
// ====================================================================================================
//
// --------------------------------------------------

// It is best to define these macros in the project options, via -D on the command line, or as the first
// line in the translation unit (.cpp file) where delegate is used. Not following this rule can lead to
// obscure errors when a header includes delegate.h before the macro has been defined.

// ====================================================================================================

//
// Less restrictive types (type-check relaxation)
//
// Callbacks in C++
// ( http://bpeers.com/articles/callback/ )
//
//
// to use this feature, DO NOT define FD_DISABLE_TYPE_RELAXATION macro so that a function is allowed to be assigned/binded
// to delegate as long as it does meet the follwing three conditions
//
//    a) the number of argument matches,
//    b) each matching argument can be trivially converted (from delegate's argument to target function's argument) by compiler,
//    c) return type can be trivaially converted (from delegate's return type to target function's return type "and vice versa") by compiler.
//
// ex)
//
// int foo(int n1, int n2) { };
// long bar(long n, float f) { };
//
// // #define FD_DISABLE_TYPE_RELAXATION
// #include "delegate.h"
//
// typedef fd::delegate<int (int)> MyDelegate;

// MyDelegate dg1 = &foo; // ok!
// MyDelegate dg2 = &bar; // ok! since the number of arguments (2 arguments) match
//                        // and long and floag can be trivially converted into int by compiler
//                        // but float > int conversion might cause a warning of 'possible loss of data'
//                        //
//                        // if FD_DISABLE_TYPE_RELAXATION were to be defined above, it causes errors here
//
// int result1 = dg1(123, 456); // ok!
// int result2 = dg2(345, 678); // ok!
//
// ====================================================================================================
// #define FD_DISABLE_TYPE_RELAXATION
// ====================================================================================================

//
// Cloning bound object
//

// Cloning bound object feature are added in v1.10. and there are several changes in public member function
// interfaces (member fucntion which related with argument binding,
// ex) delegate::bind(mfn, obj), delegate::operator <<= (ftor). fd::make_delegate(mfn, obj), fd::bind(mfn, obj, ...) )
//
// Unless otherwise FD_DISABLE_CLONE_BOUND_OBJECT is defined, the cloning bound object is enabled (default)
//

// ==============================
// argument binding
// ==============================

// CBase1 obj;
// smart_ptr<CBase1> sp(new CBase1);
//
// typedef int (CBase1::*TMFn)(int);
// TMFn mfn = &CBase1::foo;
//
// dg11.bind(mfn, &obj);
// dg12.bind(mfn, obj);
// dg13.bind(mfn, sp);
//
// delegate<> dg14(mfn, &obj);
// delegate<> dg15(mfn, obj);
// delegate<> dg16(mfn, sp);
//
// dg11(123); // (&obj)->foo(123);
// dg12(123); // (internal copy of obj).foo(123);
// dg13(123); // (internal copy of sp)->foo(123);
//
// dg14(123); // (&obj)->foo(123);
// dg15(123); // (internal copy of obj).foo(123);
// dg16(123); // (internal copy of sp)->foo(123);
//

// ==============================
// functor assignment
// ==============================

// struct bar { int operator()(int n) { return n; } };
//
// bar ftor;
//
// dg21 <<= &ftor;
// dg22 <<= ftor;
//
// // the second bool argument below is meaningless and provided just for the different function signature
// delegate<> dg23(&ftor, true); 
// delegate<> dg24(ftor, true);
//
// dg21(123); // (&ftor)->operator()(123);
// dg22(123); // (internal copy of ftor).operator()(123);
//
// dg23(123); // (&ftor)->operator()(123);
// dg24(123); // (internal copy of ftor).operator()(123);
//

//
// the reason I added cloning bound object feature is mostly to support any smart pointer
// (such as std::auto_ptr, boost::shared_ptr or loki::smartPtr)
// Though there are prerequisite/condition to be accomplished for a smart pointer to be recognized and supported
// by fd::delegate
//
//  a) T * get_pointer(smart_ptr (const) & p); is provided for the smart_ptr class and this function must
//     be seen in qualified namespace (including argument-dependent lookup)
//
//  b) smart_ptr class must expose public interface (typedef) of ' element_type ' that is the type of object at which
//     the smart_ptr points internally (std::auto_ptr, boost smart pointers and loki::smartPtr expose this
//     interface)
//
// If a smart_ptr class does not provide get_pointer() in its own namespace or your compiler poorly implemented the
// argument-dependent lookup (Koenig lookup) nor implemented it at all, then you can explicitly provide get_pointer
// for yourself in namespace fd
//
// namespace fd
// {
// #if defined(_MSC_VER) && _MSC_VER < 1300
// // get_pointer() for boost::shared_ptr is defined in boost namespace but VC6 did not implement koenig look-up
// // so it can't find the definition from namespace boost. therefore we can provide it in namespace fd for ourselves,
//   template<typename T> inline
//     T * get_pointer(boost::shared_ptr const & p)
//   {
//     return p.get();
//   }
// #endif // #if defined(_MSC_VER) && _MSC_VER < 1300
// } // namespace fd
//
// smart pointer will be copied internally which will transfer ownership or increase reference count and when
// the delegate is being destructed, the cloned smart pointer will be destructed, which will delete the object
// or decrease the reference count dependent on their implementation.
//
// Be aware that cloning the bound object involve heap memory allocation/deallocation using new/delete operator.
// The size of the object cloned (either it is bound object or smart pointer) is not known, therefore heap
// memory allocation/deallocation is inevitable. Since the idea of using fast delegate is avoiding
// heap memory allocation/deallocation but cloning object on the heap memory conflict with it. (though member
// function pointer will be still stored in the stack rather than heap). If speed is the only concern, do not use
// cloning bound object feature by defining FD_DISABLE_CLONE_BOUND_OBJECT macro then use reference version of
// bind() and relevant functions only. When FD_DISABLE_CLONE_BOUND_OBJECT is defined, extra 4 bytes space per delegate
// is saved as an additional bonus. (since obj_clone_man_ptr_ is not necessary any more)

// ====================================================================================================
// #define FD_DISABLE_CLONE_BOUND_OBJECT
// ====================================================================================================

// ====================================================================================================
// END of CONFIG
// ====================================================================================================

#endif  // #if !defined(__CONFIG_HPP__INCLUDED__)
