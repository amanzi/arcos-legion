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

#ifndef ARCOS_TASK_MANAGERS_HH_
#define ARCOS_TASK_MANAGERS_HH_

#include "legion.h"
#include "functions.hh"
#include "template_magic.hh"

namespace LHL = LegionRuntime::HighLevel;

namespace Arcos {


//
// A task manager for primary variables
// =============================================================================
template<typename Return_t>
struct TaskManagerPrimary {
  static Legion::TaskID taskid;
  static void preregister_task(Legion::TaskID new_taskid = AUTO_GENERATE_ID);
  static Legion::Future compute(Legion::Context ctx, Legion::Runtime *runtime, const Return_t& value);
  static Return_t cpu_task(const Legion::Task *task,
			   const std::vector<Legion::PhysicalRegion> &regions,
			   Legion::Context ctx, Legion::Runtime *runtime);
};


//
// A task manager for secondary variables
// =============================================================================
template<typename Return_t, typename Func_t, typename... Args>
struct TaskManagerSecondary {
  static Legion::TaskID taskid;
  static void preregister_task(Legion::TaskID new_taskid = AUTO_GENERATE_ID);
  static Legion::Future compute(Legion::Context ctx, Legion::Runtime *runtime,
			     const std::vector<Legion::Future>& futures,
			     const Func_t& func);
  static Return_t cpu_task(const Legion::Task *task,
			   const std::vector<Legion::PhysicalRegion> &regions,
			   Legion::Context ctx, Legion::Runtime *runtime);
};


} // namespace

#include "task_managers_impl.hh"



#endif
