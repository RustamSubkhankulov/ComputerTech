all: objdir my_cp

objdir:
	mkdir -p obj

clean:
	rm obj/*.o my_cp

my_cp: main_ my_cp_
	gcc obj/main.o obj/my_cp.o -o my_cp

main_: src/main.cpp src/my_cp/my_cp.h
	gcc src/main.cpp -c -o obj/main.o

my_cp_: src/my_cp/my_cp.cpp src/my_cp/my_cp.h
	gcc src/my_cp/my_cp.cpp -c -o obj/my_cp.o

