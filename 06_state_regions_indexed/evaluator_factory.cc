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
// ---------------------------------------------------------------------------------

#include "functions.hh"
#include "evaluator_factory.hh"

namespace Arcos {

// Note that this will be done with parameter lists in the usual way, this is mocked
std::unique_ptr<Evaluator>
Evaluator_Factory::Create(const std::string& eval_type, State& s) {
  if (eval_type == "A") {
    std::cout << "  ...creating an A evaluator." << std::endl;
    KeyList deps;
    deps.push_back("B"); deps.push_back("C");
    deps.push_back("E"); deps.push_back("H");      
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FA,double,double,double,double>, FA> >("A", deps, s);
    
  } else if (eval_type == "B") {
    std::cout << "  ...creating a B evaluator." << std::endl;
    return std::make_unique<EvaluatorPrimary<TaskManagerPrimary<double> > >("B", 2.0);      
  } else if (eval_type == "C") {
    std::cout << "  ...creating a C evaluator." << std::endl;
    KeyList deps;
    deps.push_back("D"); deps.push_back("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FC,double,double>, FC> >("C", deps, s);
  } else if (eval_type == "D") {
    std::cout << "  ...creating a D evaluator." << std::endl;
    KeyList deps; deps.push_back("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FD,double>, FD> >("D", deps, s);
  } else if (eval_type == "E") {
    std::cout << "  ...creating a E evaluator." << std::endl;
    KeyList deps;
    deps.push_back("D"); deps.push_back("F");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FE,double,double>, FE> >("E", deps, s);
  } else if (eval_type == "F") {
    std::cout << "  ...creating an F evaluator." << std::endl;
    KeyList deps; deps.push_back("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FF,double>, FF> >("F", deps, s);
  } else if (eval_type == "G") {
    std::cout << "  ...creating a G evaluator." << std::endl;
    return std::make_unique<EvaluatorPrimary<TaskManagerPrimary<double> > >("G", 3.0);      
  } else if (eval_type == "H") {
    std::cout << "  ...creating an H evaluator." << std::endl;
    KeyList deps; deps.push_back("F");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<FH,double>, FH> >("H", deps, s);
  } else {
    std::cout << "evaluator_factory passed bad argument " << eval_type << std::endl;
    throw("evaluator_factory passed bad argument");
  }
}


} // namespace Arcos
