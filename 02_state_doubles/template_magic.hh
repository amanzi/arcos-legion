//! --------------------------------------------------------------------------------
//
// Arcos
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
// This file simply has useful template magic stolen from various
// public domains i.e. stack overflow for use in Arcos.
//
// ---------------------------------------------------------------------------------


#ifndef TEMPLATE_MAGIC_HH_
#define TEMPLATE_MAGIC_HH_

namespace Arcos {
namespace Magic {

//
// magic to invoke a generic function on an unpacked tuple of arguments
// --------------------------------------------------------------------------------

// a sequence of integers from 1..N
template<int ...> struct seq {};

// a generator for sequences of integers from 1..N
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

// an invocation tool
template<typename Functor_t, typename ...Args>
struct Invoker
{
  Functor_t func;
  std::tuple<Args...> params;
  
  double Invoke() {
    return CallFunctor(typename gens<sizeof...(Args)>::type());
  }

  template<int ...S>
  double CallFunctor(seq<S...>)
  {
    return func(std::get<S>(params) ...);
  }
};

//
// Generic function that takes a functor and a tuple of arguments,
// then invokes that functor's operator() using the unpacked tuple as
// arguments.
//
// This functionality is replaced in c++17 by std::apply()
//
template<typename Return_t, typename Functor_t, typename ...Args>
Return_t invoke(const Functor_t& functor, const std::tuple<Args...>& args)
{
  Invoker<Functor_t, Args...> inv = { functor, args };
  return inv.Invoke();
}


} // namespace Magic
} // namespace Arcos




#endif 

