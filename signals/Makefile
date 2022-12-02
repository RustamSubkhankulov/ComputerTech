all: exec_cat

##-----------------------------------------------

FLAGS = -g 

##-----------------------------------------------

exec_cat: obj/sigsend.o obj/cat.o obj/main.o
	$(CC) $(^) -o $(@) $(FLAGS)

##-----------------------------------------------

obj/sigsend.o: src/sigsend/sigsend.c inc/sigsend/sigsend.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

obj/cat.o: src/cat/cat.c inc/cat/cat.h inc/sigsend/sigsend.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

obj/main.o: main.c inc/cat/cat.h
	$(CC) $(<) -c -o $(@) $(FLAGS)

##-----------------------------------------------

.PHONY: clean

clean:
	- rm exec_cat
	- rm obj/*.o
