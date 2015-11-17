
CXX ?=		g++
CXXFLAGS ?=	-Wall

interval_tree_test: interval_tree_test.cpp IntervalTree.h
	${CXX} ${CXXFLAGS} interval_tree_test.cpp -o interval_tree_test -std=c++0x

.PHONY: clean

clean:
	rm interval_tree_test
