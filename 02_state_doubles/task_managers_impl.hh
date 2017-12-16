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
//   static Legion::TaskID taskid;
//   static void preregister_task(Legion::TaskID new_taskid = AUTO_GENERATE_ID);
//   static Legion::Future compute(Legion::Context ctx, Legion::Runtime *runtime, ...);
//   static double cpu_task(const Legion::Task *task,
// 		        const std::vector<Legion::PhysicalRegion> &regions,
// 		        Legion::Context ctx, Legion::Runtime *runtime);
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
TaskManagerPrimary<Return_t>::preregister_task(Legion::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
	    Legion::Runtime::generate_static_task_id() :
	      new_taskid);
  Legion::TaskVariantRegistrar tvr(taskid, "primary_variable");
  //  std::cout << "Registering task: primary_variable" << std::endl;
  tvr.add_constraint(Legion::ProcessorConstraint(Legion::Processor::LOC_PROC));
  tvr.set_leaf(true);
  Legion::Runtime::preregister_task_variant<Return_t, &TaskManagerPrimary<double>::cpu_task>(tvr, "primary_variable");
}


template<typename Return_t>
Legion::Future
TaskManagerPrimary<Return_t>::compute(Legion::Context ctx, Legion::Runtime *runtime, const Return_t& value)
{
  Legion::TaskLauncher launcher(taskid, Legion::TaskArgument(&value, sizeof(Return_t)));
  return runtime->execute_task(ctx, launcher);
}


template<typename Return_t>
Return_t
TaskManagerPrimary<Return_t>::cpu_task(const Legion::Task *task,
				       const std::vector<Legion::PhysicalRegion> &regions,
				       Legion::Context ctx, Legion::Runtime *runtime)
{
  //  std::cout << "  Running primary variable" << std::endl;
  return *((Return_t*) task->args);
}

template<typename Return_t>
Legion::TaskID TaskManagerPrimary<Return_t>::taskid = 0;



// implementation of Secondary
// ------------------------------------------------------------------

// template magic to pack a tuple of values from a vector of their futures
//
// read a future and return its result
template<typename T>
T readFuture(std::vector<Legion::Future>::const_iterator& f)
{
  
  T t = f->get_result<T>();
  f++;
  return std::move(t);
}

// read a vector of futures and return a tuple of their results
template<typename... Args>
std::tuple<Args...> futureToValue(std::vector<Legion::Future>::const_iterator f)
{
  return std::make_tuple(readFuture<Args>(f)...);
}


// Secondary methods
// ------------------------------------------------------------------
template<typename Return_t, typename Func_t, typename... Args>
void
TaskManagerSecondary<Return_t, Func_t, Args...>
::preregister_task(Legion::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
  	      Legion::Runtime::generate_static_task_id() :
	      new_taskid);
  std::cout << "Registering task: " << Func_t::name << std::endl;
  Legion::TaskVariantRegistrar tvr(taskid, Func_t::name);
  tvr.add_constraint(Legion::ProcessorConstraint(Legion::Processor::LOC_PROC));
  tvr.set_leaf(true);
  Legion::Runtime::preregister_task_variant<Return_t, &TaskManagerSecondary<double, Func_t,Args...>::cpu_task>(tvr, Func_t::name);
}


template<typename Return_t, typename Func_t, typename... Args>
Legion::Future
TaskManagerSecondary<Return_t,Func_t,Args...>
::compute(Legion::Context ctx, Legion::Runtime *runtime, const std::vector<Legion::Future>& futures, const Func_t& func)
{
  Legion::TaskLauncher launcher(taskid, Legion::TaskArgument(&func, sizeof(Func_t)));
  for (auto f : futures) launcher.add_future(f);
  return runtime->execute_task(ctx, launcher);
}


template<typename Return_t, typename Func_t, typename... Args>
Return_t
TaskManagerSecondary<Return_t,Func_t,Args...>
::cpu_task(const Legion::Task *task,
	   const std::vector<Legion::PhysicalRegion> &regions,
	   Legion::Context ctx, Legion::Runtime *runtime)
{
  assert(std::tuple_size<std::tuple<Args...> >::value == task->futures.size());
  const Func_t& func= *(const Func_t*)(task->args);
  auto values = futureToValue<Args...>(task->futures.begin());
  std::cout << "got a task with " << task->futures.size() << " elems" << std::endl;

  return Arcos::Magic::invoke<Return_t>(func,values);
}


template<typename Return_t, typename Func_t, typename... Args>
Legion::TaskID TaskManagerSecondary<Return_t,Func_t,Args...>::taskid = 0;




} // namespace
