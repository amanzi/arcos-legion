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

namespace Arcos {

static const int g_ncells = 4;

class Evaluator;

struct State {
  State(Legion::Context ctx_, Legion::Runtime *runtime_)
    : ctx(ctx_),
      runtime(runtime_),
      domain(Legion::DomainPoint(0), Legion::DomainPoint(g_ncells-1)),
      n_fids(0)
  {}

  ~State();
  
  Legion::Context ctx;
  Legion::Runtime *runtime;
  Legion::Domain domain;
  Legion::LogicalRegion logical_region;
  Legion::PhysicalRegion physical_region;
  
  std::map<std::string,Legion::Future> futures;
  std::map<std::string,Legion::FieldID> field_ids;
  std::map<std::string,std::unique_ptr<Evaluator> > evaluators;

  void report();
  void RequireEvaluator(const std::string& eval_type);

  void Setup();

 private:
  int n_fids;
};

} // namespace Arcos

#endif
