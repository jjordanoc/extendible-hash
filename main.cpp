#include <iostream>
#include <functional>
#include <sstream>

#include "ExtendibleHash.hpp"

struct Record {
    char code[5];
    char name[20];
    int cycle;

    std::string to_string() {
        std::stringstream ss;
        ss << "(" << code << ", " << name << ", " << cycle << ")";
        return ss.str();
    }
};


int main() {
    std::function<bool(char *, char *)> greater = [](char * a, char * b) {
        return std::string(a) > std::string(b);
    };

    std::function<char *(Record &)> index = [=](Record &record) {
        return record.name;
    };

    ExtendibleHash<char *, Record, decltype(greater), decltype(index), 3> extendibleHash{"data.dat", true, index, greater};
    extendibleHash.search("Hello");
    return 0;
}
