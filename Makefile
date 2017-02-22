# Makefile for BDT codes

# $< = name of first dependancy
# $@ = name of target ("main")
# -c ask the compiler not to link
# -o specify target
#

#Define variables
CC=g++ #Compiler used
DEBUG = -g
CFLAGS = -c  $(DEBUG) #Compilation options
LFLAGS = $(DEBUG) -L/home/nico/root/lib -lTMVA -lTMVAGui
ROOTFLAGS = `root-config --glibs --cflags`

SRCS = $(wildcard *.cc) #Source files are all files with .cc extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cc=.o) #Object files are all files with .o extension, which have same names as source files
BDT_AN = BDT_analysis.exe #Name of executable file
INTERFACE = Interface_ntuples_TMVA.exe
SCALEFAKES = scaleFakes.exe
#.PHONY : $(wildcard *.o)  #Force to always recompile object


#Instructions
all: $(BDT_AN) $(INTERFACE) $(SCALEFAKES)

#Obtain executables from object files
$(SCALEFAKES) :	scaleFakes.o
	@echo "-- Creating executable ./$(SCALEFAKES) --"
	@$(CC) scaleFakes.o -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Done --"

$(INTERFACE) :	Interface_ntuples_TMVA.o
	@echo "-- Creating executable ./$(INTERFACE) --"
	@$(CC) Interface_ntuples_TMVA.o -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Done --"

$(BDT_AN): BDT_analysis.o theMVAtool.o
	@echo "-- Creating executable ./$(BDT_AN) --"
	@$(CC) BDT_analysis.o theMVAtool.o -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Done --"



#Obtain objects from source and header files
%.o: %.cc $(HDRS)
	@echo "-- Compiling --"
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $< -o $@



#Erase all objects and executable
clean:
	@rm -f *.o *.exe
