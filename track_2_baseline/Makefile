CXXFLAGS = -std=c++17 -Wall -Werror

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -O0 -p
else
	CXXFLAGS += -O2
endif

all: build

build: baseline

run: baseline
	./baseline

baseline: baseline.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp ripped_evaluator/evaluator.cpp \
	 baseline.cpp -I ripped_evaluator/include -o baseline

compute_features: compute_features.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp compute_features.cpp -o compute_features

clean:
	rm -f baseline compute_features
