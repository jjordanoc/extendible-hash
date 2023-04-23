#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>

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

std::ostream &operator<<(std::ostream &stream, Record &p) {
    stream.write((char *) &p, sizeof(Record));
    stream << std::flush;
    return stream;
}

std::ifstream &operator>>(std::ifstream &stream, Record &p) {
    stream.read((char *) &p, sizeof(p));
    return stream;
}


void readFromConsole(char buffer[], int size) {
    std::string temp;
    std::cin >> temp;
    for (int i = 0; i < size; i++)
        buffer[i] = (i < temp.size()) ? temp[i] : ' ';
    buffer[size - 1] = '\0';
    std::cin.clear();
}

template<typename std::size_t D>
struct ExtendibleHashEntry {
    std::size_t local_depth = 1;// < Stores the local depth of the bucket
    char sequence[D + 1];       // < Stores the binary hash sequence
    long bucket_ref;            // < Stores a reference to a page in disk

    std::string to_string() {
        std::stringstream ss;
        ss << "(" << local_depth << ", " << sequence << ", " << bucket_ref << ")";
        return ss.str();
    }
};

void create_file() {
    std::ofstream file("data.dat", std::ios::binary);
    int n;
    std::cin >> n;
    for (int i = 0; i < n; ++i) {
        Record newRecord{};
        readFromConsole(newRecord.code, 5);
        readFromConsole(newRecord.name, 20);
        std::cin >> newRecord.cycle;
        file << newRecord;
    }
    file.close();
    std::ifstream infile("data.dat", std::ios::binary);
    while (!infile.eof()) {
        Record tmp{};
        infile >> tmp;
        if (!infile.eof()) {
            std::cout << tmp.to_string() << std::endl;
        }
    }
    infile.close();
}

void create_index_file() {
    std::ofstream file("data.dat_index.dat", std::ios::binary);
    int n;
    std::cin >> n;
    for (int i = 0; i < n; ++i) {
        ExtendibleHashEntry<3> newEntry{};
        readFromConsole(newEntry.sequence, 4);
        std::cin >> newEntry.bucket_ref;
        file.write((char *) &newEntry, sizeof(ExtendibleHashEntry<3>));
    }
    file.close();
    std::ifstream infile("data.dat_index.dat", std::ios::binary);
    while (!infile.eof()) {
        ExtendibleHashEntry<3> tmp{};
        infile.read((char *) &tmp, sizeof(ExtendibleHashEntry<3>));
        if (!infile.eof()) {
            std::cout << tmp.to_string() << std::endl;
        }
    }
    infile.close();
}

int main() {
//    create_index_file();
//    std::cout << ?
}