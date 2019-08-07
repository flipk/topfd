
OBJDIR = obj

PROG_TARGETS = topfd testsplitstring

topfd_TARGET = $(OBJDIR)/topfd
topfd_CXXSRCS = splitstring.cc walknet.cc walkproc.cc studylink.cc main.cc

testsplitstring_TARGET = $(OBJDIR)/testsplitstring
testsplitstring_CXXSRCS = splitstring.cc
testsplitstring_DEFS = -D__INCLUDE_SPLITSTRING_TEST_MAIN__=1

include ../pfkutils/Makefile.inc


bundle:
	git bundle create topfd.bundle --all
	git bundle verify topfd.bundle
