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

#include "evaluator_factory.hh"

namespace Arcos {

// Note that this will be done with parameter lists in the usual way, this is mocked
std::unique_ptr<Evaluator>
Evaluator_Factory::Create(const std::string& eval_type, State& s) {
  if (eval_type == "A") {
    std::set<std::string> deps;
    deps.insert("B"); deps.insert("C");
    deps.insert("E"); deps.insert("H");      
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FA,double,double,double,double>, FA> >("A", deps, s);
    
  } else if (eval_type == "B") {
    return std::make_unique<EvaluatorPrimary<TaskManagerPrimary<double> > >("B", 2.0);      
  } else if (eval_type == "C") {
    std::set<std::string> deps;
    deps.insert("D"); deps.insert("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FC,double,double>, FC> >("C", deps, s);
  } else if (eval_type == "D") {
    std::set<std::string> deps; deps.insert("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FD,double>, FD> >("D", deps, s);
  } else if (eval_type == "E") {
    std::set<std::string> deps;
    deps.insert("D"); deps.insert("F");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FE,double,double>, FE> >("E", deps, s);
  } else if (eval_type == "F") {
    std::set<std::string> deps; deps.insert("G");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FF,double>, FF> >("F", deps, s);
  } else if (eval_type == "G") {
    return std::make_unique<EvaluatorPrimary<TaskManagerPrimary<double> > >("G", 3.0);      
  } else if (eval_type == "H") {
    std::set<std::string> deps; deps.insert("F");
    return std::make_unique<EvaluatorSecondary<TaskManagerSecondary<double, FH,double>, FH> >("H", deps, s);
  } else {
    std::cout << "evaluator_factory passed bad argument " << eval_type << std::endl;
    throw("evaluator_factory passed bad argument");
  }
}


} // namespace Arcos
