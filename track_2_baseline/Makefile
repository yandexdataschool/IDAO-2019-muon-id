CXXFLAGS = -std=c++17

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -O2 -p
else
	CXXFLAGS += -O2
endif

all: baseline compute_features

baseline: baseline.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp ripped_evaluator/evaluator.cpp \
	 baseline.cpp -I ripped_evaluator/include -o baseline

compute_features: compute_features.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp compute_features.cpp -o compute_features

clean:
	rm -f baseline compute_features
