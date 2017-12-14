/* 

Ugly first cut, tasks only.  Chains results.  


Totally blocking.

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
  assert(task->arglen == sizeof(std::tuple<double,double,double,double>));
  auto args = *(const std::tuple<double,double,double,double>*)task->args;
  double fb = std::get<0>(args);
  double fc = std::get<1>(args);
  double fe = std::get<2>(args);
  double fh = std::get<3>(args);
  double a = 2*fb + fc*fe*fh;
  std::cout << "A = " << a << std::endl;
  return a;
}


double CEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->arglen == sizeof(std::tuple<double,double>));
  auto args = *(const std::tuple<double,double>*)task->args;
  double fd = std::get<0>(args);
  double fg = std::get<1>(args);

  double b = 2*fd + fg;
  std::cout << "B = " << b << std::endl;
  return b;
}


double DEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->arglen == sizeof(double));
  double fg = *(const double*)task->args;
  double d = 2*fg;
  std::cout << "D = " << d << std::endl;
  return d;

}

double EEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->arglen == sizeof(std::tuple<double,double>));
  auto args = *(const std::tuple<double,double>*)task->args;
  double fd = std::get<0>(args);
  double ff = std::get<1>(args);

  double e = fd*ff;
  std::cout << "E = " << e << std::endl;
  return e;
  
}

double FEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->arglen == sizeof(double));
  double fg = *(const double*)task->args;
  double f = 2*fg;
  std::cout << "F = " << f << std::endl;
  return f;
}

double HEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(task->arglen == sizeof(double));
  double ff = *(const double*)task->args;
  double h = 2*ff;
  std::cout << "H = " << h << std::endl;
  return h;
}


void top_level_task(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  // manually form the graph
  double a,b,c,d,e,f,g,h;
  
  // leaf level is primaries
  {
    std::cout << "(Launching B): ";
    TaskLauncher B(BEvaluator_ID, TaskArgument());
    auto b_future = runtime->execute_task(ctx, B);
    b = b_future.get_result<double>();
  }

  {
    std::cout << "(Launching G): ";
    TaskLauncher G(GEvaluator_ID, TaskArgument());
    auto g_future = runtime->execute_task(ctx, G);
    g = g_future.get_result<double>();
  }

  // second tier
  {
    std::cout << "(Launching D): ";
    TaskLauncher D(DEvaluator_ID, TaskArgument(&g, sizeof(g)));
    auto d_future = runtime->execute_task(ctx, D);
    d = d_future.get_result<double>();
  }

  {
    std::cout << "(Launching F): ";
    TaskLauncher F(FEvaluator_ID, TaskArgument(&g, sizeof(g)));
    auto f_future = runtime->execute_task(ctx, F);
    f = f_future.get_result<double>();
  }

  // third tier
  {
    std::cout << "(Launching C): ";
    auto argC = std::make_tuple(d,g);
    TaskLauncher C(CEvaluator_ID, TaskArgument(&argC, sizeof(argC)));
    auto c_future = runtime->execute_task(ctx, C);
    c = c_future.get_result<double>();
  }

  {
    std::cout << "(Launching E): ";
    auto argE = std::make_tuple(d,f);
    TaskLauncher E(EEvaluator_ID, TaskArgument(&argE, sizeof(argE)));
    auto e_future = runtime->execute_task(ctx, E);
    e = e_future.get_result<double>();
  }

  {
    std::cout << "(Launching H): ";
    TaskLauncher H(HEvaluator_ID, TaskArgument(&f, sizeof(double)));
    auto h_future = runtime->execute_task(ctx, H);
    h = h_future.get_result<double>();
  }

  // fourth tier
  {
    std::cout << "(Launching A): ";
    auto argA = std::make_tuple(b,c,e,h);
    TaskLauncher A(AEvaluator_ID, TaskArgument(&argA, sizeof(argA)));
    auto a_future = runtime->execute_task(ctx, A);
    a = a_future.get_result<double>();
  }
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






