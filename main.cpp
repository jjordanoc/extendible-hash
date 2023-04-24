#include <iostream>
#include <functional>
#include <sstream>

#include "ExtendibleHashFile.hpp"

void readFromConsole(char buffer[], int size) {
    std::string temp;
    std::cin >> temp;
    for (int i = 0; i < size; i++)
        buffer[i] = (i < temp.size()) ? temp[i] : ' ';
    buffer[size - 1] = '\0';
    std::cin.clear();
}

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

    std::function<int (Record &)> index = [=](Record &record) {
        return record.cycle;
    };

    ExtendibleHashFile<int, Record, decltype(greater), decltype(index), 3> extendibleHash{"data.dat", true, index, greater};
    Record new_record{};
    readFromConsole(new_record.code, 5);
    readFromConsole(new_record.name, 20);
    std::cin >> new_record.cycle;
    extendibleHash.insert(new_record);
    return 0;
}
