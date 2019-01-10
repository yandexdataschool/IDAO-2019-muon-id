#include "parser.h"

// Global variables FTW!
// Buffer size is properly measured in the file
// But the code doesn't rely on lines fitting into the buffer
static const size_t BUFFER_SIZE = 1016;
char buffer[BUFFER_SIZE];

inline bool not_number(const char pen) {
    return !isdigit(pen) && !(pen == '.') && !(pen == '-');
}

void skip_to_number(const char *& pen) {
    while ((*pen) && not_number(*pen)) ++pen;
}

void skip_to_number(std::istream& stream,
		    const char *& pen) {
    skip_to_number(pen);
    while ((*pen) == 0) {
	stream.getline(buffer, BUFFER_SIZE);
	pen = buffer;
	skip_to_number(pen);
	// The skip stops either at 0-byte or
	// a number part
    }
}

inline float square(const float x) {
    return x*x;
}

// https://stackoverflow.com/questions/5678932/fastest-way-to-read-numerical-values-from-text-file-in-c-double-in-this-case
template<class T>
T rip_uint(const char *&pen, T val = 0) {
    // Will return val if *pen is not a digit
    // BEWARE: no overflow checks
    for (char c; (c = *pen ^ '0') <= 9; ++pen) 
	val = val * 10 + c;
    return val;
}

template<class value_t>
value_t rip_float(const char *&pen) {
    static double const exp_lookup[]
	= {1, 0.1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10,
	   1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17};
    float sign = 1.;
    if (*pen == '-' ) {
	++pen;
	sign = -1.;
    }
    uint64_t val = rip_uint<uint64_t>(pen);
    unsigned int neg_exp = 0;
    if (*pen == '.') {
        const char* const fracs = ++pen;
        val = rip_uint(pen, val);
        neg_exp  = pen - fracs;
    }
    return std::copysign(val*exp_lookup[neg_exp], sign);
}

template<class T>
std::vector<T> fill_vector(std::istream& stream,
			   const char *& pen,
			   const size_t size) {
    std::vector<T> result(size);
    for (auto& value: result) {
	skip_to_number(stream, pen);
	value = rip_float<T>(pen);
    }
    return result;
}

// if constexpr is C++17 :(
template<class T>
std::vector<T> fill_vector_uint(std::istream& stream,
				const char *& pen,
				const size_t size) {
    std::vector<T> result(size);
    for (auto& value: result) {
	skip_to_number(stream, pen);
	value = rip_uint<T>(pen);
    }
    return result;
}


void skip_to_char(std::istream& stream,
		  const char *& pen,
		  const char delimiter) {
    while ((*pen) != delimiter) {
	while ((*pen) && (*(++pen)) != delimiter);
	if (!(*pen)) {
	    stream.getline(buffer, BUFFER_SIZE);
	    pen = buffer;
	}
    }
}

void skip_record(std::istream& stream,
		 const char *& pen,
		 const char delimiter) {
    skip_to_char(stream, pen, delimiter);
    ++pen;
    skip_to_char(stream, pen, delimiter);
}

void ugly_hardcoded_parse(std::istream& stream, size_t* id, std::vector<float>* result) {
    stream.getline(buffer, BUFFER_SIZE);
    const char* pen = buffer;
    *id = rip_uint<size_t>(pen);
    skip_to_number(pen);
    for (auto value = result->begin();
	 value != result->begin() + N_RAW_FEATURES - N_RAW_FEATURES_TAIL;
	 ++value) {
	*value = rip_float<float>(pen);
	skip_to_number(pen);
    }
    const size_t FOI_hits_N = (*result)[FOI_HITS_N_INDEX];
    const std::vector<float> FOI_hits_X = fill_vector<float>(stream, pen, FOI_hits_N);
    const std::vector<float> FOI_hits_Y = fill_vector<float>(stream, pen, FOI_hits_N);
    const std::vector<float> FOI_hits_Z = fill_vector<float>(stream, pen, FOI_hits_N);
    const std::vector<float> FOI_hits_DX = fill_vector<float>(stream, pen, FOI_hits_N);
    const std::vector<float> FOI_hits_DY = fill_vector<float>(stream, pen, FOI_hits_N);
    skip_record(stream, pen, DELIMITER);
    const std::vector<float> FOI_hits_T = fill_vector<float>(stream, pen, FOI_hits_N);
    skip_record(stream, pen, DELIMITER);
    const std::vector<size_t> FOI_hits_S = \
	fill_vector_uint<size_t>(stream, pen, FOI_hits_N);

    std::array<size_t, N_STATIONS> closest_hit_per_station;
    std::array<float, N_STATIONS> closest_hit_distance;
    closest_hit_distance.fill(std::numeric_limits<float>::infinity());    
    for (size_t hit_index = 0; hit_index < FOI_hits_N; ++hit_index) {
	const size_t this_station = FOI_hits_S[hit_index];
	const float distance_x_2 = square(FOI_hits_X[hit_index] -
					  (*result)[LEXTRA_X_INDEX + this_station]);
	const float distance_y_2 = square(FOI_hits_Y[hit_index] -
					  (*result)[LEXTRA_Y_INDEX + this_station]);
	const float distance_2 = distance_x_2 + distance_y_2;
	if (distance_2 < closest_hit_distance[this_station]) {
	    closest_hit_distance[this_station] = distance_2;
	    closest_hit_per_station[this_station] = hit_index;
	    (*result)[FOI_FEATURES_START + this_station] = distance_x_2;
	    (*result)[FOI_FEATURES_START + N_STATIONS + this_station] = distance_y_2;
	}
    }
    /* [closest_x_per_station, closest_y_per_station, closest_T_per_station,
       closest_z_per_station, closest_dx_per_station, closest_dy_per_station]) */
    for (size_t station = 0; station < N_STATIONS; ++station) {
	if (std::isinf(closest_hit_distance[station])) {
	    for (size_t feature_index = 0;
		 feature_index < FOI_FEATURES_PER_STATION;
		 ++feature_index) {
		(*result)[FOI_FEATURES_START + feature_index * N_STATIONS + station] = EMPTY_FILLER;
	    }
	} else {
	    // x, y have already been filled by the closest hit search
	    (*result)[FOI_FEATURES_START + 2 * N_STATIONS + station] =	\
		FOI_hits_T[closest_hit_per_station[station]];
	    (*result)[FOI_FEATURES_START + 3 * N_STATIONS + station] =	\
		FOI_hits_Z[closest_hit_per_station[station]];
	    (*result)[FOI_FEATURES_START + 4 * N_STATIONS + station] =	\
		FOI_hits_DX[closest_hit_per_station[station]];
	    (*result)[FOI_FEATURES_START + 5 * N_STATIONS + station] =	\
		FOI_hits_DY[closest_hit_per_station[station]];
	}
    }
    for (size_t tail = N_RAW_FEATURES - N_RAW_FEATURES_TAIL; tail < N_RAW_FEATURES; ++tail) {
	skip_to_number(pen);
	(*result)[tail] = rip_float<float>(pen);
    }
}
