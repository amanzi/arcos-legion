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

#include <iostream>
#include "evaluators.hh"
#include "evaluator_factory.hh"
#include "state.hh"

namespace Arcos {

void
State::report() {
  std::cout << "State Report:" << std::endl
	    << "-------------" << std::endl;
  for (auto& kf : futures) {
    double result = kf.second.get_result<double>();
    std::cout << "  " << kf.first << " = " << result << std::endl;
  }
  std:: cout << "-------------" << std::endl;
}

void
State::RequireEvaluator(const std::string& eval_type) {
  if (evaluators.count(eval_type) == 0) {
    Evaluator_Factory fac;
    evaluators[eval_type] = fac.Create(eval_type, *this);
  }
}


} // namespace Arcos
