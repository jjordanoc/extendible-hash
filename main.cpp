#include <chrono>
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

template<typename Function, typename... Params>
void time_function(Function &fun, const std::string &function_name, const Params &...params) {
    std::cout << "Executing function " << function_name << "..." << std::endl;
    const auto start = std::chrono::steady_clock::now();
    fun(params...);
    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Executed function " << function_name << " in " << duration << " seconds." << std::endl;
}

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
    ExtendibleHashFile<int, MovieRecord, decltype(equal), decltype(index), decltype(hash), 16> *extendible_hash = nullptr;

    auto create = [&]() {
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
        extendible_hash = new ExtendibleHashFile<int, MovieRecord, decltype(equal), decltype(index), decltype(hash), 16>{"movies_and_series.dat", false, index, equal, hash};
    };

    auto search_all = [&]() {
        long total = 0;
        for (short i = 1874; i <= 2023; ++i) {
            auto result = extendible_hash->search(i);
            if (!result.empty()) {
                total += result.size();
            }
        }
        total += extendible_hash->search(-1).size();
        std::cout << "Total: " << total << std::endl;
    };
    time_function(create, "create");
    extendible_hash->remove(1874);
    time_function(search_all, "search_all");
    delete extendible_hash;
    return 0;
}
