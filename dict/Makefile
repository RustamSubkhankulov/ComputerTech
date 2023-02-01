#==========================================================
#CONFIG
#==========================================================

# compilers
CXX  = g++
CC   = gcc

# object files output directory
OBJ_DIR = obj

# bool flag (0 or 1) - shows whether use flags ot not
USE_DEBUG_FLAGS = 0

#==========================================================
# FLAGS
#==========================================================

# flags for c++ 
CXXFLAGS = -O3

# flags for c
CFLAGS   = -O3

#----------------------------------------------------------

ifeq ($(USE_DEBUG_FLAGS), 1)

	DEBUG_FLAGS = 												\
	-lubsan -D NDEBUG -g -std=c++20 -fmax-errors=1 				\
	-Wall -Wextra -Weffc++ -Waggressive-loop-optimizations  	\
	-Wc++0x-compat -Wc++11-compat -Wc++14-compat  				\
	-Wcast-qual -Wchar-subscripts -Wconditionally-supported 	\
	-Wconversion  -Wctor-dtor-privacy -Wempty-body 				\
	-Wfloat-equal -Wformat-nonliteral -Wformat-security 		\
	-Wformat-signedness -Wformat=2 -Winline  -Wlogical-op 		\
	-Wmissing-declarations  -Wnon-virtual-dtor -Wopenmp-simd	\
	-Woverloaded-virtual -Wpacked -Wpointer-arith 				\
	-Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo	\
	-Wstack-usage=8192  -Wstrict-null-sentinel 					\
	-Wstrict-overflow=2 -Wsuggest-attribute=noreturn 			\
	-Wsuggest-final-methods -Wsuggest-final-types  				\
	-Wsuggest-override -Wswitch-default -Wswitch-enum 			\
	-Wsync-nand -Wundef  -Wunreachable-code -Wunused 			\
	-Wuseless-cast -Wvariadic-macros -Wno-literal-suffix 		\
	-Wno-missing-field-initializers -Wno-narrowing				\
	-Wno-old-style-cast -Wno-varargs -fcheck-new 				\
	-fsized-deallocation  -fstack-protector 					\
	-fstrict-overflow   -flto-odr-type-merging 					\
	-fno-omit-frame-pointer -fPIE -fsanitize=address  			\
	-fsanitize=bool -fsanitize=bounds -fsanitize=enum  			\
	-fsanitize=float-cast-overflow 								\
	-fsanitize=float-divide-by-zero 							\
	-fsanitize=integer-divide-by-zero -fsanitize=leak 			\
	-fsanitize=nonnull-attribute -fsanitize=null 				\
	-fsanitize=object-size -fsanitize=return 					\
	-fsanitize=returns-nonnull-attribute -fsanitize=shift 		\
	-fsanitize=signed-integer-overflow -fsanitize=undefined 	\
	-fsanitize=unreachable -fsanitize=vla-bound 				\
	-fsanitize=vptr -lm -pie

else

	FLAGS =

endif

#==========================================================
# TARGETS
#==========================================================

all: say_hello

#----------------------------------------------------------

test: $(OBJ_DIR)/dict.o      \
	  $(OBJ_DIR)/word.o      \
	  $(OBJ_DIR)/word_rus.o  \
	  $(OBJ_DIR)/word_eng.o  \
	  $(OBJ_DIR)/word_clang.o
	$(CC) $(^) -o $(@)
	 
#----------------------------------------------------------

$(OBJ_DIR)/dict.o: dict/dict.c dict/dict.h word/word.h         \
										   word_rus/word_rus.h \
										   word_eng/word_eng.h \
										   word_clang/word_clang.h
	$(CC) $(<) -c -o $(@) $(DEBUG_FLAGS) $(CFLAGS)

$(OBJ_DIR)/word.o: word/word.c word/word.h 
	$(CC) $(<) -c -o $(@) $(DEBUG_FLAGS) $(CFLAGS)

$(OBJ_DIR)/word_rus.o: word_rus/word_rus.c word_rus/word_rus.h word/word.h
	$(CC) $(<) -c -o $(@) $(DEBUG_FLAGS) $(CFLAGS)

$(OBJ_DIR)/word_eng.o: word_eng/word_eng.c word_eng/word_eng.h word/word.h
	$(CC) $(<) -c -o $(@) $(DEBUG_FLAGS) $(CFLAGS)

$(OBJ_DIR)/word_clang.o: word_clang/word_clang.c word_clang/word_clang.h word/word.h
	$(CC) $(<) -c -o $(@) $(DEBUG_FLAGS) $(CFLAGS)

#==========================================================
# .PHONY TARGETS
#==========================================================

.PHONY: dir clean say_hello

say_hello:
	@ echo "Please, specify make target"

dir:
	@ mkdir -p obj

clean:
	- rm $(OBJ_DIR)/*.o 
	- rm test

#==========================================================
