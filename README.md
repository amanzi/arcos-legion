# Arcos using Legion

This is a set of micro-apps based on Legion's tutorials that start to
explore how Arcos can be adopted to use Legion.

Each subdirectory is a standalone app which explores some aspect of
the Legion runtime within the context of Arcos.  Nearly all of these
examples implements the same DAG structure of Amanzi's test,

$AMANZI_SRC_DIR/src/state/test/state_dag.cc

doing it in a variety of ways.


Note these map into the CANGA tasks under the milestone Develop Arcos
micro-app as a "learning Legion" exercise:

https://canga.teamwork.com/#tasklists/750559

## 0. tasks

This is a simple hard-coding of setting up tasks evaluating the models
in state_dag for A-G evaluators.  The main task is blocking in the
sense that it explicitly calls each task, waits to get the result,
then passes the result in by reference to the next task up the DAG.

## 1. futures

This is tasks, but uses futures to allow the entire DAG of tasks to be
launched prior to any individual task being executed.

## 2. state on doubles

This lays out a first micro-app of Arcos for the test, using tasks,
futures, and evaluators within a State object.  No regions yet -- each
task works on a double still.


## 3. regions

This is test 01_futures, but instead of using doubles, using Regions.
This requires permissions to be mapped out, and allocators/launchers/index sets.


## 4. state on regions


