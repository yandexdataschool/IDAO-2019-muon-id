#include <iostream>
#include <iomanip>
#include <iterator>
#include <limits>

#include "reference_header.h"
#include "parser.h"

int main() {
    if (! check_header(std::cin)) {
	std::cerr << "Header mistmatch" << std::endl;
	return -1;
    }
    std::cout << std::setprecision(std::numeric_limits<float>::max_digits10);
    std::ostream_iterator<float> stdout_writer(std::cout, " ");
    std::vector<float> features(N_FEATURES);
    while (std::cin.good() && std::cin.peek() != EOF) {
	size_t id;
	ugly_hardcoded_parse(std::cin, &id, &features);
	std::copy(features.begin(), features.end(), stdout_writer);
	std::cout << '\n';
    }
    return 0;
}
