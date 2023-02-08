all: exec_cat

##-----------------------------------------------

FLAGS = -g 

##-----------------------------------------------

exec_cat: obj/monitor.o obj/cat.o obj/main.o obj/rbuffer.o
	$(CC) $(^) -o $(@) $(FLAGS) -lpthread

##-----------------------------------------------

obj/monitor.o: monitor/monitor.c monitor/monitor.h rbuffer/rbuffer.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

obj/cat.o: cat/cat.c cat/cat.h monitor/monitor.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

obj/main.o: main.c cat/cat.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

obj/rbuffer.o: rbuffer/rbuffer.c rbuffer/rbuffer.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

##-----------------------------------------------

.PHONY: clean

clean:
	- rm exec_cat
	- rm obj/*.o
