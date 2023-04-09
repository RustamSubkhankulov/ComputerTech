#==========================================================
#CONFIG
#==========================================================

CXX  = g++
CC   = gcc
CXXFLAGS = -O0 -g 
CFLAGS   = -O0 -g 

OBJ_DIR = build
SRC_DIR = src
INC_DIR = include

#----------------------------------------------------------

# flags needed to use with sfml
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# CXXFLAGS +=\
# 		   -fsanitize=address                                              \
#            -fsanitize=alignment                                            \
#            -fsanitize=bool                                                 \
#            -fsanitize=bounds                                               \
#            -fsanitize=enum                                                 \
#            -fsanitize=float-cast-overflow                                  \
#            -fsanitize=float-divide-by-zero                                 \
#            -fsanitize=integer-divide-by-zero                               \
#            -fsanitize=leak                                                 \
#            -fsanitize=nonnull-attribute                                    \
#            -fsanitize=null                                                 \
#            -fsanitize=object-size                                          \
#            -fsanitize=return                                               \
#            -fsanitize=returns-nonnull-attribute                            \
#            -fsanitize=shift                                                \
#            -fsanitize=signed-integer-overflow                              \
#            -fsanitize=undefined                                            \
#            -fsanitize=unreachable                                          \
#            -fsanitize=vla-bound                                            \
#            -fsanitize=vptr                                                 \

CXXFLAGS += \
           -g --static-pie -std=c++14 -fmax-errors=100 -Wall -Wextra       \
           -Waggressive-loop-optimizations -Wc++0x-compat                  \
           -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual          \
           -Wchar-subscripts -Wconditionally-supported -Wconversion        \
           -Wctor-dtor-privacy -Wempty-body -Wfloat-equal                  \
           -Wformat-nonliteral -Wformat-security -Wformat-signedness       \
           -Wformat=2 -Winline -Wlogical-op                                \
           -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd         \
           -Woverloaded-virtual -Wpointer-arith -Wredundant-decls          \
           -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192      \
           -Wstrict-null-sentinel -Wstrict-overflow=2                      \
           -Wsuggest-attribute=noreturn -Wsuggest-final-methods            \
           -Wsuggest-final-types -Wsuggest-override -Wswitch-default       \
           -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused   \
           -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix            \
           -Wno-missing-field-initializers -Wno-narrowing                  \
           -Wno-old-style-cast -Wno-varargs -fcheck-new                    \
           -fsized-deallocation -fstack-protector                          \
           -fstrict-overflow -flto-odr-type-merging                        \
           -fno-omit-frame-pointer                                         \
           -fPIE                                                           \
           -lm -pie

#==========================================================
# SOURCE / OBJECT FILES LIST
#==========================================================

SRC_PATH = $(SRC_DIR)/$(1)/$(1).cpp

MODULES = model view view_text view_graph controller main vector

OBJ = $(foreach module, $(MODULES), $(OBJ_DIR)/$(module).o)
DEP = $(foreach module, $(MODULES), $(OBJ_DIR)/$(module).d)
SRC = $(foreach module, $(MODULES), $(call SRC_PATH,$(module)))

#==========================================================
# TARGETS
#==========================================================

all: dir mkdep build
build: snake 

#==========================================================
# DEPENDENCIES
#==========================================================

mkdep: $(DEP)

define depend_rule =
$(OBJ_DIR)/$(1).d: $(call SRC_PATH,$(1))
	$(CXX) $(call SRC_PATH,$(1)) -MM -MT"$(OBJ_DIR)/$(1).o" -o $(OBJ_DIR)/$(1).d
    -include $(OBJ_DIR)/$(1).d
endef

$(foreach module, $(MODULES), $(eval $(call depend_rule,$(module))))

#----------------------------------------------------------

snake: $(OBJ)
	$(CXX) $(^) -o $(@) $(SFML_FLAGS) $(CXXFLAGS)

define build_rule =
$(OBJ_DIR)/$(1).o: $(call SRC_PATH,$(1)) 
	$(CXX) $(call SRC_PATH,$(1)) -c -o $(OBJ_DIR)/$(1).o $(CXXFLAGS)
endef

-include depends

$(foreach module, $(MODULES), $(eval $(call build_rule,$(module))))

#==========================================================
# .PHONY TARGETS
#==========================================================

.PHONY: dir mkdep clean

dir:
	@ mkdir -p $(OBJ_DIR)

clean:
	-@ rm $(OBJ_DIR)/*.o 
	-@ rm $(OBJ_DIR)/*.d 
	-@ rm snake



#==========================================================

