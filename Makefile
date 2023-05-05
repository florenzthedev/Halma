IDIR =./include
CC = gcc
#Flags for C compiler
CFLAGS = -g -Wall -pedantic -Werror -fshort-enums

ODIR = ./obj
LDIR = ./lib
BDIR = ./bin

#Name of final executable
EXEC = halma

#Libs to link to, use -l[libray name] seperate entries with spaces
LIBS = 

#Headers in project, space seperated
_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#Name of source files needed, but with .o at the end, space seperated
_OBJ = halma.o bitmask.o halma_term.o main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#Rule for making .o files from .c files
$(ODIR)/%.o: %.c $(DEPS) $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

#Final program target
$(EXEC): $(OBJ) | $(BDIR)
	$(CC) -o $(BDIR)/$@ $^ $(LIBS)

#Make sure obj directory exitsts, '$@' represents everything before the :
#in the target
$(ODIR):
	mkdir -p $@

#Make sure bin directory exitsts, '$@' represents everything before the :
#in the target
$(BDIR):
	mkdir -p $@

#Prevents 'make clean' from messing with a file named clean if it exists
.PHONY: clean

#Removes object and temp files
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

#runs the program in question, and depends on it being up to date
run: $(EXEC)
	gnome-terminal -- ./$(EXEC)
