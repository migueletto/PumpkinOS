include ../common.mak

PROGRAM=$(BIN)/libchipmunk$(SOEXT)

OBJS=chipmunk.o cpArbiter.o cpArray.o cpBBTree.o cpBody.o cpCollision.o cpConstraint.o cpDampedRotarySpring.o cpDampedSpring.o \
     cpGearJoint.o cpGrooveJoint.o cpHashSet.o cpHastySpace.o cpMarch.o cpPinJoint.o cpPivotJoint.o cpPolyShape.o cpPolyline.o \
     cpRatchetJoint.o cpRobust.o cpRotaryLimitJoint.o cpShape.o cpSimpleMotor.o cpSlideJoint.o cpSpace.o cpSpaceComponent.o \
     cpSpaceDebug.o cpSpaceHash.o cpSpaceQuery.o cpSpaceStep.o cpSpatialIndex.o cpSweep1D.o

CUSTOMFLAGS=-I. -DNDEBUG

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
