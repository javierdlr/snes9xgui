#
# Makefile generated by:
# CodeBench 0.42
#
# Project: SNES9XGUI
#
# Created on: 23-06-2016 14:29:43
#
#

CC = SDK:gcc/bin/gcc
LD = SDK:gcc/bin/gcc
OBJ = \
	 snes9xgui.o gui.o cfg.o \
	


BIN = snes9xGUI_contrib


OS := $(shell uname)

ifeq ($(strip $(OS)),AmigaOS)
	AMIGADATE = $(shell c:date LFORMAT %d.%m.%Y)
	#YEAR = $(shell c:date LFORMAT %Y)
else
	AMIGADATE = $(shell date +"%-d.%m.%Y")
	#YEAR = $(shell date +"%Y")
endif

DEBUG = -DDEBUG


INCPATH = -I. -Iincludes

CFLAGS = $(DEBUG) $(INCPATH) -Wall -D__AMIGADATE__=\"$(AMIGADATE)\" -gstabs

LDFLAGS = 

LIBS = 
#	add any extra linker libraries you want here

.PHONY: all all-before all-after clean clean-custom realclean

all: all-before $(BIN) all-after

all-before:
#	You can add rules here to execute before the project is built

all-after:
#	You can add rules here to execute after the project is built

clean: clean-custom
	rm -v $(OBJ)

realclean:
	rm -v $(OBJ) $(BIN)

$(BIN): $(OBJ) $(LIBS)
#	You may need to move the LDFLAGS variable in this rule depending on its contents
	@echo "Linking $(BIN)"
	@$(LD) -o $(BIN).debug $(OBJ) $(LDFLAGS) $(LIBS)
	strip $(BIN).debug -o $(BIN)

###################################################################
##
##  Standard rules
##
###################################################################

# A default rule to make all the objects listed below
# because we are hiding compiler commands from the output

.c.o:
	@echo "Compiling $<"
	@$(CC) -c $< -o $*.o $(CFLAGS)

snes9xgui.o: snes9xgui.c includes/includes.h snes9xgui_rev.h snes9xgui_strings.h

gui.o: gui.c includes/includes.h snes9xgui_rev.h snes9xgui_strings.h \
includes/gui_about.h includes/gui_general.h \
includes/gui_settingsLB_graphics.h includes/gui_settingsLB_audio.h includes/gui_settingsLB_engine.h \
includes/gui_settingsLB_misc.h includes/gui_settingsLB_rewinding.h \
includes/gui_amigainput.h 

cfg.o: cfg.c includes/includes.h snes9xgui_rev.h snes9xgui_strings.h

###################################################################
##
##  Custom rules
##
###################################################################

snes9xgui_strings.h: snes9xgui.cd
	APPDIR:CatComp snes9xgui.cd CFILE snes9xgui_strings.h


###################################################################
