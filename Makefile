CXX = c++
CXXFLAGS = -std=c++17 -O2 -DNDEBUG -Wall -Wextra -Wpedantic
BENCHMARK_FLAGS = $(shell pkg-config --cflags --libs benchmark)

.PHONY: all test clean
all: demo tests benchmarks work_counts

demo: main.cpp skip_list.cpp skip_list.hpp
	$(CXX) $(CXXFLAGS) main.cpp skip_list.cpp -o demo

tests: tests.cpp skip_list.cpp skip_list.hpp
	$(CXX) $(CXXFLAGS) tests.cpp skip_list.cpp -o tests

benchmarks: benchmarks.cpp skip_list.cpp skip_list.hpp experiment.hpp Makefile
	$(CXX) $(CXXFLAGS) benchmarks.cpp skip_list.cpp $(BENCHMARK_FLAGS) -pthread -o benchmarks

work_counts: work_counts.cpp skip_list.cpp skip_list.hpp experiment.hpp
	$(CXX) $(CXXFLAGS) -DCOUNT_WORK work_counts.cpp skip_list.cpp -o work_counts

tests_sanitized: tests.cpp skip_list.cpp skip_list.hpp
	$(CXX) -std=c++17 -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined tests.cpp skip_list.cpp -o tests_sanitized

test: tests
	./tests

clean:
	rm -f demo tests benchmarks work_counts tests_sanitized
