#include <functional>
#include <iostream>
#include <sstream>

#include "ExtendibleHashFile.hpp"


struct MovieRecord {
    int dataId{};
    char contentType[16]{'\0'};
    char title[256]{'\0'};
    short length{};
    short releaseYear{};
    short endYear{};
    int votes{};
    float rating{};
    int gross{};
    char certificate[16]{'\0'};
    char description[512]{'\0'};
    bool removed{};

    std::string to_string() {
        std::stringstream ss;
        ss << "("
           << dataId << ", " << contentType << ", " << title << ", " << length << ", " << releaseYear << ", "
           << endYear << ", " << votes << ", " << rating << ", " << gross << ", " << certificate
           << ", " << std::boolalpha << removed << ")";
        return ss.str();
    }
};

int main() {
    std::function<bool(int, int)> equal = [](int a, int b) {
        return a == b;
    };

    std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
        return record.releaseYear;
    };
    std::hash<int> hasher;
    std::function<std::size_t(int)> hash = [&hasher](int key) {
        return hasher(key);
    };

    ExtendibleHashFile<int, MovieRecord, decltype(equal), decltype(index), decltype(hash), 16> extendibleHash{"movies_and_series.dat", false, index, equal, hash};
    long total = 0;
    for (short i = 1874; i <= 2023; ++i) {
        auto result = extendibleHash.search(i);
        if (!result.empty()) {
            total += result.size();
        }
    }
    total += extendibleHash.search(-1).size();
    std::cout << "Total: " << total << std::endl;
    return 0;
}
