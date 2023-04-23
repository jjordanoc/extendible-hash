#ifndef EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
#define EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP

#include <bitset>
#include <cstring>
#include <fstream>
#include <vector>

/*
 * File I/O Macro definitions
 */

#define SAFE_FILE_OPEN(file, file_name, flags)            \
    file.open(file_name, flags);                          \
    if (!file.is_open()) {                                \
        throw std::runtime_error("Could not open file."); \
    }

/*
 * Inspired by answer 2 on
 * https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros
 * Defines a macro with optional parameters for seeking both file pointers at once
 */

#define SEEK_ALL_2(file, pos) \
    file.seekg(pos);          \
    file.seekp(pos);

#define SEEK_ALL_3(file, pos, relative) \
    file.seekg(pos, relative);          \
    file.seekp(pos, relative);

#define SEEK_ALL_X(x, file, pos, relative, FUNC, ...) FUNC

#define SEEK_ALL(...) SEEK_ALL_X(, ##__VA_ARGS__,         \
                                 SEEK_ALL_3(__VA_ARGS__), \
                                 SEEK_ALL_2(__VA_ARGS__))

#define TELL(file) file.tellp()


/*
 * Definitions of constants related to Disk Space Management
 */

#define BLOCK_SIZE 1024

/*
 * Each bucket should fit in RAM.
 * Thus, the equation for determining the maximum amount of records per bucket is given by the sum of the size of its attributes:
 * BLOCK_SIZE = sizeof(long) + (MAX_RECORDS_PER_BUCKET * sizeof(RecordType)) + sizeof(long)
 */

template<typename RecordType>
const long MAX_RECORDS_PER_BUCKET = (BLOCK_SIZE - 2 * sizeof(long)) / sizeof(RecordType);

#define MAX_RECORDS_PER_BUCKET MAX_RECORDS_PER_BUCKET<RecordType>


/*
 * Class/Struct definitions
 */

template<typename RecordType>
struct Bucket {
    long size = 0;                             // < Stores the real amount of records the bucket holds
    RecordType records[MAX_RECORDS_PER_BUCKET];// < Stores the data of the records themselves
    long next = -1;                            // < Stores a reference to the next bucket in the chain (if it exists)
};

template<typename std::size_t D>
struct ExtendibleHashEntry {
    std::size_t local_depth = 0;// < Stores the local depth of the bucket
    char sequence[D + 1] = {};  // < Stores the binary hash sequence
    long bucket_ref = 0;        // < Stores a reference to a page in disk
};

template<typename std::size_t D>
class ExtendibleHash {
    std::vector<ExtendibleHashEntry<D>> hash_entries;

public:
    ExtendibleHash() {
        // Initialize an empty index with one entry (the sequence 0...0) at local depth 0 with a reference to the first bucket of the file (0)
        ExtendibleHashEntry<D> newEntry{};
        std::string empty_sequence = std::bitset<D>(0);
        std::strcpy(newEntry.sequence, empty_sequence);
        hash_entries.push_back(newEntry);
    }
    explicit ExtendibleHash(std::fstream &index_file) {
        // Get the size of the index file
        SEEK_ALL(index_file, 0, std::ios::end)
        std::size_t index_file_size = TELL(index_file);
        // Read the entire index file (should fit in RAM)
        SEEK_ALL(index_file, 0)
        char buffer[index_file_size];
        index_file.read(buffer, (long long) index_file_size);
        // Unpack the binary char buffer
        std::stringstream buf(std::string(buffer, index_file_size));
        while (!buf.eof()) {
            ExtendibleHashEntry<D> newEntry;
            buf.read((char *) &newEntry, sizeof(newEntry));
            if (!buf.eof()) {
                hash_entries.push_back(newEntry);
            }
        }
    }
    void insert(const std::string &hash_sequence) {

    }
    long lookup(const std::string &hash_sequence) {
        for (const auto &entry: hash_entries) {
            auto local_depth = entry.local_depth;
            bool eq = true;
            for (int j = 0; j < local_depth; ++j) {
                // If the sequences are different given the local depth, this is not the bucket we're looking for
                if (hash_sequence[D - 1 - j] != entry.sequence[D - 1 - j]) {
                    eq = false;
                    break;
                }
            }
            if (eq) {
                return entry.bucket_ref;
            }
        }
        throw std::runtime_error("Could not find given hash sequence on ExtendibleHash.");
    }
};


template<typename KeyType,
         typename RecordType,
         typename Greater,
         typename Index,
         std::size_t global_depth = 32>// < Maximum depth of the binary index key (defaults to 32, like in most systems)
class ExtendibleHashFile {
    std::fstream file;    //< File object used to manage disk accesses
    std::string file_name;//< File name
    std::fstream index_file;
    std::string index_name;//< Name of index file to be created
                           //    std::size_t global_depth = 32;
    const std::_Ios_Openmode flags = std::ios::in | std::ios::binary | std::ios::out;

    /* Generic purposes member variables */
    bool primary_key;                                       //< Is `true` when indexing a primary key and `false` otherwise
    Index index;                                            //< Receives a `RecordType` and returns his `KeyType` associated
    Greater greater;                                        //< Returns `true` if the first parameter is greater than the second and `false` otherwise
    std::hash<KeyType> hash_function = std::hash<KeyType>{};// < Hash function
    ExtendibleHash<global_depth> *hash_index;               // < Extendible hash index (stored in RAM)
public:
    explicit ExtendibleHashFile(const std::string &fileName, bool primaryKey, Index index, Greater greater) : file_name(fileName), primary_key(primaryKey), index(index), greater(greater) {
        index_name = file_name + "_index.dat";
        // Load or create index file
        SAFE_FILE_OPEN(index_file, index_name, flags)
        // If the index file is empty, initialize the index depending on whether the data file is empty or not
        if (index_file.peek() == std::ifstream::traits_type::eof()) {
            SAFE_FILE_OPEN(file, file_name, flags)
            // Data file is empty, just initialize an empty index
            hash_index = new ExtendibleHash<global_depth>{};
            // Data file is not empty, construct the index accordingly (insert the entries)
            if (file.peek() != std::ifstream::traits_type::eof()) {

            }
            file.close();
        }
        // The file is not empty, read its contents
        else {
            hash_index = new ExtendibleHash<global_depth>{index_file};
            //            hash_index->lookup("101");
        }
        index_file.close();
    }

    std::vector<RecordType> search(KeyType key) {
        std::vector<RecordType> result;
        //        SAFE_FILE_OPEN(file, file_name, flags)
        //        SAFE_FILE_OPEN(index_file, index_name, flags)
        //        std::string hash_sequence = std::bitset<global_depth>(hash_function(key) % global_depth).to_string();
        //        // Get the size of the index file
        //        SEEK_ALL(index_file, 0, std::ios::end)
        //        std::size_t index_file_size = TELL(index_file);
        //        // Read the entire index file
        //        SEEK_ALL(index_file, 0)
        //        char buffer[index_file_size];
        //        index_file.read(buffer, index_file_size);
        //        HashIndexPage<global_depth> hashIndexPage{buffer};
        //        file.close();
        //        index_file.close();
        SAFE_FILE_OPEN(file, file_name, flags)
        std::string hash_sequence = std::bitset<global_depth>(hash_function(key) % global_depth).to_string();
        long bucket_ref = hash_index->lookup(hash_sequence);
        SEEK_ALL(file, bucket_ref)
        Bucket<RecordType> bucket{};
        file.read((char *) &bucket, BLOCK_SIZE);
        file.close();
        return result;
    }

    void insert(RecordType &record) {
        SAFE_FILE_OPEN(file, file_name, flags)
        std::string hash_sequence = std::bitset<global_depth>(hash_function(index(record)) % global_depth).to_string();
        long bucket_ref = hash_index->lookup(hash_sequence);
        std::cout << bucket_ref << std::endl;
    }

    virtual ~ExtendibleHashFile() {
        delete hash_index;
    }
};


#endif//EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
