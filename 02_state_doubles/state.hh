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

using namespace LegionRuntime::HighLevel;

namespace Arcos {

class Evaluator;

struct State {
  State(Context ctx_, Runtime *runtime_)
    : ctx(ctx_),
      runtime(runtime_) {}

  Context ctx;
  Runtime *runtime;
  std::map<std::string,Future> futures;
  std::map<std::string,std::unique_ptr<Evaluator> > evaluators;

  void report();
  void RequireEvaluator(const std::string& eval_type);
  
};

} // namespace Arcos

#endif
