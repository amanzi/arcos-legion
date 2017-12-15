//! --------------------------------------------------------------------------------
//
// Arcos -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
// An Evaluator is a node in the dependency graph, and represents a
// method of evaluating a function or algorithm on some set of data to
// provide some other set of data.
//
// Evaluators come in three main flavors:
//   * primary variable evaluators:
//	  leafs on the dag, these are what is being solved for
//   * independent variable evaluators:
//	  also leafs on the dag, these are data provided by the user
//   * secondary variable evaluators:
//	  interior nodes of the dag, these do some work to calculate their
//        provided variable
//
//
// Note this is a Legion-enabled mockup with some Arcos Evaluator
// functionality, but not all.
//
// ---------------------------------------------------------------------------------


namespace Arcos {


template<typename TaskManager_t>
bool
EvaluatorPrimary<TaskManager_t>::IsDependency(const Key& key) const {
  return false;
}


template<typename TaskManager_t>
bool
EvaluatorPrimary<TaskManager_t>::ProvidesKey(const Key& key) const {
  return key == key_;
}

template<typename TaskManager_t>
bool
EvaluatorPrimary<TaskManager_t>::Update(State& S, const Key& request) {
  std::cout << "Calling Primary::Update() on " << key_ << "..." << std::endl;
  if (!done_once_) {
    Update_(S);
    done_once_ = true;
  }

  if (requests_.find(request) == requests_.end()) {
    requests_.insert(request);
    return true;
  }
  return false;
}


// start the task, store the future
template<typename TaskManager_t>
void
EvaluatorPrimary<TaskManager_t>::Update_(State& S) {
  std::cout << "Launching Primary task for " << key_ << std::endl;
  TaskLauncher launcher(TaskManager_t::taskid, TaskArgument(&value_, sizeof(value_)));
  launcher.add_region_requirement(RegionRequirement(S.logical_region, WRITE_DISCARD, EXCLUSIVE, S.logical_region));
  launcher.add_field(0, S.field_ids[key_]);  
  S.futures[key_] = TaskManager_t::compute(S.ctx, S.runtime, launcher, value_);
}


// --------------------------------------------------------------------------------

template<typename TaskManager_t, typename Function_t>
bool
EvaluatorSecondary<TaskManager_t,Function_t>::IsDependency(const Key& key) const {
  return std::find(dependencies_.begin(), dependencies_.end(), key) != dependencies_.end();
}


template<typename TaskManager_t, typename Function_t>
bool
EvaluatorSecondary<TaskManager_t,Function_t>::ProvidesKey(const Key& key) const {
  return key == key_;
}

template<typename TaskManager_t, typename Function_t>
bool
EvaluatorSecondary<TaskManager_t,Function_t>::Update(State& S, const Key& request) {
  std::cout << "Calling Secondary::Update() on " << key_ << "..." << std::endl;
  bool update = false;

  for (auto dep : dependencies_)
    update |= S.evaluators[dep]->Update(S,key_);

  if (update) {
    Update_(S);
    requests_.clear();
    requests_.insert(request);
    return true;
  } else if (requests_.find(request) == requests_.end()) {
    requests_.insert(request);
    return true;
  } else {
    return false;
  }
}


template<typename TaskManager_t, typename Function_t>
void
EvaluatorSecondary<TaskManager_t,Function_t>::Update_(State& S) {
  std::cout << "Launching Secondary task for " << key_ << std::endl;

  TaskLauncher launcher(TaskManager_t::taskid, TaskArgument(NULL, 0));
  launcher.add_region_requirement(RegionRequirement(S.logical_region, WRITE_DISCARD, EXCLUSIVE, S.logical_region));
  launcher.add_field(0, S.field_ids[key_]);
  auto rr = RegionRequirement{S.logical_region, READ_ONLY, EXCLUSIVE, S.logical_region};
  std::vector<Legion::FieldID> rr_deps;
  for (auto dep : dependencies_) rr_deps.push_back(S.field_ids[dep]);
  rr.add_fields(rr_deps);
  launcher.add_region_requirement(rr);
  // for (auto dep : dependencies_) {
  //   std::cout << "      adding field " << S.field_ids[dep] << std::endl;
  //   launcher.add_field(1, S.field_ids[dep]);
  // }
  
  Future f = TaskManager_t::compute(S.ctx, S.runtime, launcher, func_);
  S.futures[key_] = f;
}


} // namespace
