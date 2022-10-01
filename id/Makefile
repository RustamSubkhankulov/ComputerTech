all: objdir my_id

objdir:
	mkdir -p obj

clean:
	rm obj/*.o my_id

my_id: main_ my_id_
	gcc obj/main.o obj/my_id.o -o my_id

main_: src/main.cpp include/my_id/my_id.h
	gcc src/main.cpp -c -o obj/main.o

my_id_: src/my_id/my_id.cpp include/my_id/my_id.h
	gcc src/my_id/my_id.cpp -c -o obj/my_id.o

