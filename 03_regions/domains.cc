

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
};

enum FieldIDs {
  FID_A,
  FID_B,
  FID_C,
  FID_D,
  FID_E,
  FID_F,
  FID_G
};

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, Runtime *runtime)
{
  // Logical regions are the core abstraction of data in Legion. 
  // Constructing a logical region involves specifying an index
  // space and a field space.  We describe how to create these
  // building blocks first and then show how to create logical
  // regions from them.

  // Index spaces are the abstraction in Legion that is used to
  // describe the row entries in a logical region.  There are two
  // kinds of index spaces in Legion: unstructured and structured.
  // Both are created using the 'create_index_space' runtime
  // call with different parameters.  Both types are represented
  // by the same IndexSpace type.  Unstructured index spaces are 
  // created by specifying the context and the maximum number of
  // elements that may be allocated in the index space.  We note
  // that specifying the upper bound on elements is mildly
  // restrictive but it significantly simplifies and improves
  // performance of our runtime implementation.  Furthermore most
  // programmers usually have an approximation of how big their
  // data sets are, or can at least approximate it based on some 
  // input parameter.  If you have a sufficiently complex example
  // which requires an unbounded number of elements we would be
  // interested in learning more.  Here we create an unstructured
  // index space that will store at most 1024 elements.
  int ncells = 4;

  const Domain domain(DomainPoint(0), DomainPoint(ncells-1));
  IndexSpace untyped_is = runtime->create_index_space(ctx, domain); 
  printf("Created untyped index space %x\n", untyped_is.get_id());

  // Fields spaces are the abstraction that Legion uses for describing
  // the column entries in a logical region.  Field spaces are created
  // using the 'create_field_space' call.
  FieldSpace fs = runtime->create_field_space(ctx);
  printf("Created field space field space %x\n", fs.get_id());

  // Fields can be dynamically allocated and destroyed in fields spaces
  // using field allocators.  For performance reasons there is a compile-time
  // upper bound placed on the maximum number of fields that can be
  // allocated in a field space at a time (see 'MAX_FIELDS' at the
  // top of legion_types.h).  If a program exceeds this maximum then
  // the Legion runtime will report an error and exit.
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
  }

  // Logical regions are created by passing an index space and a field
  // space to the 'create_logical_region' runtime method.  Note that
  // both structured and unstructured index spaces can be used.
  // Note that we use the same field space for both logical regions
  // which means they both will have the same set of fields.  Any
  // modifications to the field space will effect both logical regions.
  LogicalRegion state_lr = 
    runtime->create_logical_region(ctx, untyped_is, fs);
  printf("Created untyped logical region (%x,%x,%x)\n",
      state_lr.get_index_space().get_id(), 
      state_lr.get_field_space().get_id(),
      state_lr.get_tree_id());


  // Now make the physical region: need region requirements.  Since this is main, most permissive?
  RegionRequirement req(state_lr, READ_WRITE, EXCLUSIVE, state_lr);

  // We also need to specify which fields we plan to access in our
  // RegionRequirement.  To do this we invoke the 'add_field' method
  // on the RegionRequirement.
  req.add_field(FID_A);
  req.add_field(FID_B);
  req.add_field(FID_C);
  req.add_field(FID_D);
  req.add_field(FID_E);
  req.add_field(FID_F);
  req.add_field(FID_G);
  InlineLauncher region_launcher(req);
  PhysicalRegion r = runtime->map_region(ctx, region_launcher);
  r.wait_until_valid();

  // set up our ICs
  const FieldAccessor<READ_WRITE,double,1> acc_a(r, FID_A);
  for (int i=0; i!=ncells; ++i) {
    acc_a[i] = 2.0;
  }
  
  // The runtime also supports operations for destroying index spaces,
  // field spaces, and logical regions.
  runtime->destroy_logical_region(ctx, state_lr);
  runtime->destroy_field_space(ctx, fs);
  runtime->destroy_index_space(ctx, untyped_is);
  printf("Successfully cleaned up all of our resources\n");
}

int main(int argc, char **argv)
{
  Runtime::set_top_level_task_id(TOP_LEVEL_TASK_ID);

  {
    TaskVariantRegistrar registrar(TOP_LEVEL_TASK_ID, "top_level");
    registrar.add_constraint(ProcessorConstraint(Processor::LOC_PROC));
    Runtime::preregister_task_variant<top_level_task>(registrar, "top_level");
  }

  return Runtime::start(argc, argv);
}

