//! --------------------------------------------------------------------------------
//
// Arcos -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
// A TaskManager is a struct for holding several static methods used
// in pushing tasks onto the Legion runtime.  All TaskManagers must supply:
//
// struct TaskManager {
//   static LHL::TaskID taskid;
//   static void preregister_task(LHL::TaskID new_taskid = AUTO_GENERATE_ID);
//   static LHL::Future compute(LHL::Context ctx, LHL::Runtime *runtime, ...);
//   static double cpu_task(const LHL::Task *task,
// 		        const std::vector<LHL::PhysicalRegion> &regions,
// 		        LHL::Context ctx, LHL::Runtime *runtime);
// };
//
//
// preregister_task() should call some variant of
//	LegionRuntime::HighLevel::register_X() or
//	LegionRuntime::HighLevel::preregister_Y()
// This function is called exactly once on each task.
//
// compute() should take whatever arguments (parameters, future lists,
// etc) needed to bundle and spawn the task.  This is called each time
// a task is passed off to the runtime queue.
//
// cpu_task() should be the actual task implementation, which unpacks
// the futures/args and does the work.
//
// Note cpu_task() is the only one whose interface is fixed by Legion.
//
// ---------------------------------------------------------------------------------


namespace Arcos {

// implementation of primary
// ------------------------------------------------------------------
template<typename Return_t>
void
TaskManagerPrimary<Return_t>::preregister_task(LHL::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
	    LHL::Runtime::generate_static_task_id() :
	      new_taskid);
  LHL::TaskVariantRegistrar tvr(taskid, "primary_variable");
  //  std::cout << "Registering task: primary_variable" << std::endl;
  tvr.add_constraint(LHL::ProcessorConstraint(LHL::Processor::LOC_PROC));
  tvr.set_leaf(true);
  LHL::Runtime::preregister_task_variant<Return_t, &TaskManagerPrimary<double>::cpu_task>(tvr, "primary_variable");
}


template<typename Return_t>
LHL::Future
TaskManagerPrimary<Return_t>::compute(LHL::Context ctx, LHL::Runtime *runtime, const Return_t& value)
{
  LHL::TaskLauncher launcher(taskid, TaskArgument(&value, sizeof(Return_t)));
  return runtime->execute_task(ctx, launcher);
}


template<typename Return_t>
Return_t
TaskManagerPrimary<Return_t>::cpu_task(const LHL::Task *task,
				       const std::vector<LHL::PhysicalRegion> &regions,
				       LHL::Context ctx, LHL::Runtime *runtime)
{
  //  std::cout << "  Running primary variable" << std::endl;
  return *((Return_t*) task->args);
}

template<typename Return_t>
LHL::TaskID TaskManagerPrimary<Return_t>::taskid = 0;



// implementation of Secondary
// ------------------------------------------------------------------

// template magic to pack a tuple of values from a vector of their futures
//
// read a future and return its result
template<typename T>
T readFuture(std::vector<LHL::Future>::const_iterator& f)
{
  
  T t = f->get_result<T>();
  f++;
  return std::move(t);
}

// read a vector of futures and return a tuple of their results
template<typename... Args>
std::tuple<Args...> futureToValue(std::vector<LHL::Future>::const_iterator f)
{
  return std::make_tuple(readFuture<Args>(f)...);
}


// Secondary methods
// ------------------------------------------------------------------
template<typename Return_t, typename Func_t, typename... Args>
void
TaskManagerSecondary<Return_t, Func_t, Args...>
::preregister_task(LHL::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
  	      LHL::Runtime::generate_static_task_id() :
	      new_taskid);
  std::cout << "Registering task: " << Func_t::name << std::endl;
  LHL::TaskVariantRegistrar tvr(taskid, Func_t::name);
  tvr.add_constraint(LHL::ProcessorConstraint(LHL::Processor::LOC_PROC));
  tvr.set_leaf(true);
  LHL::Runtime::preregister_task_variant<Return_t, &TaskManagerSecondary<double, Func_t,Args...>::cpu_task>(tvr, Func_t::name);
}


template<typename Return_t, typename Func_t, typename... Args>
LHL::Future
TaskManagerSecondary<Return_t,Func_t,Args...>
::compute(LHL::Context ctx, LHL::Runtime *runtime, const std::vector<LHL::Future>& futures, const Func_t& func)
{
  LHL::TaskLauncher launcher(taskid, TaskArgument(&func, sizeof(Func_t)));
  for (auto f : futures) launcher.add_future(f);
  return runtime->execute_task(ctx, launcher);
}


template<typename Return_t, typename Func_t, typename... Args>
Return_t
TaskManagerSecondary<Return_t,Func_t,Args...>
::cpu_task(const LHL::Task *task,
	   const std::vector<LHL::PhysicalRegion> &regions,
	   LHL::Context ctx, LHL::Runtime *runtime)
{
  assert(std::tuple_size<std::tuple<Args...> >::value == task->futures.size());
  const Func_t& func= *(const Func_t*)(task->args);
  auto values = futureToValue<Args...>(task->futures.begin());
  std::cout << "got a task with " << task->futures.size() << " elems" << std::endl;

  return Arcos::Magic::invoke<Return_t>(func,values);
}


template<typename Return_t, typename Func_t, typename... Args>
LHL::TaskID TaskManagerSecondary<Return_t,Func_t,Args...>::taskid = 0;




} // namespace
