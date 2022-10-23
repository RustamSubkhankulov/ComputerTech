#==========================================================
#CONFIG
#==========================================================

# compilers
CXX  = g++
CC   = gcc

#==========================================================
# FLAGS
#==========================================================

# flags for c++ 
CXXFLAGS = -O2 

# flags for c
CFLAGS   = -O0 -Wno-multichar -g

#==========================================================
# TARGETS
#==========================================================

all: objdir my_shell

#----------------------------------------------------------

my_shell: obj/main.o obj/my_shell.o obj/parser.o obj/utility.o obj/argvec.o
	$(CC) $(^) -o $(@)

obj/main.o:  src/main.cpp inc/my_shell/my_shell.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/my_shell.o: src/my_shell/my_shell.cpp inc/my_shell/my_shell.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/parser.o: src/parser/parser.cpp inc/parser/parser.h 
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/utility.o: src/utility/utility.cpp inc/utility/utility.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/argvec.o: src/argvec/argvec.cpp inc/argvec/argvec.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

#----------------------------------------------------------
# PHONY
#----------------------------------------------------------

.PHONY: objdir clean

objdir:
	mkdir -p obj

clean:
	-rm obj/*.o 
	-rm my_shell