
export CXX := g++
export CXXFLAGS += -O2 -DANDROID -fPIC --pie -std=c++11 -fexceptions -frtti -L../.. -lupk

export MAKE := make --no-print-directory

export SRC := $(shell cd src && ls *.cc)
export OBJ := $(SRC:.cc=.o)
export BIN := upk



all:
	@cd src && $(MAKE)


.PHONY: clean
clean: 
	@-cd src && $(MAKE) clean
	@echo -e "  CLEAN\t" $(BIN)
	@$(RM) $(BIN)
