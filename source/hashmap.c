#define TEMP_HASHMAP_SIZE 128

// Note(abi): used to map variables to their (integer) location on the stack frame
typedef struct HashmapValue HashmapValue;
struct HashmapValue {
    int set;
    int value;
};

typedef struct Hashmap Hashmap;
struct Hashmap {
    int size;
    HashmapValue values[TEMP_HASHMAP_SIZE];
};

int Hash(char * string, int string_length) {
    // djb2 algotihm
    unsigned int hash = 5381;
    for(char c = 0; c < string_length; ++c) {
        int i = string[c];
        hash = ((hash << 5) + hash + i);
    }
    return hash;
}

int HashmapContains(Hashmap * map, char * string, int string_length) {
    int key = Hash(string, string_length) % map->size;
    return map->values[key].set;
}

void HashmapPut(Hashmap * map, char * string, int string_length, int value) {
    if(HashmapContains(map, string, string_length))
        printf("[Info] attempting to set a hashmap value that is already filled.");
    
    int key = Hash(string, string_length) % map->size;
    map->values[key] = (HashmapValue){1, value};
}

int HashmapGet(Hashmap * map, char * string, int string_length) {
    int key = Hash(string, string_length) % map->size;
    // todo check set
    return map->values[key].value;
}