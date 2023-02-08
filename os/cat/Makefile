all: objdir my_cat

objdir:
	mkdir -p obj

clean:
	rm obj/*.o my_cat

my_cat: main my_cat_
	gcc obj/main.o obj/my_cat.o -o my_cat

main: main.cpp src/my_cat/my_cat.h
	gcc main.cpp -c -o obj/main.o

my_cat_: src/my_cat/my_cat.cpp src/my_cat/my_cat.h
	gcc src/my_cat/my_cat.cpp -c -o obj/my_cat.o

