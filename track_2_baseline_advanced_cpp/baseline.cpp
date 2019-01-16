// Copyright 2019, Nikita Kazeev, Higher School of Economics
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <vector>
#include <limits>

#include "./parser.h"
#include "ripped_evaluator/evaluator.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    const std::string MODEL_FILE = "track_2_model.cbm";
    NCatboostStandalone::TOwningEvaluator evaluator(MODEL_FILE);
    // Skip header
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << std::setprecision(std::numeric_limits<float>::max_digits10);
    std::cout << "id,prediction\n";
    while (std::cin.good() && std::cin.peek() != EOF) {
	std::vector<float> features(N_FEATURES);
        size_t id;
        ugly_hardcoded_parse(std::cin, &id, &features);
        const float prediction = \
            evaluator.Apply(features, NCatboostStandalone::EPredictionType::RawValue);
        std::cout << id << DELIMITER << prediction  << '\n';
    }
    return 0;
}
