LDFLAGS = -lm
CFLAGS= -O3 -Wall
CXXFLAGS= -O3 -Wall
CC=mpicxx
RM=rm

PROGRAM_NAME= mandelbox

all: $(PROGRAM_NAME)
	#mpirun -np 32 ./$(PROGRAM_NAME)

$(PROGRAM_NAME): main.o print.o timing.o savebmp.o getparams.o 3d.o getcolor.o distance_est.o mandelboxde.o raymarching.o renderer.o init3D.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

run-serial: $(PROGRAM_NAME)
	./$(PROGRAM_NAME)$(EXEXT) params.dat

clean:
	$(RM) *.o $(PROGRAM_NAME)$(EXEEXT) *~
