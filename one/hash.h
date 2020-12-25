#ifndef __HASH_H__
#define __HASH_H__

#include <cstring>
#include <cstdlib>
#include <cstdint>


using namespace std;

static inline void FallThroughIntended() { }

inline uint32_t DecodeFixed32(const char* ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

// 散列
uint32_t Hash(const char* data, size_t n, uint32_t seed) {
    const uint32_t m = 0xc6a4a793;
    const uint32_t r = 24;
    const char* limit = data + n;
    uint32_t h = seed ^ (n * m);

    // Pick up four bytes at a time
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    switch (limit - data) {
        case 3:
        h += static_cast<unsigned char>(data[2]) << 16;
        FallThroughIntended();
        
        case 2:
        h += static_cast<unsigned char>(data[1]) << 8;
        FallThroughIntended();

        case 1:
        h += static_cast<unsigned char>(data[0]);
        h *= m;
        h ^= (h >> r);
        break;
    }
    return h;
}

// 分割
std::vector<std::string> SplitString(const std::string& src, const std::string& delim) {
    std::vector<std::string> dst;
    std::string::size_type pos1 = 0, pos2 = src.find(delim);
    while (std::string::npos != pos2) {
        dst.push_back(src.substr(pos1, pos2-pos1));

        pos1 = pos2 + delim.size();
        pos2 = src.find(delim, pos1);
    }
    if (pos1 != src.length()) {
        dst.push_back(src.substr(pos1));
    }
    return dst;
}


// 分割字符串
// void SplitString(const string& s, vector<string>& v, const string& c) {
//     string::size_type pos1, pos2;
//     pos2 = s.find(c);
//     pos1 = 0;
//     while (string::npos != pos2) {
//         v.push_back(s.substr(pos1, pos2-pos1));
//         pos1 = pos2 + c.size();
//         pos2 = s.find(c, pos1);
//     }

//     if (pos1 != s.length()) {
//         v.push_back(s.substr(pos1));
//     }
// }

#endif
