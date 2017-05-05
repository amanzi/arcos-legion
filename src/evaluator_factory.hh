//! --------------------------------------------------------------------------------
//
// Arcos -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
//  A mockup of some evaluators for use in testing.  This can be
//  replaced with the standard Arcos factory.
//
//
//  This only deals with evaluators A-H, as in Amanzi test example
//  src/state/state_dag.cc
//  ---------------------------------------------------------------------------------

#ifndef EVALUATORS_FACTORY_HH_
#define EVALUATORS_FACTORY_HH_

#include "evaluators.hh"
#include "task_managers.hh"
#include "UniqueHelpers.hh"

namespace Arcos {

struct Evaluator_Factory {
  // Note that this will be done with parameter lists in the usual way, this is mocked
  std::unique_ptr<Evaluator> Create(const std::string& eval_type, State& s);
};

} // namespace

#endif
