PREFIX=/usr/local
CC=g++
CFLAGS=-Wall 
XFLAGS=-Wall -lXss -lXext -lX11

all: x devinput sim

x: xmain.o FlexTimeTracker.o FlexConfiguration.o CommonUI.o
	${CC} $(XFLAGS) xmain.o FlexConfiguration.o FlexTimeTracker.o CommonUI.o -o XFlexTimeTool

devinput: FlexTimeTool.o FlexTimeTracker.o FlexConfiguration.o CommonUI.o
	${CC}  FlexTimeTool.o FlexConfiguration.o FlexTimeTracker.o CommonUI.o -o FlexTimeTool

sim: simulation.o FlexTimeTracker.o FlexConfiguration.o CommonUI.o
	${CC}  simulation.o FlexConfiguration.o FlexTimeTracker.o CommonUI.o -o simulation

FlexTimeTracker.o: FlexTimeTracker.cxx FlexTimeTracker.h
	${CC} $(CFLAGS) -c FlexTimeTracker.cxx

FlexConfiguration.o: FlexConfiguration.cxx FlexConfiguration.h
	${CC} $(CFLAGS) -c FlexConfiguration.cxx

FlexTimeTool.o: FlexTimeTool.cxx FlexTimeTracker.h
	${CC} $(CFLAGS) -c FlexTimeTool.cxx

simulation.o: simulation.cxx FlexTimeTracker.h
	${CC} $(CFLAGS) -c simulation.cxx

CommonUI.o: CommonUI.cxx FlexTimeTracker.h
	${CC} $(CFLAGS) -c CommonUI.cxx

xmain.o: xmain.cxx FlexTimeTracker.h
	${CC} $(CFLAGS) -c xmain.cxx

clean: 
	rm -f *.o XFlexTimeTool FlexTimeTool simulation

