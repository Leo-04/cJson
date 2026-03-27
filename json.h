#pragma once

#include <stdbool.h>
#include <stdlib.h>

// JSON Types
typedef enum cJsonType{
    JNULL,
    JBOOL,
    JNUMBER,
    JSTRING,
    JARRAY,
    JOBJECT,

    JERROR,
} cJsonType;

// JSON Object
typedef struct cJsonObject{
    struct cJsonPair* objects;
    size_t length;
} cJsonObject;

// JSON Array
typedef struct cJsonArray{
    struct cJson* objects;
    size_t length;
} cJsonArray;

// JSON Value
typedef union cJsonValue{
    double number;
    bool boolean;
    char* string;
    cJsonObject object;
    cJsonArray array;
} cJsonValue;

// JSON
typedef struct cJson{
    cJsonType type;
    cJsonValue data;
} cJson;

// JSON Object Pair
struct cJsonPair{
    char* key;
    struct cJson value;
};

// Loading and unloading
cJson cJsonRead(const char*);
void cJsonDestroy(cJson*);
cJson cJsonCopy(cJson);
int cJsonDump(cJson, char*);
int cJsonFormatDump(cJson, int, char*);

// JSON Object operations
cJson cJsonFind(cJsonObject*, const char*);
void cJsonPut(cJsonObject*, const char*, cJson);
cJson cJsonDrop(cJsonObject*, const char*);

// Json Array Operatrions
cJson cJsonGet(cJsonArray*, ssize_t);
int cJsonSet(cJsonArray*, ssize_t, cJson);
int cJsonAdd(cJsonArray*, ssize_t, cJson);
cJson cJsonRemove(cJsonArray*, ssize_t);

// Creators
#define cJsonNull() {JNULL, {0}}
#define cJsonBoolean(value) {JBOOL, {.boolean=value}}
#define cJsonNumber(value) {JNUMBER, {.number=value}}
#define cJsonString(value) {JSTRING, {.string=strdup(value)}}
#define cJsonArray() {JARRAY, {.array.length = 0, .array.objects = NULL}}
#define cJsonObject() {JARRAY, {.objects.length = 0, .objects.objects = NULL}}