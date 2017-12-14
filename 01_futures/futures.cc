/* 

Second cut uses futures, which is then nonblocking.  Still hard-coded,
explicitly executed DAG.

 */


#include <cstdio>
#include <cassert>
#include <cstdlib>
#include "legion.h"
using namespace Legion;




enum TaskIDs {
  TOP_LEVEL_ID,
  AEvaluator_ID,
  BEvaluator_ID,
  CEvaluator_ID,
  DEvaluator_ID,
  EEvaluator_ID,
  FEvaluator_ID,
  GEvaluator_ID,
  HEvaluator_ID
};

// primaries
double BEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  std::cout << "B = 2" << std::endl;
  return 2.;
}
double GEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  std::cout << "G = 3" << std::endl;
  return 3.;
}



// secondaries
double AEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 4);
  auto fb = task->futures[0].get_result<double>();
  auto fc = task->futures[1].get_result<double>();
  auto fe = task->futures[2].get_result<double>();
  auto fh = task->futures[3].get_result<double>();

  double a = 2*fb + fc*fe*fh;
  std::cout << "A = " << a << std::endl;
  return a;
}


double CEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 2);
  auto fd = task->futures[0].get_result<double>();
  auto fg = task->futures[1].get_result<double>();

  double b = 2*fd + fg;
  std::cout << "B = " << b << std::endl;
  return b;
}


double DEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 1);
  auto fg = task->futures[0].get_result<double>();
  double d = 2*fg;
  std::cout << "D = " << d << std::endl;
  return d;

}

double EEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 2);
  auto fd = task->futures[0].get_result<double>();
  auto ff = task->futures[0].get_result<double>();

  double e = fd*ff;
  std::cout << "E = " << e << std::endl;
  return e;
  
}

double FEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 1);
  auto fg = task->futures[0].get_result<double>();
  double f = 2*fg;
  std::cout << "F = " << f << std::endl;
  return f;
}

double HEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->futures.size() == 1);
  auto ff = task->futures[0].get_result<double>();
  double h = 2*ff;
  std::cout << "H = " << h << std::endl;
  return h;
}


void top_level_task(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  // leaf level is primaries
  std::cout << "(Launching B): ";
  TaskLauncher B(BEvaluator_ID, TaskArgument(NULL,0));
  auto b_future = runtime->execute_task(ctx, B);

  std::cout << "(Launching G): ";
  TaskLauncher G(GEvaluator_ID, TaskArgument(NULL,0));
  auto g_future = runtime->execute_task(ctx, G);

  // second tier
  std::cout << "(Launching D): ";
  TaskLauncher D(DEvaluator_ID, TaskArgument(NULL,0));
  D.add_future(g_future);
  auto d_future = runtime->execute_task(ctx, D);

  std::cout << "(Launching F): ";
  TaskLauncher F(FEvaluator_ID, TaskArgument(NULL,0));
  F.add_future(g_future);
  auto f_future = runtime->execute_task(ctx, F);

  // third tier
  std::cout << "(Launching C): ";
  TaskLauncher C(CEvaluator_ID, TaskArgument(NULL,0));
  C.add_future(d_future);
  C.add_future(g_future);
  auto c_future = runtime->execute_task(ctx, C);

  std::cout << "(Launching E): ";
  TaskLauncher E(EEvaluator_ID, TaskArgument(NULL,0));
  E.add_future(d_future);
  E.add_future(f_future);
  auto e_future = runtime->execute_task(ctx, E);

  std::cout << "(Launching H): ";
  TaskLauncher H(HEvaluator_ID, TaskArgument(NULL,0));
  H.add_future(f_future);
  auto h_future = runtime->execute_task(ctx, H);

  // fourth tier
  std::cout << "(Launching A): ";
  TaskLauncher A(AEvaluator_ID, TaskArgument(NULL,0));
  A.add_future(b_future);
  A.add_future(c_future);
  A.add_future(e_future);
  A.add_future(h_future);
  auto a_future = runtime->execute_task(ctx, A);
}


int main(int argc, char **argv) {
  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/);

  Runtime::register_legion_task<double,AEvaluator>(AEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "AEvaluator");
  Runtime::register_legion_task<double,BEvaluator>(BEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "BEvaluator");
  Runtime::register_legion_task<double,CEvaluator>(CEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "CEvaluator");
  Runtime::register_legion_task<double,DEvaluator>(DEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "DEvaluator");
  Runtime::register_legion_task<double,EEvaluator>(EEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "EEvaluator");
  Runtime::register_legion_task<double,FEvaluator>(FEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "FEvaluator");
  Runtime::register_legion_task<double,GEvaluator>(GEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "GEvaluator");
  Runtime::register_legion_task<double,HEvaluator>(HEvaluator_ID,
      Processor::LOC_PROC, true/*single*/, false/*index*/, 
      AUTO_GENERATE_ID, TaskConfigOptions(true/*leaf*/), "HEvaluator");

  
  return HighLevelRuntime::start(argc, argv);
}






