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
# LFLAGS = $(DEBUG) -L/home/nico/root/lib -lTMVA -lTMVAGui
LFLAGS = $(DEBUG) -lTMVA #-lTMVAGui
ROOTFLAGS = `root-config --glibs --cflags`


#Instructions
all:
	@g++ Create_Script_Datacard_Generation.cc -o Create_Script_Datacard_Generation.exe `root-config --cflags --glibs` -lTMVA
	@echo "-- Created executable [Create_Script_Datacard_Generation.exe] --"


#Erase all objects and executable
clean:
	@rm Create_Script_Datacard_Generation.exe
