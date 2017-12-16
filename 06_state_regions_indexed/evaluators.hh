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

#ifndef EVALUATORS_HH_
#define EVALUATORS_HH_

#include <string>
#include <set>
#include "state.hh"

namespace Arcos {

typedef std::string Key;
typedef std::set<Key> KeySet;
typedef std::vector<Key> KeyList;

//
// Virtual base class for evaluators
// =============================================================================
class Evaluator {
public:

  //
  // Update:
  //
  //   potentially update this evaluator, and return whether this
  //   evaluator's data has changed since the last time Update() was
  //   called by this requestor.
  // -------------------------------------------------------------------------------
  virtual bool Update(State& S, const Key& request) = 0;

  //
  // IsDependency:
  //
  //   Forms the dag -- is key a dependency of this evaluator?
  // -------------------------------------------------------------------------------
  virtual bool IsDependency(const Key& key) const = 0;

  //
  // ProvidesKey():
  //
  //   Forms the dag -- is this key provided by this evaluator?  Note
  //   this is posed this way to allow evaluators to evaluate MULTIPLE
  //   keys at the same time.  
  // -------------------------------------------------------------------------------
  virtual bool ProvidesKey(const Key& key) const = 0;
};


//
// Primary variable evaluators
// =============================================================================
template<typename TaskManager_t>
class EvaluatorPrimary : public Evaluator {
public:
  // constructor
  EvaluatorPrimary(const Key& key, double value)
    : key_(key),
      done_once_(false),
      value_(std::move(value)) {}
      
  // this does nothing except ensure an IC has been provided
  virtual bool Update(State& S, const Key& request) override;

  // primary variables have no dependencies
  virtual bool IsDependency(const Key& key) const override;

  // primary variables provide themselves only
  virtual bool ProvidesKey(const Key& key) const override;

protected:
  void Update_(State& S);
  
  Key key_;
  KeySet requests_;
  bool done_once_;
  double value_;
};


//
// Secondary variable evaluators
// =============================================================================
template<typename TaskManager_t, typename Function_t>
class EvaluatorSecondary : public Evaluator {
public:
  // constructor
  EvaluatorSecondary(Key key, KeyList deps, State& s)
    : key_(std::move(key)),
      dependencies_(std::move(deps)) {
    // make sure not self-referential
    assert(std::find(dependencies_.begin(), dependencies_.end(), key_) == dependencies_.end());
    for (auto dep : dependencies_) {
      s.RequireEvaluator(dep);
    }
    
  }

  // update if needed
  virtual bool Update(State& S, const Key& request) override;

  // is key in my list of dependencies?
  virtual bool IsDependency(const Key& key) const override;

  // is key my key?
  virtual bool ProvidesKey(const Key& key) const override;

protected:
  void Update_(State& S);

  Key key_;
  KeySet requests_;
  KeyList dependencies_;

  Function_t func_;
};



} // namespace Arcos  
  

#include "evaluators_impl.hh"


#endif
