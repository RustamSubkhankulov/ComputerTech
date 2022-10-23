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
CFLAGS   = -O2 -Wno-multichar

#==========================================================
# TARGETS
#==========================================================

all: objdir my_ls

#----------------------------------------------------------

my_ls: obj/main.o obj/my_ls.o obj/coloredo.o
	$(CC) $(^) -o $(@)

obj/main.o:  src/main.cpp inc/my_ls/my_ls.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/my_ls.o: src/my_ls/my_ls.cpp inc/my_ls/my_ls.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

obj/coloredo.o: src/coloredo/coloredo.cpp inc/coloredo/coloredo.h
	$(CC) $(<) -c -o $(@) $(CFLAGS)

#----------------------------------------------------------
# PHONY
#----------------------------------------------------------

.PHONY: objdir clean

objdir:
	mkdir -p obj

clean:
	-rm obj/*.o 
	-rm my_ls