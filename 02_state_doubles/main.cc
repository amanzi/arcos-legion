#include <iostream>
#include <cmath>

//#define STRING_HOLDER(VAR) struct VAR { static const char* asString() { return #VAR; } } 


#include "legion.h"
#include "functions.hh"
#include "task_managers.hh"
#include "evaluators.hh"
#include "state.hh"
#include "UniqueHelpers.hh"


using namespace Legion;
using namespace Arcos;

enum TaskIDList {
  TOP_LEVEL_TASK_ID
};



void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{
  State s(ctx, runtime);

  // require primaries
  s.RequireEvaluator("B");
  s.RequireEvaluator("G");

  // require top level A
  s.RequireEvaluator("A");

  s.report(); // empty?
  
  // go
  s.evaluators["A"]->Update(s, "main");

  s.report(); // correct?

  // check the answer
  assert(std::abs(s.futures["A"].get_result<double>() - 6484.0) < 1.e-10);
  std::cout << "Test passed!" << std::endl;
}
  


const char* FA::name = "fa";
const char* FC::name = "fc";
const char* FD::name = "fd";
const char* FE::name = "fe";
const char* FF::name = "ff";
const char* FH::name = "fh";

int main(int argc, char **argv) {
  {
    TaskVariantRegistrar tvr(TOP_LEVEL_TASK_ID, "top_level_task");
    tvr.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<top_level_task>(tvr, "top_level_task");
    Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  }

  TaskManagerPrimary<double>::preregister_task();

  TaskManagerSecondary<double, FA,double,double,double,double>::preregister_task();
  TaskManagerSecondary<double, FC,double,double>::preregister_task();
  TaskManagerSecondary<double, FD,double>::preregister_task();
  TaskManagerSecondary<double, FE,double,double>::preregister_task();
  TaskManagerSecondary<double, FF,double>::preregister_task();
  TaskManagerSecondary<double, FH,double>::preregister_task();
  
  return HighLevelRuntime::start(argc,argv);
}
