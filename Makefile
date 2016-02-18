
export CXX := g++
export CXXFLAGS += -O2 -DANDROID -fPIC --pie -std=c++11 -fexceptions -frtti -g

export MAKE := make --no-print-directory

export SRC := $(shell cd src && ls *.cc)
export OBJ := $(SRC:.cc=.o)
export LIB := libupk.a



all:
	@cd src && $(MAKE)
	@cd upk-cli && $(MAKE)


.PHONY: clean
clean: 
	@-cd src && $(MAKE) clean
	@-cd upk-cli && $(MAKE) clean
	@echo -e "  CLEAN\t" $(LIB)
	@$(RM) $(LIB)
