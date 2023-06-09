# Extendible Hash
Constant creation of buckets with a hashcode in order to store record on secondary memory, has a limit. Supports insertion, key-based search and remove.

## Public methods
`explicit operator bool()`
* Returns a bool that indicates whether the index has already been created.

`void create_index()`
* Constructs the hash index file from a fixed length binary data file.
* It creates 2 files: The directory file (.ehashdir) and the hash index (.ehash).
* Accesses to disk: O(n) where n is the total number of records in the data file.

`void insert(RecordType &record, long record_ref)`
* When overflow happens, a new bucket is pushed to the front of the overflow chain and linked, to allow for more efficient insertions.
* Throws an exception if the key of the record to be inserted is already present and the index is for a primary key.

`void remove(KeyType key)`
* Removes every record that matches the given key by marking it as removed on the data file.
* Does nothing if the key does not exist.
  
`std::vector<RecordType> search(KeyType key)`
* Searches a given key.
* Returns a vector of elements that match the given key.
* If the index was created for primary keys, it returns a single element.
* If no element matches the given key, it returns an empty vector.

## Algorithms Performance (In function of disk accesses)

$D := Number \ of \ global \ depth \ used \ for \ the \ buckets$

$K := Length \ of \ the \ bucket \ chain \ accessed$

If the **hash** is indexing a primary, secondary key or any other field:


|                  Member Function                  |     Performance      |                                                                      Description                                                                       |
|:-------------------------------------------------:|:--------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------:|
| ```insert(RecordType &record, long record_ref)``` | $\mathcal{O}(K + D)$ | As a hash function, access should be constant. However, we must consider bucket chaining and the idea of descending on the maximum depth of the index. |
|             ```search(KeyType key)```             |   $\mathcal{O}(K)$   |                               As a hash function, access should be constant. However, we must consider bucket chaining.                                |
|             ```remove(KeyType key)```             |   $\mathcal{O}(K)$   |                               As a hash function, access should be constant. However, we must consider bucket chaining.                                |

## Usage example
````c++
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
    bool removed{}; // < Note: As of now it is required that your structure implements a bool removed

    std::string to_string() {
        std::stringstream ss;
        ss << "("
           << dataId << ", " << contentType << ", " << title << ", " << length << ", " << releaseYear << ", "
           << endYear << ", " << votes << ", " << rating << ", " << gross << ", " << certificate
           << ", " << std::boolalpha << removed << ")";
        return ss.str();
    }
};

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
ExtendibleHashFile<char[16], MovieRecord, 16, std::function<char *(MovieRecord &)>, std::function<bool(char[16], char[16])>, std::function<std::size_t(char[16])>> extendible_hash_content_type{"movies_and_series.dat", "content_type", false, index, equal, hash};
if (!extendible_hash_content_type) {
    extendible_hash_content_type.create_index();
}
char str[16] = "movie\0";
auto result = extendible_hash_content_type.search(str);
std::cout << "Total: " << result.size() << std::endl;
````
See `main.cpp` for further reference on how to index different data types.
