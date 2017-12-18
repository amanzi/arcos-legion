//! --------------------------------------------------------------------------------
//
// Arcos -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
// A mockup of State for testing with Legion
//
// ---------------------------------------------------------------------------------

#include <iostream>
#include "evaluators.hh"
#include "evaluator_factory.hh"
#include "state.hh"
#include "default_mapper.h"

namespace Arcos {

void
State::report() {
  std::cout << "State Report:" << std::endl
	    << "-------------" << std::endl;
}

void
State::RequireEvaluator(const std::string& eval_type) {
  std::cout << "Evaluator Required: " << eval_type;
  if (evaluators.count(eval_type) == 0) {
    Evaluator_Factory fac;
    evaluators[eval_type] = fac.Create(eval_type, *this);
    field_ids[eval_type] = n_fids;
    n_fids++;
  } else {
    std::cout << "  ...already have one." << std::endl;
  }
}



void
State::Setup() {
  // create the logical region to hold all data
  // -- create the index space
  Legion::IndexSpace untyped_is = runtime->create_index_space(ctx, domain);
  printf("State Setup:\n  Created untyped index space %x\n", untyped_is.get_id());

  // -- create the field space
  Legion::FieldSpace fs = runtime->create_field_space(ctx);
  printf("  Created field space field space %x\n", fs.get_id());
  {
    Legion::FieldAllocator allocator = runtime->create_field_allocator(ctx, fs);
    for (auto fid : field_ids) {
      allocator.allocate_field(sizeof(double), fid.second);
    }
  }

  // -- form the cross product to create the LR
  logical_region = runtime->create_logical_region(ctx, untyped_is, fs);
  runtime->attach_name(logical_region, "state lr");
  printf("  Created untyped logical region (%x,%x,%x)\n",
      logical_region.get_index_space().get_id(), 
      logical_region.get_field_space().get_id(),
      logical_region.get_tree_id());

  // -- form the partitioning
  // create the partitioning
  int num_subregions =
      runtime->select_tunable_value(ctx, Legion::Mapping::DefaultMapper::DEFAULT_TUNABLE_GLOBAL_CPUS,
                                  0).get_result<size_t>();
  printf("Partitioning data into %d sub-regions...\n", num_subregions);

  Legion::Rect<1> color_bounds(0, num_subregions-1);
  partition = runtime->create_index_space(ctx, color_bounds);
  runtime->attach_name(partition, "state partition");

  Legion::IndexPartition ip = runtime->create_equal_partition(ctx, untyped_is, partition);

  logical_partition = runtime->get_logical_partition(ctx, logical_region, ip);
  runtime->attach_name(ip, "state partition");

  // clean up?
  printf("  Cleaning up FS and IS\n");
  runtime->destroy_field_space(ctx, fs);
  runtime->destroy_index_space(ctx, untyped_is);
  printf("  Setup Completed!\n");
};

State::~State() {
  runtime->destroy_logical_region(ctx, logical_region);
  runtime->destroy_index_space(ctx, partition);
}



} // namespace Arcos
