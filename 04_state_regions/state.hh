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

using namespace LegionRuntime::HighLevel;

namespace Arcos {

static const int g_ncells = 4;

class Evaluator;

struct State {
  State(Context ctx_, Runtime *runtime_)
    : ctx(ctx_),
      runtime(runtime_),
      domain(DomainPoint(0), DomainPoint(g_ncells-1)),
      n_fids(0)
  {}

  ~State();
  
  Context ctx;
  Runtime *runtime;
  Domain domain;
  LogicalRegion logical_region;
  PhysicalRegion physical_region;
  
  std::map<std::string,Future> futures;
  std::map<std::string,FieldID> field_ids;
  std::map<std::string,std::unique_ptr<Evaluator> > evaluators;

  void report();
  void RequireEvaluator(const std::string& eval_type);

  void Setup();

 private:
  int n_fids;
};

} // namespace Arcos

#endif
