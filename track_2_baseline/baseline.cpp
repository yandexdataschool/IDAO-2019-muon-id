#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <vector>
#include <array>
#include <limits>

#include "ripped_evaluator/evaluator.h"

const size_t N_STATIONS = 4;
const size_t FOI_FEATURES_PER_STATION = 6;
const size_t N_RAW_FEATURES = 65;
const size_t N_RAW_FEATURES_TAIL = 2;
const size_t FOI_HITS_N_INDEX = 62;
const size_t LEXTRA_X_INDEX = 45;
const size_t LEXTRA_Y_INDEX = 49;
const size_t N_FEATURES = N_RAW_FEATURES + N_STATIONS * FOI_FEATURES_PER_STATION;
const size_t FOI_FEATURES_START = N_RAW_FEATURES;
const float EMPTY_FILLER = 1000;

const std::string REFERENCE_HEADER = "id,ncl[0],ncl[1],ncl[2],ncl[3],avg_cs[0],avg_cs[1],avg_cs[2],avg_cs[3],ndof,MatchedHit_TYPE[0],MatchedHit_TYPE[1],MatchedHit_TYPE[2],MatchedHit_TYPE[3],MatchedHit_X[0],MatchedHit_X[1],MatchedHit_X[2],MatchedHit_X[3],MatchedHit_Y[0],MatchedHit_Y[1],MatchedHit_Y[2],MatchedHit_Y[3],MatchedHit_Z[0],MatchedHit_Z[1],MatchedHit_Z[2],MatchedHit_Z[3],MatchedHit_DX[0],MatchedHit_DX[1],MatchedHit_DX[2],MatchedHit_DX[3],MatchedHit_DY[0],MatchedHit_DY[1],MatchedHit_DY[2],MatchedHit_DY[3],MatchedHit_DZ[0],MatchedHit_DZ[1],MatchedHit_DZ[2],MatchedHit_DZ[3],MatchedHit_T[0],MatchedHit_T[1],MatchedHit_T[2],MatchedHit_T[3],MatchedHit_DT[0],MatchedHit_DT[1],MatchedHit_DT[2],MatchedHit_DT[3],Lextra_X[0],Lextra_X[1],Lextra_X[2],Lextra_X[3],Lextra_Y[0],Lextra_Y[1],Lextra_Y[2],Lextra_Y[3],NShared,Mextra_DX2[0],Mextra_DX2[1],Mextra_DX2[2],Mextra_DX2[3],Mextra_DY2[0],Mextra_DY2[1],Mextra_DY2[2],Mextra_DY2[3],FOI_hits_N,FOI_hits_X,FOI_hits_Y,FOI_hits_Z,FOI_hits_DX,FOI_hits_DY,FOI_hits_DZ,FOI_hits_T,FOI_hits_DT,FOI_hits_S,PT,P";

void skip_to_next_record(std::istream& stream) {
    if (stream.eof()) {
	return;
    }
    char last_char = stream.get();
    while (! stream.eof() && last_char != ',' && last_char != '\n') {
	stream >> last_char;
    }
}

// Reads pandas-produced array of the form
// [-3421.235, 23425]
// "[1241 12]"
template<class T>
std::vector<T> read_array(std::istream& stream, const size_t array_size) {
    stream.ignore(std::numeric_limits<std::streamsize>::max(), '[');
    std::vector<T> result(array_size);
    for (size_t i = 0; i < array_size; ++i) {
	stream >> std::skipws >> result[i];
    }
    skip_to_next_record(stream);
    return result;
}

inline float square(const float x) {
    return x*x;
}

std::vector<float> nice_hardocred_parse(std::istream& stream, size_t* id) {
    stream >> (*id);
    char skip;
    stream >> skip;
    std::vector<float> result(N_FEATURES);
    for (size_t i = 0; i < N_RAW_FEATURES - N_RAW_FEATURES_TAIL; ++i) {
	stream >> result[i] >> skip;
    }
    const size_t FOI_hits_N = result[FOI_HITS_N_INDEX];
    const std::vector<float> FOI_hits_X = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_Y = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_Z = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_DX = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_DY = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_DZ = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_T = read_array<float>(stream, FOI_hits_N);
    const std::vector<float> FOI_hits_DT = read_array<float>(stream, FOI_hits_N);
    const std::vector<size_t> FOI_hits_S = read_array<size_t>(stream, FOI_hits_N);

    std::array<size_t, N_STATIONS> closest_hit_per_station;
    std::array<float, N_STATIONS> closest_hit_distance;
    closest_hit_distance.fill(std::numeric_limits<float>::infinity());    
    for (size_t hit_index = 0; hit_index < FOI_hits_N; ++hit_index) {
	const size_t this_station = FOI_hits_S[hit_index];
	const float distance_x_2 = square(FOI_hits_X[hit_index] -
					  result[LEXTRA_X_INDEX + this_station]);
	const float distance_y_2 = square(FOI_hits_Y[hit_index] -
					  result[LEXTRA_Y_INDEX + this_station]);
	const float distance_2 = distance_x_2 + distance_y_2;
	if (distance_2 < closest_hit_distance[this_station]) {
	    closest_hit_distance[this_station] = distance_2;
	    closest_hit_per_station[this_station] = hit_index;
	    result[FOI_FEATURES_START + this_station] = distance_x_2;
	    result[FOI_FEATURES_START + N_STATIONS + this_station] = distance_y_2;
	}
    }
    /* [closest_x_per_station, closest_y_per_station, closest_T_per_station,
       closest_z_per_station, closest_dx_per_station, closest_dy_per_station]) */
    for (size_t station = 0; station < N_STATIONS; ++station) {
	if (std::isinf(closest_hit_distance[station])) {
	    for (size_t feature_index = 0;
		 feature_index < FOI_FEATURES_PER_STATION;
		 ++feature_index) {
		result[FOI_FEATURES_START + feature_index * N_STATIONS + station] = EMPTY_FILLER;
	    }
	} else {
	    // x, y have already been filled by the closest hit search
	    result[FOI_FEATURES_START + 2 * N_STATIONS + station] = \
		FOI_hits_T[closest_hit_per_station[station]];
	    result[FOI_FEATURES_START + 3 * N_STATIONS + station] = \
		FOI_hits_Z[closest_hit_per_station[station]];
	    result[FOI_FEATURES_START + 4 * N_STATIONS + station] = \
		FOI_hits_DX[closest_hit_per_station[station]];
	    result[FOI_FEATURES_START + 5 * N_STATIONS + station] = \
		FOI_hits_DY[closest_hit_per_station[station]];
	}
    }
    for (size_t tail = N_RAW_FEATURES - N_RAW_FEATURES_TAIL; tail < N_RAW_FEATURES; ++tail) {
	stream >> result[tail];
	skip_to_next_record(stream);
    }
    return result;
}


int main() {
    const std::string MODEL_FILE = "track_2_model.cbm";
    NCatboostStandalone::TOwningEvaluator evaluator(MODEL_FILE);
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    std::string header;
    std::getline(std::cin, header);
    if (header != REFERENCE_HEADER) {
	std::cerr << "Header mistmatch" << std::endl;
	return -1;
    }
    std::cout << "id,prediction\n";
    std::cout << std::setprecision(8);
    while (std::cin.good() && std::cin.peek() != EOF) {
	size_t id;
	const std::vector<float> features = nice_hardocred_parse(std::cin, &id);
	const float prediction = \
	    evaluator.Apply(features, NCatboostStandalone::EPredictionType::RawValue);
	std::cout << id << "," << prediction  << "\n";
	// std::copy(features.begin(), features.end(), std::ostream_iterator<float>(std::cout, " "));
	// std::cout << '\n';
    }
    return 0;
}
