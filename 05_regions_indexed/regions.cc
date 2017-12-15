
#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include "legion.h"
using namespace Legion;

/*
 * This example shows how to create index
 * spaces, field spaces, and logical regions.
 * It also shows how to dynamically allocate
 * and free elements in index spaces and fields
 * in field spaces.
 */

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
  TID_A,
  TID_B,
  TID_C,
  TID_D,
  TID_E,
  TID_F,
  TID_G,
  TID_H,
  TID_TEST
};

enum FieldIDs {
  FID_A,
  FID_B,
  FID_C,
  FID_D,
  FID_E,
  FID_F,
  FID_G,
  FID_H
};

static const int g_ncells = 20;  // will need to figure this out sometime...
static const int g_npartitions = 4;  // will need to figure this out sometime...

// Note, since all results are stored in regions, all tasks return an error
// code.

// primaries
void BEvaluator(const Task *task,
                const std::vector<PhysicalRegion> &regions,
                Context ctx, Runtime *runtime)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  // This is a field polymorphic function so figure out
  // which field we are responsible for initializing.
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  printf("Initializing B(field %d) = 2.0", fid);
  assert(fid == FID_B);

  const FieldAccessor<WRITE_DISCARD,double,1> acc(regions[0], fid);
  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  int count = 0;
  for (PointInRectIterator<1> p(domain); p(); p++) {
    acc[*p] = 2.;
    count++;
  }

  printf(" on %d cells\n",count);
}


void GEvaluator(const Task *task,
                const std::vector<PhysicalRegion> &regions,
                Context ctx, Runtime *runtime)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  // This is a field polymorphic function so figure out
  // which field we are responsible for initializing.
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  printf("Initializing G (field %d) = 3.0\n", fid);
  assert(fid == FID_G);

  const FieldAccessor<WRITE_DISCARD,double,1> acc(regions[0], fid);
  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) acc[*p] = 3.;
}


// secondaries
void AEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 4);

  // in
  const FieldAccessor<READ_ONLY,double,1> fb(regions[1], FID_B);
  const FieldAccessor<READ_ONLY,double,1> fc(regions[1], FID_C);
  const FieldAccessor<READ_ONLY,double,1> fe(regions[1], FID_E);
  const FieldAccessor<READ_ONLY,double,1> fh(regions[1], FID_H);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> fa(regions[0], FID_A);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) fa[*p] = 2*fb[*p] + fc[*p]*fe[*p]*fh[*p];
  printf("Evaluating A = %g\n", (double) fa[0]);
}


void CEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 2);

  // in
  const FieldAccessor<READ_ONLY,double,1> fd(regions[1], FID_D);
  const FieldAccessor<READ_ONLY,double,1> fg(regions[1], FID_G);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> fc(regions[0], FID_C);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) fc[*p] = 2 * fd[*p] + fg[*p];
  printf("Evaluating C = %g\n", (double) fc[0]);
}


void DEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);

  // in
  const FieldAccessor<READ_ONLY,double,1> fg(regions[1], FID_G);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> fd(regions[0], FID_D);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) fd[*p] = 2 * fg[*p];
  printf("Evaluating D = %g\n", (double) fd[0]);
}

void EEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 2);

  // in
  const FieldAccessor<READ_ONLY,double,1> fd(regions[1], FID_D);
  const FieldAccessor<READ_ONLY,double,1> ff(regions[1], FID_F);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> fe(regions[0], FID_E);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) fe[*p] = fd[*p] * ff[*p];
  printf("Evaluating E = %g\n", (double) fe[0]);
}

void FEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);

  // in
  const FieldAccessor<READ_ONLY,double,1> fg(regions[1], FID_G);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> ff(regions[0], FID_F);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) ff[*p] = 2 * fg[*p];
  printf("Evaluating F = %g\n", (double) ff[0]);
}

void HEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 2);
  assert(task->regions.size() == 2);
  assert(task->regions[0].privilege_fields.size() == 1);
  assert(task->regions[1].privilege_fields.size() == 1);

  // in
  const FieldAccessor<READ_ONLY,double,1> ff(regions[1], FID_F);

  // out
  const FieldAccessor<WRITE_DISCARD,double,1> fh(regions[0], FID_H);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  for (PointInRectIterator<1> p(domain); p(); p++) fh[*p] = 2 * ff[*p];
  printf("Evaluating H = %g\n", (double) fh[0]);
}


void TestEvaluator(const Task *task,
		  const std::vector<PhysicalRegion> &regions,
		  Context ctx, Runtime *runtime)
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  // in
  const FieldAccessor<READ_ONLY,double,1> fa(regions[0], FID_A);

  auto domain = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  int count = 0;
  for (PointInRectIterator<1> p(domain); p(); p++) {
    assert(std::abs(fa[*p] - 6484.0) < 1.e-10);
    count++;
  }
  printf("Successful test on %d cells!\n", count);
}


void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, Runtime *runtime)
{
  std::cout << "Top Level Task" << std::endl;

  // create a domain and index space of size g_ncells
  const Rect<1> domain(DomainPoint(0), DomainPoint(g_ncells-1));
  IndexSpace untyped_is = runtime->create_index_space(ctx, domain); 
  runtime->attach_name(untyped_is, "domain");
  printf("Created untyped index space %x\n", untyped_is.get_id());

  // create a fieldspace to include all data -- this is an implemntation
  // choice, could do one field per fieldspace, could do (most) in one, etc.
  // May need to have one with all "cell-only" vectors, one with all
  // "cell+face" vectors, etc?  Need to get some advice here!
  FieldSpace fs = runtime->create_field_space(ctx);
  runtime->attach_name(fs, "state_fs");
  printf("Created field space field space %x\n", fs.get_id());
  {
    FieldAllocator allocator = runtime->create_field_allocator(ctx, fs);
    // When fields are allocated they must specify the size of the data
    // to be stored in the field in bytes.  Users may also optionally
    // specify the ID for the field being allocated.  If this is done,
    // the user is responsible for ensuring that each field ID is used
    // only once for a each field space.  Legion support parallel allocation
    // of fields in the same field space, but it will result in undefined
    // behavior if two fields are allocated in the same field space at
    // the same time with the same user provided ID.
    auto a = allocator.allocate_field(sizeof(double), FID_A); assert(a == FID_A);
    auto b = allocator.allocate_field(sizeof(double), FID_B); assert(b == FID_B);
    auto c = allocator.allocate_field(sizeof(double), FID_C); assert(c == FID_C);
    auto d = allocator.allocate_field(sizeof(double), FID_D); assert(d == FID_D);
    auto e = allocator.allocate_field(sizeof(double), FID_E); assert(e == FID_E);
    auto f = allocator.allocate_field(sizeof(double), FID_F); assert(f == FID_F);
    auto g = allocator.allocate_field(sizeof(double), FID_G); assert(g == FID_G);
    auto h = allocator.allocate_field(sizeof(double), FID_H); assert(h == FID_H);
  }

  // Logical region is the cross product of IndexSpace and FieldSpace
  LogicalRegion state_lr = 
    runtime->create_logical_region(ctx, untyped_is, fs);
  runtime->attach_name(state_lr, "state");
  printf("Created untyped logical region (%x,%x,%x)\n",
      state_lr.get_index_space().get_id(), 
      state_lr.get_field_space().get_id(),
      state_lr.get_tree_id());

  // create the partitioning
  Rect<1> color_bounds(0, g_npartitions-1);
  IndexSpace color_is = runtime->create_index_space(ctx, color_bounds);
  runtime->attach_name(color_is, "coloring is");

  IndexPartition ip = runtime->create_equal_partition(ctx, untyped_is, color_is);
  runtime->attach_name(ip, "ip");

  LogicalPartition state_lp = runtime->get_logical_partition(ctx, state_lr, ip);
  runtime->attach_name(state_lp, "state_lp");
  
  // WTF we never make a physical region? --etc
  // // Now make the physical region: need region requirements.  Since this is
  // // main, most permissive?
  // RegionRequirement req(state_lr, READ_WRITE, EXCLUSIVE, state_lr);
  // req.add_field(FID_A);
  // req.add_field(FID_B);
  // req.add_field(FID_C);
  // req.add_field(FID_D);
  // req.add_field(FID_E);
  // req.add_field(FID_F);
  // req.add_field(FID_G);
  // req.add_field(FID_H);
  // InlineLauncher region_launcher(req);

  // // make the physical region
  // PhysicalRegion r = runtime->map_region(ctx, region_launcher);
  // r.wait_until_valid();

  
  ArgumentMap arg_map;
  
  // launch task for B IC
  std::cout << "Launching B" << std::endl;
  IndexLauncher Blauncher(TID_B, color_is, TaskArgument(NULL, 0), arg_map);
  Blauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Blauncher.add_field(0,FID_B);
  runtime->execute_index_space(ctx, Blauncher);

  // launch task for G IC
  std::cout << "Launching G" << std::endl;
  IndexLauncher Glauncher(TID_G, color_is, TaskArgument(NULL, 0), arg_map);
  Glauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Glauncher.add_field(0,FID_G);
  runtime->execute_index_space(ctx, Glauncher);

  // second level launches
  std::cout << "Launching D" << std::endl;
  IndexLauncher Dlauncher(TID_D, color_is, TaskArgument(NULL, 0), arg_map);
  Dlauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Dlauncher.add_field(0, FID_D); // 0 refers to the 0th region requirement?
  Dlauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Dlauncher.add_field(1, FID_G); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Dlauncher);

  std::cout << "Launching F" << std::endl;
  IndexLauncher Flauncher(TID_F, color_is, TaskArgument(NULL, 0), arg_map);
  Flauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Flauncher.add_field(0, FID_F); // 0 refers to the 0th region requirement?
  Flauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Flauncher.add_field(1, FID_G); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Flauncher);

  // third level launches
  std::cout << "Launching C" << std::endl;
  IndexLauncher Clauncher(TID_C, color_is, TaskArgument(NULL, 0), arg_map);
  Clauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Clauncher.add_field(0, FID_C); // 0 refers to the 0th region requirement?
  Clauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Clauncher.add_field(1, FID_D); // 0 refers to the 0th region requirement?
  Clauncher.add_field(1, FID_G); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Clauncher);
  
  // third level launches
  std::cout << "Launching E" << std::endl;
  IndexLauncher Elauncher(TID_E, color_is, TaskArgument(NULL, 0), arg_map);
  Elauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Elauncher.add_field(0, FID_E); // 0 refers to the 0th region requirement?
  Elauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Elauncher.add_field(1, FID_D); // 0 refers to the 0th region requirement?
  Elauncher.add_field(1, FID_F); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Elauncher);

  std::cout << "Launching H" << std::endl;
  IndexLauncher Hlauncher(TID_H, color_is, TaskArgument(NULL, 0), arg_map);
  Hlauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Hlauncher.add_field(0,FID_H);
  Hlauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Hlauncher.add_field(1, FID_F); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Hlauncher);

  // fourth level launches
  std::cout << "Launching A" << std::endl;
  IndexLauncher Alauncher(TID_A, color_is, TaskArgument(NULL, 0), arg_map);
  Alauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, WRITE_DISCARD, EXCLUSIVE, state_lr));
  Alauncher.add_field(0,FID_A);
  Alauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Alauncher.add_field(1, FID_B); // 0 refers to the 0th region requirement?
  Alauncher.add_field(1, FID_C); // 0 refers to the 0th region requirement?
  Alauncher.add_field(1, FID_E); // 0 refers to the 0th region requirement?
  Alauncher.add_field(1, FID_H); // 0 refers to the 0th region requirement?
  runtime->execute_index_space(ctx, Alauncher);


  // Get the A result and check to make sure it worked!
  std::cout << "Launching Test Check Answer" << std::endl;
  IndexLauncher Tlauncher(TID_TEST, color_is, TaskArgument(NULL, 0), arg_map);
  Tlauncher.add_region_requirement(
      RegionRequirement(state_lp, 0, READ_ONLY, EXCLUSIVE, state_lr));
  Tlauncher.add_field(0,FID_A);
  runtime->execute_index_space(ctx, Tlauncher);
  
  // immediately destroy (note this doesn't actually destroy here, through a
  // shared_ptr or some other reference counting mechanism)
  runtime->destroy_logical_region(ctx, state_lr);
  runtime->destroy_field_space(ctx, fs);
  runtime->destroy_index_space(ctx, untyped_is);
  printf("Successfully cleaned up all of our resources\n");
}


// Main just calls top level task
int main(int argc, char **argv)
{
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);

  {
    TaskVariantRegistrar registrar(TOP_LEVEL_TASK_ID, "top_level");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<top_level_task>(registrar, "top_level");
  }

  {
    TaskVariantRegistrar registrar(TID_A, "AEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<AEvaluator>(registrar, "AEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_A, "AEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<AEvaluator>(registrar, "AEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_B, "BEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<BEvaluator>(registrar, "BEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_C, "CEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<CEvaluator>(registrar, "CEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_D, "DEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<DEvaluator>(registrar, "DEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_E, "EEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<EEvaluator>(registrar, "EEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_F, "FEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<FEvaluator>(registrar, "FEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_G, "GEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<GEvaluator>(registrar, "GEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_H, "HEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<HEvaluator>(registrar, "HEvaluator");
  }

  {
    TaskVariantRegistrar registrar(TID_TEST, "TestEvaluator");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    registrar.set_leaf();
    Runtime::preregister_task_variant<TestEvaluator>(registrar, "TestEvaluator");
  }
  
  return Runtime::start(argc, argv);
}

