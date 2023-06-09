CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror
ALL = zombie controller labo

all: $(ALL)

zombie : zombie.o utils_v2.o zombie.h
	$(CC) $(CCFLAGS) -o zombie zombie.o utils_v2.o

zombie.o: zombie.c utils_v2.h zombie.h
	$(CC) $(CCFLAGS) -c zombie.c 

controller : controller.o utils_v2.o controller.h
	$(CC) $(CCFLAGS) -o controller controller.o utils_v2.o	

controller.o: controller.c utils_v2.h controller.h
	$(CC) $(CCFLAGS) -c controller.c 

labo : labo.o utils_v2.o labo.h
	$(CC) $(CCFLAGS) -o labo labo.o utils_v2.o	

labo.o: labo.c utils_v2.h labo.h
	$(CC) $(CCFLAGS) -c labo.c 

utils_v2.o: utils_v2.c utils_v2.h
	$(CC) $(CCFLAGS) -c utils_v2.c 

clear: clean

clean : 
	rm -f *.o
	rm -f $(ALL)
