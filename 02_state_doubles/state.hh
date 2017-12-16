//! --------------------------------------------------------------------------------
//
// Arcos -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
// A mockup of State for testing with Legion
//
// ---------------------------------------------------------------------------------


#ifndef STATE_HH_
#define STATE_HH_

#include "legion.h"
#include "evaluators.hh"

namespace Arcos {

class Evaluator;

struct State {
  State(Legion::Context ctx_, Legion::Runtime *runtime_)
    : ctx(ctx_),
      runtime(runtime_) {}

  Legion::Context ctx;
  Legion::Runtime *runtime;
  std::map<std::string,Legion::Future> futures;
  std::map<std::string,std::unique_ptr<Evaluator> > evaluators;

  void report();
  void RequireEvaluator(const std::string& eval_type);
  
};

} // namespace Arcos

#endif
