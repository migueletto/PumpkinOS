include ../common.mak

PROGRAM=$(BIN)/libbox2d$(SOEXT)

OBJS=aabb.o array.o bitset.o body.o broad_phase.o constraint_graph.o contact.o contact_solver.o core.o \
     distance.o distance_joint.o dynamic_tree.o geometry.o hull.o id_pool.o island.o joint.o manifold.o \
     math_functions.o motor_joint.o mouse_joint.o prismatic_joint.o revolute_joint.o shape.o solver.o \
     solver_set.o stack_allocator.o table.o timer.o types.o weld_joint.o wheel_joint.o world.o

CUSTOMFLAGS=-I.

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
