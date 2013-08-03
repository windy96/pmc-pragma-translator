## A sample Makefile to build a ROSE tool.
##
## Important: remember that Makefile recipes must contain tabs:
##
##     <target>: [ <dependency > ]*
##         [ <TAB> <command> <endl> ]+

## ROSE installation contains
##   * libraries, e.g. "librose.la"
##   * headers, e.g. "rose.h"
ROSE_INSTALL=/home/kim77/local/development/rose/edg4/install4

## ROSE uses the BOOST C++ libraries
BOOST_INSTALL=/home/kim77/local/development/1_45_0

## Your translator
TRANSLATOR=myomp
#TRANSLATOR=nextline
TRANSLATOR_SOURCE=$(TRANSLATOR).C

## Input testcode for your translator
TESTCODE=matrixTranspose.c

#-------------------------------------------------------------
# Makefile Targets
#-------------------------------------------------------------

all: $(TRANSLATOR)

# compile the translator and generate an executable
# -g is recommended to be used by default to enable debugging your code
$(TRANSLATOR): $(TRANSLATOR_SOURCE)
	g++ -g $(TRANSLATOR_SOURCE) -I$(BOOST_INSTALL)/include -I$(ROSE_INSTALL)/include -L$(ROSE_INSTALL)/lib -lrose -o $(TRANSLATOR)

# test the translator
check: $(TRANSLATOR)
	./$(TRANSLATOR) -c -I. -I$(ROSE_INSTALL)/include $(TESTCODE)

clean:
	rm -rf $(TRANSLATOR) *.o rose_* *.dot
