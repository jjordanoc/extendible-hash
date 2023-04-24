#ifndef EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
#define EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP

#include <bitset>
#include <cmath>
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

#define SAFE_FILE_CREATE_IF_NOT_EXISTS(file, file_name)   \
    file.open(file_name, std::ios::app);                  \
    if (!file.is_open()) {                                \
        throw std::runtime_error("Could not open file."); \
    }                                                     \
    file.close();
/*
 * Inspired by answer 2 on
 * https://stackoverflow.com/questions/3046889/optional-parameters-with-c-macros
 * Defines a macro with optional parameters for seeking both file pointers at once
 */

#define SEEK_ALL(file, pos) \
    file.seekg(pos);        \
    file.seekp(pos);

#define SEEK_ALL_RELATIVE(file, pos, relative) \
    file.seekg(pos, relative);                 \
    file.seekp(pos, relative);

#define TELL(file) file.tellp()

/*
 * Debugging tools
 */

#define PRINT_FLAGS(file) \
    std::cout << std::boolalpha << "Good: " << file.good() << " Eof: " << file.eof() << " Bad: " << file.bad() << " Fail: " << file.fail() << std::endl;

#define PRINT_SIZE(T) \
    std::cout << "Size: " << sizeof(T) << std::endl;

#define PRINT_TELL(file) \
    std::cout << "tellg: " << file.tellg() << " tellp: " << file.tellp() << std::endl;

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
constexpr long MAX_RECORDS_PER_BUCKET = (BLOCK_SIZE - 2 * sizeof(long)) / sizeof(RecordType);

#define MAX_RECORDS_PER_BUCKET MAX_RECORDS_PER_BUCKET<RecordType>


/*
 * Class/Struct definitions
 */

template<typename RecordType>
struct Bucket {
    long size = 0;                             // < Stores the real amount of records the bucket holds
    RecordType records[MAX_RECORDS_PER_BUCKET];// < Stores the data of the records themselves
    long next = -1;                            // < Stores a reference to the next bucket in the chain (if it exists)
    Bucket() = default;
    //    explicit Bucket(std::fstream &hash_file) {
    //        char *block_buffer = new char[BLOCK_SIZE];
    //        hash_file.read(block_buffer, BLOCK_SIZE);
    //        std::stringstream buf{std::string{block_buffer, BLOCK_SIZE}};
    //        buf.read((char *) &size, sizeof(long));
    //        for (int i = 0; i < size; ++i) {
    //            buf.read((char *) &(records[i]), sizeof(RecordType));
    //        }
    //        SEEK_ALL(hash_file, sizeof(long), std::ios::end)
    //        buf.read((char *) &next, sizeof(long));
    //        delete[] block_buffer;
    //    }
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
        std::string empty_sequence = std::bitset<D>(0).to_string();
        std::strcpy(newEntry.sequence, empty_sequence.c_str());
        hash_entries.push_back(newEntry);
    }
    explicit ExtendibleHash(std::fstream &index_file) {
        // Get the size of the index file
        SEEK_ALL_RELATIVE(index_file, 0, std::ios::end)
        std::size_t index_file_size = TELL(index_file);
        // Read the entire index file (should fit in RAM)
        SEEK_ALL(index_file, 0)
        char *buffer = new char[index_file_size];
        index_file.read(buffer, (long long) index_file_size);
        // Unpack the binary char buffer
        std::stringstream buf{std::string{buffer, index_file_size}};
        while (!buf.eof()) {
            ExtendibleHashEntry<D> newEntry;
            buf.read((char *) &newEntry, sizeof(newEntry));
            if (!buf.eof()) {
                hash_entries.push_back(newEntry);
            }
        }
        delete[] buffer;
    }
    void write_to_disk(std::fstream &index_file) {
        const std::size_t index_size = hash_entries.size() * sizeof(ExtendibleHashEntry<D>);
        char *buffer = new char[index_size];
        // Pack the binary char buffer
        std::stringstream buf{std::string{buffer, index_size}};

        for (std::size_t i = 0; i < hash_entries.size(); ++i) {
            std::cout << "depth: " << hash_entries[i].local_depth << "seq: " << hash_entries[i].sequence << "ref: " << hash_entries[i].bucket_ref << std::endl;
            buf.write((char *) &(hash_entries[i]), sizeof(hash_entries[i]));
        }
        std::cout << buf.str() << std::endl;
        // Write buffer to disk
        index_file.write(buf.str().c_str(), (long long) index_size);
        delete[] buffer;
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
    std::fstream raw_file;                                                                //< File object used to manage acces to the raw data file (not used if index is already created)
    std::string raw_file_name;                                                            //< Raw data file name
    std::fstream index_file;                                                              // < File object used to manage the index
    std::string index_file_name;                                                          //< Name of index raw_file to be created
    std::fstream hash_file;                                                               // < File object used to access hash-based indexed file
    std::string hash_file_name;                                                           // < Hash-based indexed file name
    const std::ios_base::openmode flags = std::ios::in | std::ios::binary | std::ios::out;// < Flags used in all accesses to disk

    /* Generic purposes member variables */
    bool primary_key;                                       //< Is `true` when indexing a primary key and `false` otherwise
    Index index;                                            //< Receives a `RecordType` and returns his `KeyType` associated
    Greater greater;                                        //< Returns `true` if the first parameter is greater than the second and `false` otherwise
    std::hash<KeyType> hash_function = std::hash<KeyType>{};// < Hash function
    ExtendibleHash<global_depth> *hash_index;               // < Extendible hash index (stored in RAM)

    std::string get_hash_sequence(RecordType &record) {
        auto key = index(record);
        auto hash_key = hash_function(key);
        auto bit_set = std::bitset<global_depth>{hash_key % (1 << global_depth)};
        std::cout << bit_set.to_string() << std::endl;
        return bit_set.to_string();
    }

public:
    explicit ExtendibleHashFile(const std::string &fileName, bool primaryKey, Index index, Greater greater) : raw_file_name(fileName), primary_key(primaryKey), index(index), greater(greater) {
        hash_file_name = raw_file_name + ".ehash";
        index_file_name = raw_file_name + ".ehashind";
        // Create needed files if they don't exist
        SAFE_FILE_CREATE_IF_NOT_EXISTS(hash_file, hash_file_name)
        SAFE_FILE_CREATE_IF_NOT_EXISTS(index_file, index_file_name)
        // Load or create index file
        SAFE_FILE_OPEN(index_file, index_file_name, flags)
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        SEEK_ALL(index_file, 0)
        SEEK_ALL(hash_file, 0)
        // If the index file is empty, initialize the index
        if (index_file.peek() == std::ifstream::traits_type::eof() && hash_file.peek() == std::ifstream::traits_type::eof()) {//
            SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
            // Data file is empty, just initialize an empty index and an empty hash file with an empty bucket
            hash_index = new ExtendibleHash<global_depth>{};
            Bucket<RecordType> emptyBucket{};
            SEEK_ALL(hash_file, 0)
            PRINT_FLAGS(hash_file)
            hash_file.write((char *) &emptyBucket, sizeof(emptyBucket));
            PRINT_FLAGS(hash_file)
            hash_file.close();
            // Data file is not empty, construct the index accordingly (insert the entries)
            if (raw_file.peek() != std::ifstream::traits_type::eof()) {
                // Construct hash file (.ehash)
                RecordType record{};
                while (!raw_file.eof()) {
                    raw_file.read((char *) &record, sizeof(RecordType));
                    if (!raw_file.eof()) {
                        insert(record);
                    }
                }
            }
            raw_file.close();
        } else if ((index_file.peek() != std::ifstream::traits_type::eof() && hash_file.peek() == std::ifstream::traits_type::eof()) || (index_file.peek() == std::ifstream::traits_type::eof() && hash_file.peek() != std::ifstream::traits_type::eof())) {
            hash_file.close();
            index_file.close();
            throw std::runtime_error("Corrupt ExtendibleHashFile file structure.");
        }
        // The index file is not empty, read its contents
        else {
            hash_file.close();
            hash_index = new ExtendibleHash<global_depth>{index_file};
            //            hash_index->lookup("101");
        }
        index_file.close();
    }

    std::vector<RecordType> search(KeyType key) {
        std::vector<RecordType> result;
        //        SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
        //        SAFE_FILE_OPEN(index_file, index_file_name, flags)
        //        std::string hash_sequence = std::bitset<global_depth>(hash_function(key) % global_depth).to_string();
        //        // Get the size of the index raw_file
        //        SEEK_ALL(index_file, 0, std::ios::end)
        //        std::size_t index_file_size = TELL(index_file);
        //        // Read the entire index raw_file
        //        SEEK_ALL(index_file, 0)
        //        char buffer[index_file_size];
        //        index_file.read(buffer, index_file_size);
        //        HashIndexPage<global_depth> hashIndexPage{buffer};
        //        raw_file.close();
        //        index_file.close();
        SAFE_FILE_OPEN(raw_file, raw_file_name, flags)
        std::string hash_sequence = std::bitset<global_depth>(hash_function(key) % global_depth).to_string();
        long bucket_ref = hash_index->lookup(hash_sequence);
        SEEK_ALL(raw_file, bucket_ref)
        Bucket<RecordType> bucket{};
        raw_file.read((char *) &bucket, BLOCK_SIZE);
        raw_file.close();
        return result;
    }

    void insert(RecordType &record) {
        SAFE_FILE_OPEN(hash_file, hash_file_name, flags)
        std::string hash_sequence = get_hash_sequence(record);
        long bucket_ref = hash_index->lookup(hash_sequence);
        std::cout << bucket_ref << std::endl;
        // Insert record into bucket bucket_ref of the hash file
        PRINT_TELL(hash_file)
        SEEK_ALL(hash_file, bucket_ref)
        PRINT_TELL(hash_file)
        // Read and update bucket bucket_ref
        PRINT_FLAGS(hash_file)
        Bucket<RecordType> bucket{};
        PRINT_SIZE(bucket)
        hash_file.read((char *) &bucket, sizeof(bucket));
        bucket.records[bucket.size++] = record;
        PRINT_FLAGS(hash_file)
        // Write bucket bucket_ref
        PRINT_TELL(hash_file)
        SEEK_ALL(hash_file, bucket_ref)
        PRINT_TELL(hash_file)
        hash_file.write((char *) &bucket, sizeof(bucket));
        hash_file.close();
    }

    virtual ~ExtendibleHashFile() {
        // Write hash_index to disk
        SAFE_FILE_OPEN(index_file, index_file_name, flags | std::ios::trunc)
        hash_index->write_to_disk(index_file);
        delete hash_index;
        index_file.close();
    }
};


#endif//EXTENDIBLE_HASH_EXTENDIBLEHASHFILE_HPP
