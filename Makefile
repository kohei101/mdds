
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST -Wall -Os -std=c++0x
LDFLAGS=

EXECS= \
	flatsegmenttree-test \
	segmenttree-test \
	stlperf-test

HEADERS= \
	$(INCDIR)/node.hpp \
	$(INCDIR)/global.hpp \
	$(INCDIR)/flatsegmenttree.hpp \
	$(INCDIR)/segmenttree.hpp \
	$(INCDIR)/rectangle_set.hpp

OBJFILES= \
	$(OBJDIR)/flatsegmenttree_test.o \
	$(OBJDIR)/segmenttree_test.o \
	$(OBJDIR)/rectangle_set_test.o

DEPENDS= \
	$(HEADERS)

all: $(EXECS)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/flatsegmenttree_test.o: $(SRCDIR)/flatsegmenttree_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/flatsegmenttree_test.cpp

$(OBJDIR)/segmenttree_test.o: $(SRCDIR)/segmenttree_test.cpp  $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/segmenttree_test.cpp

$(OBJDIR)/rectangle_set_test.o: $(SRCDIR)/rectangle_set_test.cpp  $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/rectangle_set_test.cpp

flatsegmenttree-test: pre $(OBJDIR)/flatsegmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/flatsegmenttree_test.o -o $@

segmenttree-test: pre $(OBJDIR)/segmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/segmenttree_test.o -o $@

rectangle-set-test: pre $(OBJDIR)/rectangle_set_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/rectangle_set_test.o -o $@

stlperf-test: pre $(SRCDIR)/stlperf_test.cpp
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(SRCDIR)/stlperf_test.cpp -o $@

$(OBJDIR)/template_test.o: $(SRCDIR)/template_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/template_test.cpp

test.fst: flatsegmenttree-test
	./flatsegmenttree-test

test.recset: rectangle-set-test
	./rectangle-set-test

test.recset.mem: rectangle-set-test
	valgrind --tool=memcheck --leak-check=full ./rectangle-set-test

test.fst.mem: flatsegmenttree-test
	valgrind --tool=memcheck --leak-check=full ./flatsegmenttree-test

test.st: segmenttree-test
	./segmenttree-test func

test.st.mem: segmenttree-test
	valgrind --tool=memcheck --leak-check=full ./segmenttree-test func

test.stl: stlperf-test
	./stlperf-test

clean:
	rm -rf $(OBJDIR) 2>/dev/null || /bin/true
	rm $(EXECS) 2>/dev/null || /bin/true

