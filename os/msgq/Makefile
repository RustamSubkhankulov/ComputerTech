all: objdir stad

objdir:
	mkdir -p obj

clean:
	rm obj/*.o stad

stad: main_ stad_
	gcc obj/main.o obj/stad.o -o stad

main_: src/main.cpp src/stad/stad.h
	gcc src/main.cpp -c -o obj/main.o

stad_: src/stad/stad.cpp src/stad/stad.h
	gcc src/stad/stad.cpp -c -o obj/stad.o

