#ifndef EXTENDIBLE_HASH_EXTENDIBLEHASH_HPP
#define EXTENDIBLE_HASH_EXTENDIBLEHASH_HPP

#include <bitset>
#include <fstream>
#include <vector>

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

template<typename std::size_t D>
struct HashIndexEntry {
    char depth[D];  // < Stores the binary hash sequence
    long bucket_ref;// < Stores a reference to a page in disk
};

template<typename std::size_t D>
class HashIndexPage {
    std::size_t size = 0;
    std::vector<HashIndexEntry<D>> hash_entries;

public:
    HashIndexPage(char *buffer) {
    }
};


template<typename KeyType,
         typename RecordType,
         typename Greater,
         typename Index,
         std::size_t global_depth = 32>// < Maximum depth of the binary index key (defaults to 32, like in most systems)
class ExtendibleHash {
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
public:
    explicit ExtendibleHash(const std::string &fileName, bool primaryKey, Index index, Greater greater) : file_name(fileName), primary_key(primaryKey), index(index), greater(greater) {
        index_name = file_name + "_index.dat";
        // Ensure index file is created
        SAFE_FILE_OPEN(index_file, index_name, flags)
        index_file.close();
    }

    std::vector<RecordType> search(KeyType key) {
        std::vector<RecordType> result;
        SAFE_FILE_OPEN(file, file_name, flags)
        SAFE_FILE_OPEN(index_file, index_name, flags)
        std::string hash_sequence = std::bitset<global_depth>(hash_function(key) % global_depth).to_string();
        // Get the size of the index file
        SEEK_ALL(index_file, 0, std::ios::end)
        std::size_t index_file_size = TELL(index_file);
        // Read the entire index file
        SEEK_ALL(index_file, 0)
        char buffer[index_file_size];
        index_file.read(buffer, index_file_size);
        HashIndexPage<global_depth> hashIndexPage{buffer};
        file.close();
        index_file.close();
        return result;
    }

    void insert(RecordType &record) {
    }
};


#endif//EXTENDIBLE_HASH_EXTENDIBLEHASH_HPP
