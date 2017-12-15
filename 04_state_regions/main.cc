#include <iostream>
#include <cmath>

//#define STRING_HOLDER(VAR) struct VAR { static const char* asString() { return #VAR; } } 


#include "legion.h"
#include "state.hh"
#include "functions.hh"
#include "task_managers.hh"
#include "evaluators.hh"
#include "UniqueHelpers.hh"


using namespace LegionRuntime::HighLevel;
using namespace Arcos;

enum TaskIDList {
  TOP_LEVEL_TASK_ID,
  TEST_ID
};


void TestEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  // in
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  const Legion::FieldAccessor<READ_ONLY,double,1> fa(regions[0], fid);

  for (int i=0; i!=g_ncells; ++i) {
    assert(std::abs(fa[i] - 6484.0) < 1.e-10);
  }
  printf("Successful test!\n");
}



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
  s.Setup(); // create everything
  
  // go
  s.evaluators["A"]->Update(s, "main");

  s.report(); // correct?

  // Get the A result and check to make sure it worked!
  std::cout << "Launching Test Check Answer" << std::endl;
  Legion::TaskLauncher Tlauncher(TEST_ID, TaskArgument(NULL, 0));
  Tlauncher.add_region_requirement(
      RegionRequirement(s.logical_region, READ_ONLY, EXCLUSIVE, s.logical_region));
  Tlauncher.add_field(0,s.field_ids["fa"]);
  runtime->execute_task(ctx, Tlauncher);
  
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
  {
    TaskVariantRegistrar registrar(TEST_ID, "TestEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<TestEvaluator>(registrar, "TestEvaluator");
  }

  TaskManagerPrimary<double>::preregister_task();

  TaskManagerSecondary<FA,double,double,double,double>::preregister_task();
  TaskManagerSecondary<FC,double,double>::preregister_task();
  TaskManagerSecondary<FD,double>::preregister_task();
  TaskManagerSecondary<FE,double,double>::preregister_task();
  TaskManagerSecondary<FF,double>::preregister_task();
  TaskManagerSecondary<FH,double>::preregister_task();
  
  return HighLevelRuntime::start(argc,argv);
}
