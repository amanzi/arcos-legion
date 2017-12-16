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
template<typename Data_t>
void
TaskManagerPrimary<Data_t>::preregister_task(Legion::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
	    Legion::Runtime::generate_static_task_id() :
	      new_taskid);
  Legion::TaskVariantRegistrar tvr(taskid, "primary_variable");
  //  std::cout << "Registering task: primary_variable" << std::endl;
  tvr.add_constraint(Legion::ProcessorConstraint(Legion::Processor::LOC_PROC));
  tvr.set_leaf(true);
  Legion::Runtime::preregister_task_variant<&TaskManagerPrimary<Data_t>::cpu_task>(tvr, "primary_variable");
}


template<typename Data_t>
Legion::Future
TaskManagerPrimary<Data_t>::compute(Legion::Context ctx, Legion::Runtime *runtime,
        const Legion::TaskLauncher& launcher, const Data_t& value)
{
  return runtime->execute_task(ctx, launcher);
}


template<typename Data_t>
void
TaskManagerPrimary<Data_t>::cpu_task(const Legion::Task *task,
				       const std::vector<Legion::PhysicalRegion> &regions,
				       Legion::Context ctx, Legion::Runtime *runtime)
{
  std::cout << "Executing primary task...";
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  double val = *((double*) task->args);
  
  // This is a field polymorphic function so figure out
  // which field we are responsible for initializing.
  auto fid = *(task->regions[0].privilege_fields.begin());
  printf("Initializing (field %d) = %g\n", fid, val);

  const Legion::FieldAccessor<WRITE_DISCARD,double,1> acc(regions[0], fid);
  for (int i=0; i!=g_ncells; ++i) acc[i] = val;
}

template<typename Data_t>
Legion::TaskID TaskManagerPrimary<Data_t>::taskid = 0;



// implementation of Secondary
// ------------------------------------------------------------------

// Secondary methods
// ------------------------------------------------------------------
template<typename Func_t, typename... Args>
void
TaskManagerSecondary<Func_t, Args...>
::preregister_task(Legion::TaskID new_taskid)
{
  taskid = ((new_taskid == AUTO_GENERATE_ID) ?
  	      Legion::Runtime::generate_static_task_id() :
	      new_taskid);
  std::cout << "Registering task: " << Func_t::name << std::endl;
  Legion::TaskVariantRegistrar tvr(taskid, Func_t::name);
  tvr.add_constraint(Legion::ProcessorConstraint(Legion::Processor::LOC_PROC));
  tvr.set_leaf(true);
  Legion::Runtime::preregister_task_variant<&TaskManagerSecondary<Func_t,Args...>::cpu_task>(tvr, Func_t::name);
}


template<typename Func_t, typename... Args>
Legion::Future
TaskManagerSecondary<Func_t,Args...>
::compute(Legion::Context ctx, Legion::Runtime *runtime, const Legion::TaskLauncher& launcher,
          const Func_t& func)
{
  return runtime->execute_task(ctx, launcher);
}



template<typename Accessor_iter_t, typename T>
T readAccessor(Accessor_iter_t& a, int i)
{
  T t = (*a)[i];
  a++;
  return std::move(t);
}

// read a vector of futures and return a tuple of their results
template<typename Accessor_iter_t, typename... Args>
std::tuple<Args...> accessorsToValues(Accessor_iter_t a, int i)
{
  return std::make_tuple(readAccessor<Accessor_iter_t,Args>(a,i)...);
}


template<typename Func_t, typename... Args>
void
TaskManagerSecondary<Func_t,Args...>
::cpu_task(const Legion::Task *task,
	   const std::vector<Legion::PhysicalRegion> &regions,
	   Legion::Context ctx, Legion::Runtime *runtime)
{
  std::cout << "Executing secondary task...";
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == std::tuple_size<std::tuple<Args...> >::value);

  // get the function
  const Func_t& func= *(const Func_t*)(task->args);

  // get a list of accessors for the argument
  std::vector<Legion::FieldAccessor<READ_ONLY,double,1>> fas_in;
  std::cout << " depending upon FIDs: ";
  for (auto fid : task->regions[1].privilege_fields) {
    std::cout << (int) fid << " ";
    fas_in.emplace_back(Legion::FieldAccessor<READ_ONLY,double,1>(regions[1], fid));
  }

  // get the accessor for the output
  const Legion::FieldAccessor<WRITE_DISCARD,double,1> fa_out(regions[0], *task->regions[0].privilege_fields.begin());

  // iterate and invoke the function
  for (int i=0; i!=g_ncells; ++i) {
    auto values = accessorsToValues<std::vector<Legion::FieldAccessor<READ_ONLY,double,1>>::const_iterator, Args...>(fas_in.begin(), i);
    fa_out[i] = Arcos::Magic::invoke<double>(func, values);
    std::cout << ", got: " << (double) fa_out[i] << std::endl;
  }
}


template<typename Func_t, typename... Args>
Legion::TaskID TaskManagerSecondary<Func_t,Args...>::taskid = 0;




} // namespace
