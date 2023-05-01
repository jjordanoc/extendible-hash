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
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Executed function " << function_name << " in " << duration << " microseconds." << std::endl;
}

int main() {


    ExtendibleHashFile<int, MovieRecord, std::function<bool(int, int)>, std::function<int(MovieRecord &)>, std::function<std::size_t(int)>, 16> *extendible_hash_release_year = nullptr;
    auto create_release_year = [&]() {
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
        extendible_hash_release_year = new ExtendibleHashFile<int, MovieRecord, decltype(equal), decltype(index), decltype(hash), 16>{"movies_and_series.dat", "release_year", false, index, equal, hash};
    };
    auto search_all_release_year = [&]() {
                long total = 0;
                for (short i = 1874; i <= 2023; ++i) {
                    auto result = extendible_hash_release_year->search(i);
                    if (!result.empty()) {
                        total += result.size();
                    }
                }
                total += extendible_hash_release_year->search(-1).size();
                std::cout << "Total: " << total << std::endl;
//        extendible_hash_release_year->search(1874);
    };
    time_function(create_release_year, "create_release_year");
    time_function(search_all_release_year, "search_all_release_year");

    ExtendibleHashFile<char[16], MovieRecord, std::function<bool(char[16], char[16])>, std::function<char *(MovieRecord &)>, std::function<std::size_t(char[16])>, 16> *extendible_hash_content_type = nullptr;

    auto create_content_type = [&]() {
        std::function<bool(char[16], char[16])> equal = [](char a[16], char b[16]) -> bool {
            return std::string(a) == std::string(b);
        };

        std::function<char *(MovieRecord &)> index = [=](MovieRecord &record) {
            return record.contentType;
        };
        std::hash<std::string> hasher;
        std::function<std::size_t(char[16])> hash = [&hasher](char key[16]) {
            return hasher(std::string(key));
        };
        extendible_hash_content_type = new ExtendibleHashFile<char[16], MovieRecord, std::function<bool(char[16], char[16])>, std::function<char *(MovieRecord &)>, std::function<std::size_t(char[16])>, 16>{"movies_and_series.dat", "content_type", false, index, equal, hash};
    };
    auto search_content_type = [&]() {
        char str[16] = "movie\0";
        auto result = extendible_hash_content_type->search(str);
        std::cout << "Total: " << result.size() << std::endl;
    };
    time_function(create_content_type, "create_content_type");
    time_function(search_content_type, "search_content_type");
    delete extendible_hash_release_year;
    delete extendible_hash_content_type;

    ExtendibleHashFile<int, MovieRecord, std::function<bool(int, int)>, std::function<int(MovieRecord &)>, std::function<std::size_t(int)>, 16> *extendible_hash_data_id = nullptr;
    auto create_data_id = [&]() {
        std::function<bool(int, int)> equal = [](int a, int b) {
            return a == b;
        };

        std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
            return record.dataId;
        };
        std::hash<int> hasher;
        std::function<std::size_t(int)> hash = [&hasher](int key) {
            return hasher(key);
        };
        extendible_hash_data_id = new ExtendibleHashFile<int, MovieRecord, decltype(equal), decltype(index), decltype(hash), 16>{"movies_and_series.dat", "data_id", true, index, equal, hash};
    };
    auto search_data_id = [&]() {
        auto res = extendible_hash_data_id->search(102795);
        for (auto &record: res) {
            std::cout << record.to_string() << std::endl;
        }
    };
    time_function(create_data_id, "create_data_id");
    time_function(search_data_id, "search_data_id");
    delete extendible_hash_data_id;
    return 0;
}
