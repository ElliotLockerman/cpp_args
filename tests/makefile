
TARGETS = test_1
CXXFLAGS = -std=c++11 -Wall -Wextra -I../

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): %: %.cpp ../args.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm $(TARGETS) || true

