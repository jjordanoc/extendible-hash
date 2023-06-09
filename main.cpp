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
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Executed function " << function_name << " in " << duration << " ms." << std::endl;
}

int main() {
    constexpr std::size_t global_depth = 16;
    std::string path_to_file = "database/movies_and_series.dat";
    {
        std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
            return record.releaseYear;
        };
        ExtendibleHashFile<int, MovieRecord, global_depth> extendible_hash_release_year{path_to_file, "release_year", false, index};
        auto create_release_year = [&]() {
            if (!extendible_hash_release_year) {
                extendible_hash_release_year.create_index();
            }
        };
        auto search_all_release_year = [&]() {
            extendible_hash_release_year.remove(2014);
            long total = 0;
            for (short i = 1874; i <= 2023; ++i) {
                auto result = extendible_hash_release_year.search(i);
                if (!result.empty()) {
                    total += result.size();
//                    if (i == 2014) {
//                        for (auto &record : result) {
//                            std::cout << record.to_string() << std::endl;
//                        }
//                    }
                }
            }
            total += extendible_hash_release_year.search(-1).size();
            std::cout << "Total: " << total << std::endl;
        };
        time_function(create_release_year, "create_release_year");
        time_function(search_all_release_year, "search_all_release_year");
    }
    {
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
        ExtendibleHashFile<char[16], MovieRecord, global_depth, std::function<char *(MovieRecord &)>, std::function<bool(char[16], char[16])>, std::function<std::size_t(char[16])>> extendible_hash_content_type{path_to_file, "content_type", false, index, equal, hash};
        auto create_content_type = [&]() {
            if (!extendible_hash_content_type) {
                extendible_hash_content_type.create_index();
            }
        };
        auto search_content_type = [&]() {
            char str[16] = "movie\0";
            auto result = extendible_hash_content_type.search(str);
            std::cout << "Total: " << result.size() << std::endl;
        };
        time_function(create_content_type, "create_content_type");
        time_function(search_content_type, "search_content_type");
    }
    {
        std::function<int(MovieRecord &)> index = [=](MovieRecord &record) {
            return record.dataId;
        };
        ExtendibleHashFile<int, MovieRecord, global_depth> extendible_hash_data_id{path_to_file, "data_id", true, index};
        auto create_data_id = [&]() {
            if (!extendible_hash_data_id) {
                extendible_hash_data_id.create_index();
            }
        };
        auto search_data_id = [&]() {
//            extendible_hash_data_id.remove(102795);
            auto res = extendible_hash_data_id.search(102795);
            for (auto &record: res) {
                std::cout << record.to_string() << std::endl;
            }
        };
        time_function(create_data_id, "create_data_id");
        time_function(search_data_id, "search_data_id");
    }


    return 0;
}
