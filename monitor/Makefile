all: my_cat

##-----------------------------------------------

FLAGS = -g 

##-----------------------------------------------

my_cat: obj/monitor.o obj/cat.o 
	$(CC) $(^) -o $(@) $(FLAGS)

##-----------------------------------------------

obj/monitor.o: monitor/monitor.c monitor/monitor.h 
	$(CC) $(>) -o $(@) $(FLAGS)

obj/cat.o: cat/cat.c cat/cat.h 
	$(CC) $(>) -o $(@) $(FLAGS)