
TARGETS = test_1
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -I../ -g

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): %: %.cpp ../args.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm $(TARGETS) || true

