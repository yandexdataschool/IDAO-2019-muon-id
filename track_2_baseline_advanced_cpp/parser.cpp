#include "./parser.h"

inline bool not_number(const char pen) {
    return !isdigit(pen) && !(pen == '.') && !(pen == '-');
}

void skip_to_number_pointer(const char *& pen) {
    while ((*pen) && not_number(*pen)) ++pen;
}

inline float square(const float x) {
    return x*x;
}

// https://stackoverflow.com/questions/5678932/fastest-way-to-read-numerical-values-from-text-file-in-c-double-in-this-case
template<class T>
T rip_uint_pointer(const char *&pen, T val = 0) {
    // Will return val if *pen is not a digit
    // WARNING: no overflow checks
    for (char c; (c = *pen ^ '0') <= 9; ++pen)
        val = val * 10 + c;
    return val;
}

template<class T>
T rip_float_pointer(const char *&pen) {
    static double const exp_lookup[]
        = {1, 0.1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10,
           1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17};
    T sign = 1.;
    if (*pen == '-') {
        ++pen;
        sign = -1.;
    }
    uint64_t val = rip_uint_pointer<uint64_t>(pen);
    unsigned int neg_exp = 0;
    if (*pen == '.') {
        const char* const fracs = ++pen;
        val = rip_uint_pointer(pen, val);
        neg_exp  = pen - fracs;
    }
    return std::copysign(val*exp_lookup[neg_exp], sign);
}

// Warning: this is not a general-puropse parser, you have
// std::istream for that. As a rule, in the interest of speed, it
// doesn't check for input correctness and will have undefined
// behavior at incorrect input
class BufferedStream {
 public:
    explicit BufferedStream(std::istream& stream);
    // Discards data from the stream until ecountering a digit, "." or "-"
    void skip_to_number();
    // Reads a float from the stream, starting from the current character
    // and has undefined behaviour if there is no number at the
    // current position
    template<class T> T rip_float() {return rip_float_pointer<T>(pen);}
    // Reads an unsigned integer from stream, starting from the
    // current character and has undefined behaviour if there is no
    // number at the current position
    template<class T> T rip_uint() {return rip_uint_pointer<T>(pen);}
    // Reads a vector of floats of the given size from the stream,
    // skipping everything as needed
    template<class T>
    std::vector<T> fill_vector_float(const size_t size);
    // Reads a vector of unsigned ints of the given size from the stream,
    // skipping as needed. In principle, should be templated from
    // fill_vector_float, but if constexpr is C++17 :(
    template<class T>
    std::vector<T> fill_vector_uint(const size_t size);
    // Reads count floating point numbers and stores them into the
    // container pointed to by the iterator
    template<class IteratorType>
    void fill_iterator_float(const IteratorType& iterator, const size_t count);
    // Discards data from the stream until encountering the delimiter
    void skip_to_char(const char delimiter);
    // Discrads data from the stream until twice encountering the delimiter
    void skip_record(const char delimiter);

 private:
    void next_line();
    // Buffer size is measured to fit the longest line in the test dataset
    // but the code doesn't rely on it
    static const size_t BUFFER_SIZE = 1016;
    char buffer[BUFFER_SIZE];
    std::istream& stream;
    const char* pen;
};

void BufferedStream::next_line() {
    stream.getline(buffer, BUFFER_SIZE);
    pen = buffer;
}

BufferedStream::BufferedStream(std::istream& stream): stream(stream) {
    next_line();
}

void BufferedStream::skip_to_number() {
    skip_to_number_pointer(pen);
    while ((*pen) == 0) {
        next_line();
        skip_to_number_pointer(pen);
        // The skip stops either at 0-byte or
        // a number part
    }
}

template<class T>
std::vector<T> BufferedStream::fill_vector_float(const size_t size) {
    std::vector<T> result(size);
    fill_iterator_float<std::vector<float>::iterator>(result.begin(), size);
    return result;
}

template<class T>
std::vector<T> BufferedStream::fill_vector_uint(const size_t size) {
    std::vector<T> result(size);
    for (auto& value : result) {
        skip_to_number();
        value = rip_uint<T>();
    }
    return result;
}

void BufferedStream::skip_to_char(const char delimiter) {
    while ((*pen) != delimiter) {
        while ((*pen) && (*(++pen)) != delimiter) {}
        if (!(*pen)) next_line();
    }
}

void BufferedStream::skip_record(const char delimiter) {
    skip_to_char(delimiter);
    ++pen;
    skip_to_char(delimiter);
}

template<class IteratorType>
void BufferedStream::fill_iterator_float(const IteratorType& iterator, const size_t count) {
    for (IteratorType value = iterator; value != iterator + count; ++value) {
        skip_to_number();
        *value = rip_float<typename std::iterator_traits<IteratorType>::value_type>();
    }
}

void ugly_hardcoded_parse(std::istream& stream, size_t* id, std::vector<float>* result) {
    BufferedStream buffered_stream(stream);
    *id = buffered_stream.rip_uint<size_t>();
    buffered_stream.fill_iterator_float<std::vector<float>::iterator>(
        result->begin(), N_RAW_FEATURES - N_RAW_FEATURES_TAIL);
    // No need to skip, fill_vector takes care of it
    const size_t FOI_hits_N = (*result)[FOI_HITS_N_INDEX];
    const std::vector<float> FOI_hits_X = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    const std::vector<float> FOI_hits_Y = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    const std::vector<float> FOI_hits_Z = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    const std::vector<float> FOI_hits_DX = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    const std::vector<float> FOI_hits_DY = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    buffered_stream.skip_record(DELIMITER);
    const std::vector<float> FOI_hits_T = buffered_stream.fill_vector_float<float>(FOI_hits_N);
    buffered_stream.skip_record(DELIMITER);
    const std::vector<size_t> FOI_hits_S = \
        buffered_stream.fill_vector_uint<size_t>(FOI_hits_N);

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
            (*result)[FOI_FEATURES_START + 2 * N_STATIONS + station] =  \
                FOI_hits_T[closest_hit_per_station[station]];
            (*result)[FOI_FEATURES_START + 3 * N_STATIONS + station] =  \
                FOI_hits_Z[closest_hit_per_station[station]];
            (*result)[FOI_FEATURES_START + 4 * N_STATIONS + station] =  \
                FOI_hits_DX[closest_hit_per_station[station]];
            (*result)[FOI_FEATURES_START + 5 * N_STATIONS + station] =  \
                FOI_hits_DY[closest_hit_per_station[station]];
        }
    }
    buffered_stream.fill_iterator_float<std::vector<float>::iterator>(
        result->begin() + N_RAW_FEATURES - N_RAW_FEATURES_TAIL, N_RAW_FEATURES_TAIL);
}
