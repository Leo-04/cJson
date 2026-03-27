#include "json.h"
#include "logger.h"

#include <string.h>
#include <stdlib.h>
#include <float.h>

static int ParseJson(const char* contents, cJson* json);

// Moving operations

static int cmp(const void* s1, const void* s2){
    return strcmp(*(const char**)s1, *(const char**)s2);
}

/*
* Puts a value into a json object
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   key: const char*
*       The key of the value
*
*   value: cJson
*       The value
*/
void cJsonPut(cJsonObject* json, const char* key, cJson value){
    LogAssertNull(json);
    LogAssertNull(key);

    // Find pair
    struct cJsonPair* ptr =NULL;
    if (json->objects != NULL && json->length != 0){
        ptr = bsearch(&key, json->objects, json->length, sizeof(struct cJsonPair), cmp);
    }

    // If it does not exsist
    if (ptr == NULL){
        // Allocate more space
        json->length++;
        json->objects = realloc(json->objects, json->length * sizeof(struct cJsonPair));
        LogAssertNull(json->objects);

        // Add it
        json->objects[json->length - 1].key = strdup(key);
        json->objects[json->length - 1].value = value;
        qsort(json->objects, json->length, sizeof(struct cJsonPair), cmp);
    } else {
        // Remove the old object and add the new one
        cJsonDestroy(&ptr->value);
        ptr->value = value;
    }
}

/*
* Gets a value from a json object
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   key: const char*
*       The key of the value
*
*   value: cJson
*       The value
*
* Returns:
*   a json value
*/
cJson cJsonFind(cJsonObject* json, const char* key){
    LogAssertNull(json);
    LogAssertNull(key);
    struct cJsonPair* ptr = bsearch(&key, json->objects, json->length, sizeof(struct cJsonPair), cmp);
    if (ptr == NULL){
        return (cJson){JERROR, {0}};
    }
    return ptr->value;
}

/*
* Adds a value to a json array at a given index
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   index: ssize_t
*       The index of the value
*       If this value is negative, will work like python
*
*   value: cJson
*       The value
*
* Returns:
*   a json value
*/
int cJsonAdd(cJsonArray* array, ssize_t index, cJson value){
    // Check index
    LogAssertNull(array);
    if (index < 0) index += array->length; // Python style
    if ((size_t) index > array->length || index < 0){
        LogError(
            "Index out of bounds: %i/%i",
            index, array->length
        );
        return 1;
    }

    // Resize array
    array->length++;
    array->objects = realloc(array->objects, array->length * sizeof(struct cJson));
    for (size_t i = array->length - 1; (size_t)index < i; i--){
        array->objects[i] = array->objects[i - 1];
    }

    //Assign
    array->objects[index] = value;

    return 0;
}

/*
* Removes a value from a json array
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   index: ssize_t
*       The index of the value
*       If this value is negative, will work like python
*
* Returns:
*   a json value
*/
cJson cJsonRemove(cJsonArray* array, ssize_t index){
    // Check index
    LogAssertNull(array);
    if (index < 0) index += array->length;
    if ((size_t)index >= array->length || index < 0){
        LogError(
            "Index out of bounds: %i/%i",
            index, array->length
        );
        return (cJson){JERROR, {0}};
    }

    // Get current object
    cJson object = array->objects[index];

    // Resize array
    for (size_t i = index; i < array->length; i++){
        array->objects[i] = array->objects[i + 1];
    }
    array->length--;
    array->objects = realloc(array->objects, sizeof(cJson) * array->length);

    // Return object
    return object;
}

/*
* Sets a value for a json array
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   index: ssize_t
*       The index of the value
*       If this value is negative, will work like python
*
*   value: cJson
*       The value
*
* Returns:
*   0 on success
*/
int cJsonSet(cJsonArray* array, ssize_t index, cJson data){
    // Check index
    LogAssertNull(array);
    if (index < 0) index += array->length;
    if ((size_t)index >= array->length || index < 0){
        LogError(
            "Index out of bounds: %i/%i",
            index, array->length
        );
        return 1;
    }

    // Assign value;
    array->objects[index] = data;
    return 0;
}

/*
* Gets a value from a json array
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   index: ssize_t
*       The index of the value
*
* Returns:
*   a json value
*/
cJson cJsonGet(cJsonArray* array, ssize_t index){
    LogAssertNull(array);
    if (index < 0) index += array->length;
    if ((size_t)index >= array->length || index < 0){
        LogError(
            "Index out of bounds: %i/%i",
            index, array->length
        );
        return (cJson){JERROR, {0}};
    }
    return array->objects[index];
}


// Parsing functions


/*
* Parses a string
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   json: cJson*
*       The object to insert the value into
*
* Returns:
*   The index of last parsed character 
*/
static int ParseString(const char* contents, cJson* json){
    LogAssertNull(contents);
    LogAssertNull(json);

    LogDebug("Parse String: %s", contents);
    
    // Check starting char
    json->type = JSTRING;
    if (contents[0] != '"'){
        LogError("Expected '\"' at 0, found: %i'%c'", contents[0], contents[0]);
        json->type = JERROR;
        return 0;
    }

    // Main loop
    int special = 0;
    int i = 1;
    while(true){
        LogTrace("i = %i", i);

        // Check for end of string
        if (contents[i] == '\0'){
            i--;
            LogError("Unexpected EoF found at %i", i);
            json->type = JERROR;
            return i;
        }

        // Check for escape characters
        else if (contents[i] == '\\' && contents[i + 1] != '\0'){
            i++;
            special++;
            // Ignore unicode, but still parse it
            if (contents[i] == 'U' || contents[i] == 'u'){
                special-=1;
                if (contents[i+1] == '\0' || contents[i+2] == '\0' || contents[i+3] == '\0' || contents[i+4] == '\0'){
                    i--;
                    LogError("Unexpected EoF found at %i, expected 2 byte unicode character", i);
                    json->type = JERROR;
                    return i;
                }
                i+=3;
            }
        }

        // Check for end of string
        else if (contents[i] == '"'){
            break;
        }

        i++;
    }

    // Create buffer of token
    int length = i-1;
    char buffer[length + 1];
    strncpy(buffer, contents + 1, i-1);
    buffer[length] = '\0';

    // Alloc real string
    int new_length = length - special;
    json->data.string = malloc(sizeof(char) * (new_length + 1));

    // Copy to real string including special characters
    int index = 0;
    for (int i = 0; i <= length; i++){
        if (buffer[i] == '\\'){
            i++;
            if (buffer[i] == 'n'){
                json->data.string[index] = '\n';
            } else if (buffer[i] == 'r'){
                json->data.string[index] = '\r';
            } else if (buffer[i] == 'f'){
                json->data.string[index] = '\f';
            } else if (buffer[i] == 'b'){
                json->data.string[index] = '\b';
            } else if (buffer[i] == 't'){
                json->data.string[index] = '\t';
            } else if (buffer[i] == 'U' || buffer[i] == 'u'){
                json->data.string[index] = '\\';
                index++;
                json->data.string[index] = 'u';
            } else {
                json->data.string[index] = buffer[i];
            }
        } else {
            json->data.string[index] = buffer[i];
        }
        index++;
    }
    json->data.string[new_length] = '\0';

    LogDebug("Parsed String(len:%i): \"%s\"", new_length, json->data.string);
    return i;
}

/*
* Parses a number
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   json: cJson*
*       The object to insert the value into
*
* Returns:
*   The index of last parsed character 
*/
static int ParseNumber(const char* contents, cJson* json){
    LogAssertNull(contents);
    LogAssertNull(json);

    LogDebug("Parse Number: %s", contents);

    // Check to make sure its a number
    if (('0' > contents[0] || contents[0] > '9') && contents[0] != '-'){
        LogError("Expected a number at 0, found: %i'%c'", contents[0], contents[0]);
        json->type = JERROR;
        return 0;
    }

    json->type = JNUMBER;
    
    // Parse number
    char* end;
    json->data.number = strtold(contents, &end);

    LogDebug("Parsed number: %lf", json->data.number);

    return end - contents - 1;
}

/*
* Parses a array
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   json: cJson*
*       The array to insert the value into
*
* Returns:
*   The index of last parsed character
*/
static int ParseJsonArray(const char* contents, cJson* json){
    LogAssertNull(contents);
    LogAssertNull(json);

    LogDebug("Parse Array: %s", contents);
    
    // Check if its an array
    json->type = JARRAY;
    if (contents[0] != '['){
        LogError("Expected '[' at 0, found: %i'%c'", contents[0], contents[0]);
        json->type = JERROR;
        return 0;
    }

    // initialise array struct
    json->data.array.objects = NULL;
    json->data.array.length = 0;

    // Main loop
    int i = 1;
    bool comma = false;
    while(true){
        LogTrace("i = %i", i);

        // Check for end of string
        if (contents[i] == '\0'){
            i--;
            LogError("Unexpected EoF found at %i", i);
            json->type = JERROR;
            return i;
        }

        //Check for end of array
        else if (contents[i] == ']'){
            break;
        }

        // Ignore whitespaces here
        else if (
            contents[i] == ' '
            || contents[i] == '\t'
            || contents[i] == '\n'
            || contents[i] == '\r'
        ){;}

        // If we are not expecting a comma, we expect json
        else if (!comma){
            LogTrace("Found JSON");

            cJson object;
            i += ParseJson(contents + i, &object);
            if (object.type == JERROR){
                json->type = JERROR;
                LogErrorTraceReturn(i);
            }
            if (cJsonAdd(&json->data.array, json->data.array.length, object)){
                json->type = JERROR;
                LogErrorTraceReturn(i);
            }
            comma = true;
        
        // If we are expecting a comma, and find it
        } else if (comma && contents[i] == ','){
            LogTrace("Found COMMA");
            comma = false;
        }
        
        // Unknown character
        else {
            json->type = JERROR;
            LogError("Unexpected character at %i: %i'%c'", i, contents[i], contents[i]);
            return i;
        }
        i++;
    }

    LogDebug("Parsed Array Size: %i", json->data.array.length);
    return i;
}

/*
* Parses a object
*
* Parameters:
*   json: cJsonObject*
*       The object to put the value in
*
*   json: cJson*
*       The object to insert the value into
*
* Returns:
*   The index of last parsed character
*/
static int ParseJsonObject(const char* contents, cJson* json){
    LogAssertNull(contents);
    LogAssertNull(json);

    LogDebug("Parse Object: %s", contents);
    
    // Check its an object
    json->type = JOBJECT;
    if (contents[0] != '{'){
        LogError("Expected '{' at 0, found: %i'%c'", contents[0], contents[0]);
        json->type = JERROR;
        return 0;
    }

    // Initalise struct
    json->data.object.objects = NULL;
    json->data.object.length = 0;

    // Main loop
    int i = 1;
    const int STRING = 0;
    const int COLON = 1;
    const int JSON = 2;
    const int COMMA = 3;
    int state = STRING;
    char* key = NULL;
    while(true){
        LogTrace("i = %i", i);

        // Check if at end of string
        if (contents[i] == '\0'){
            i--;
            LogError("Unexpected EoF found at %i", i);
            json->type = JERROR;
            if (key != NULL) free(key);
            return i;
        }

        // Check for end of object (make sure we can end it here too)
        else if (contents[i] == '}' && (state == STRING || state == COMMA)){
            break;
        }

        // Ignore whitespaces here
        else if (
            contents[i] == ' '
            || contents[i] == '\t'
            || contents[i] == '\n'
            || contents[i] == '\r'
        ){;}
        
        // If we are looking for a string
        else if (state == STRING && contents[i] == '"'){
            LogTrace("Found STRING");
            cJson json_key;
            i += ParseString(contents + i, &json_key);
            if (json_key.type == JERROR){
                json->type = JERROR;
                if (key != NULL) free(key);
                LogErrorTraceReturn(i);
            }
            state = COLON;

            key = json_key.data.string;
        }
        
        // If we are looking for a `:`
        else if (state == COLON && contents[i] == ':'){
            LogTrace("Found COLON");
            state = JSON;
        }
        
        // If we are looking for json
        else if (state == JSON){
            LogTrace("Found JSON");
            cJson object;
            i += ParseJson(contents + i, &object);
            if (object.type == JERROR){
                json->type = JERROR;
                if (key != NULL) free(key);
                LogErrorTraceReturn(i);
            }
            state = COMMA;

            // Put value pair
            cJsonPut(&json->data.object, key, object);
            free(key); // Need to free key
            key = NULL;
        }
        
        // If we are looking for a `,`
        else if (state == COMMA && contents[i] == ','){
            LogTrace("Found COMMA");
            state = STRING;
        }
        
        // Unexpected character
        else {
            json->type = JERROR;
            LogError("Unexpected character at %i: %i'%c'", i, contents[i], contents[i]);
            if (key != NULL) free(key);
            return i;
        }
        i++;
    }

    LogDebug("Parsed Object Size: %i", json->data.array.length);
    if (key != NULL) free(key);
    return i;
}

/*
* Parse any json object
*
* Parameters:
*   contents: char*
*       The string to parse though
*
*   index: int*
*       The index to start parsing from
*
* Returns:
*   The index of last parsed character
*/
static int ParseJson(const char* contents, cJson* json){
    LogAssertNull(contents);
    LogAssertNull(json);

    LogDebug("Parse Json: %s", contents);

    // Main loop
    int i = 0;
    while(true){
        LogTrace("i = %i", i);

        // Check for end of string
        if (contents[i] == '\0'){
            i--;
            LogError("Unexpected EoF found at %i", i);
            json->type = JERROR;
            return i;
        }

        // Check for true
        else if (contents[i] == 't' && strncmp(contents+i, "true", 4)==0){
            LogDebug("Found true");
            json->type = JBOOL;
            json->data.boolean = true;
            return i + 3;
        }

        // Check for false
        else if (contents[i] == 'f' && strncmp(contents+i, "false", 5)==0){
            LogDebug("Found false");
            json->type = JBOOL;
            json->data.boolean = false;
            return i + 4;
        }

        // Check for null
        else if (contents[i] == 'n' && strncmp(contents+i, "null", 4)==0){
            LogDebug("Found null");
            json->type = JNULL;
            return i + 3;
        }

        // Check for string
        else if (contents[i] == '"'){
            json->type = JSTRING;
            return i + ParseString(contents + i, json);
        }

        // Check for number
        else if (('0' <= contents[i] && contents[i] <= '9') || contents[i] == '-'){
            json->type = JNUMBER;
            return i + ParseNumber(contents + i, json);
        }

        // Check for array
        else if (contents[i] == '['){
            json->type = JARRAY;
            return i + ParseJsonArray(contents + i, json);
        } 

        // Check for object
        else if (contents[i] == '{'){
            json->type = JOBJECT;
            return i + ParseJsonObject(contents + i, json);
        }
        
        // If not whitespace, then a unexpected character
        else if (
            contents[i] != ' '
            && contents[i] != '\t'
            && contents[i] != '\n'
            && contents[i] != '\r'
        ){
            json->type = JERROR;
            LogError("Unexpected character at %i: %i'%c'", i, contents[i], contents[i]);
            return i;
        }

        i++;
    }
    return i;
}

// Loading and unloading

/*
* Parse any json object from a string
*
* Parameters:
*   contents: char*
*       The string to parse though
*
* Returns:
*   a json value
*/
cJson cJsonRead(const char* contents){
    cJson json;
    size_t i = ParseJson(contents, &json) + 1;

    // Check if there is extra content
    while (contents[i] != '\0'){
        if (
               contents[i] != ' '
            && contents[i] != '\t'
            && contents[i] != '\n'
            && contents[i] != '\r'
        ){
            LogError("Extra content in JSON string: %s", contents+i);
            json.type = JERROR;
            break;
        }
        i++;
    }

    // If there was an error, set the number to the index
    if (json.type == JERROR) json.data.number = i;

    return json;
}

/*
* Dumps any json object to buffer
*
* Parameters:
*   json: cJson
*       The json to jump
*
*   out: char*
*       The buffer to dump to
*       Can set to NULL to get buffer size
*
* Returns:
*   The number of characters to write to buffer
*/
int cJsonDump(cJson json, char* out){
    //Keep track of number of characters
    int n = 0;

    // Add Nul byte
    if (out != NULL) out[0] = '\0';

    switch(json.type){
        case JNULL:{
            if (out != NULL) strcat (out, "null");
            n += 4;
            break;
        }
        
        case JBOOL: {
            n += json.data.boolean? 4: 5;
            if (out != NULL) strncat(out, json.data.boolean?"true": "false", n);
            break;
        }
        
        case JNUMBER: {
            int size = 0;
            // Check if there is any decimal point
            if (json.data.number == (long long int) json.data.number){
                size = snprintf(NULL, 0, "%lli", (long long int) json.data.number) + 1;
            } else {
                size = snprintf(NULL, 0, "%lf", json.data.number) + 1;
            }
            n += size;

            // Output if needed
            if (out != NULL){
                char buffer[size + 1];
                // Check if there is any decimal point
                if (json.data.number == (long long int) json.data.number){
                    snprintf(buffer, size, "%lli", (long long int) json.data.number);
                } else {
                    snprintf(buffer, size, "%lf", json.data.number);
                }
                buffer[size] = '\0';
                strncat(out, buffer, size);
            }

            break;
        }
        
        case JSTRING: {
            // Start of string
            n += 1;
            if (out != NULL) strcat(out, "\"");

            // Parse special characters
            for (size_t i = 0; i < strlen(json.data.string); i++){
                if (json.data.string[i] == '\n'){
                    if (out != NULL) strcat(out, "\\n");
                    n += 2;
                } else if (json.data.string[i] == '\t'){
                    if (out != NULL) strcat(out, "\\t");
                    n += 2;
                } else if (json.data.string[i] == '\r'){
                    if (out != NULL) strcat(out, "\\r");
                    n += 2;
                } else if (json.data.string[i] == '\f'){
                    if (out != NULL) strcat(out, "\\f");
                    n += 2;
                } else if (json.data.string[i] == '\b'){
                    if (out != NULL) strcat(out, "\\b");
                    n += 2;
                } else if (json.data.string[i] == '\\'){
                    if (out != NULL) strcat(out, "\\\\");
                    n += 2;
                } else if (json.data.string[i] == '"'){
                    if (out != NULL) strcat(out, "\\\"");
                    n += 2;
                } else {
                    if (out != NULL) strncat(out, &json.data.string[i], 1);
                    n += 1;
                }
            }

            // End of string
            n += 1;
            if (out != NULL) strcat(out, "\"");

            break;
        }
        
        case JARRAY: {
            // Start of array
            n += 1;
            if (out != NULL) strcat(out, "[");

            for (size_t i = 0; i < json.data.array.length; i++){
                // Get size of object
                int size = cJsonDump(json.data.array.objects[i], NULL);
                n += size;

                // Dump object
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonDump(json.data.array.objects[i], buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                }

                // Add comma
                if (i != json.data.array.length - 1){
                    if (out != NULL) strcat(out, ", ");
                    n += 2;
                }
            }

            // End of array
            n += 1;
            if (out != NULL) strcat(out, "]");

            break;
        }
        
        case JOBJECT: {
            // Start of object
            n += 1;
            if (out != NULL) strcat(out, "{");

            for (size_t i = 0; i < json.data.object.length; i++){
                // Dump key as string
                LogDebug("Key: %s", json.data.object.objects[i].key);
                int size = cJsonDump((cJson){
                    JSTRING, {.string=json.data.object.objects[i].key}
                }, NULL);
                n += size + 2; // +2 for ": "
                
                // Dump string into buffer
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonDump((cJson){
                        JSTRING, {.string=json.data.object.objects[i].key}
                    }, buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                    
                    // Include colon
                    strcat(out, ": ");
                }

                // Dump object
                size = cJsonDump(json.data.object.objects[i].value, NULL);
                n += size;
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonDump(json.data.object.objects[i].value, buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                }

                // Add comma
                if (i != json.data.object.length - 1){
                    if (out != NULL) strcat(out, ", ");
                    n += 2;
                }
            }
            n += 1;
            if (out != NULL) strcat(out, "}");
            break;
        }
        
        case JERROR: {
            if (out != NULL) strcat(out, "ERROR");
            n += 5;
            break;
        }
    }

    // Log debug information
    if (out == NULL){
        LogDebug("Size: %i", n);
    } else {
        LogDebug("String(%i): %s", n, out);
    }

    return n;
}

/*
* Dumps any json object to buffer with extra formating
*
* Parameters:
*   json: cJson
*       The json to jump
*
*   indent: int
*       The current indentaion
*
*   out: char*
*       The buffer to dump to
*       Can set to NULL to get buffer size
*
* Returns:
*   The number of characters to write to buffer
*/
int cJsonFormatDump(cJson json, int indent, char* out){
    //Keep track of number of characters
    int n = 0;

    // Add Nul byte
    if (out != NULL) out[0] = '\0';

    switch(json.type){
        case JNULL:
        case JBOOL: 
        case JNUMBER: 
        case JSTRING:{
            return cJsonDump(json, out);
        }
        
        case JARRAY: {
            // Start of array
            n += 2;
            if (out != NULL) strcat(out, "[\n");

            // Increase Indent
            indent++;

            for (size_t i = 0; i < json.data.array.length; i++){
                // Indent
                for (int i = 0; i < indent; i++){
                    n += 4;
                    if (out != NULL) strcat(out, "    ");
                }

                // Get size of object
                int size = cJsonFormatDump(json.data.array.objects[i], indent, NULL);
                n += size;

                // Dump object
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonFormatDump(json.data.array.objects[i], indent, buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                }

                // Add comma
                if (i != json.data.array.length - 1){
                    if (out != NULL) strcat(out, ", ");
                    n += 2;
                }

                //New line
                n += 1;
                if (out != NULL) strcat(out, "\n");
            }

            // Decrease Indent
            indent--;

            // Indent
            for (int i = 0; i < indent; i++){
                n += 4;
                if (out != NULL) strcat(out, "    ");
            }

            // End of array
            n += 1;
            if (out != NULL) strcat(out, "]");

            break;
        }
        
        case JOBJECT: {
            // Start of object
            n += 2;
            if (out != NULL) strcat(out, "{\n");

            // Increase Indent
            indent++;

            for (size_t i = 0; i < json.data.object.length; i++){
                // Indent
                for (int i = 0; i < indent; i++){
                    n += 4;
                    if (out != NULL) strcat(out, "    ");
                }

                // Dump key as string
                LogDebug("Key: %s", json.data.object.objects[i].key);
                int size = cJsonDump((cJson){
                    JSTRING, {.string=json.data.object.objects[i].key}
                }, NULL);
                n += size + 2; // +2 for ": "
                
                // Dump string into buffer
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonDump((cJson){
                        JSTRING, {.string=json.data.object.objects[i].key}
                    }, buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                    
                    // Include colon
                    strcat(out, ": ");
                }

                // Dump object
                size = cJsonFormatDump(json.data.object.objects[i].value, indent, NULL);
                n += size;
                if (out != NULL){
                    char buffer[size + 1];
                    cJsonFormatDump(json.data.object.objects[i].value, indent, buffer);
                    buffer[size] = '\0';
                    strncat(out, buffer, size);
                }

                // Add comma
                if (i != json.data.object.length - 1){
                    if (out != NULL) strcat(out, ", ");
                    n += 2;
                }

                //New line
                n += 1;
                if (out != NULL) strcat(out, "\n");

            }

            // Decrease Indent
            indent--;

            // Indent
            for (int i = 0; i < indent; i++){
                n += 4;
                if (out != NULL) strcat(out, "    ");
            }

            // End of object
            n += 1;
            if (out != NULL) strcat(out, "}");
            break;
        }
        
        case JERROR: {
            if (out != NULL) strcat(out, "ERROR");
            n += 5;
            break;
        }
    }

    // Log debug information
    if (out == NULL){
        LogDebug("Size: %i", n);
    } else {
        LogDebug("String(%i): %s", n, out);
    }

    return n;
}

/*
* Destroys a json object, freeing all memory it owns
*
* Parameters:
*   json: cJsonObject*
*       The object to destroy
*/
void cJsonDestroy(cJson* json){
    if (json == NULL){
        LogWarning("Trying to destory NULL");
        return;
    }

    switch(json->type){
        // Don't need todo anything
        case JNULL:
        case JBOOL:
        case JNUMBER:
        case JERROR:
            break;
        
        case JSTRING:
            // Free string
            free(json->data.string);
            json->data.string = NULL;
            break;
        
        case JARRAY:
            // Destroy all sub-objects
            for (size_t i = 0; i < json->data.array.length; i++){
                cJsonDestroy(&json->data.array.objects[i]);
            }
            
            // Free array
            if (json->data.array.objects != NULL){
                free(json->data.array.objects);
                json->data.array.length = 0;
                json->data.array.objects = NULL;
            }

            break;
        case JOBJECT:
            // Destroy all sub-objects & keys
            for (size_t i = 0; i < json->data.object.length; i++){
                free(json->data.object.objects[i].key);
                cJsonDestroy(&json->data.object.objects[i].value);
            }

            // Free objects
            if (json->data.object.objects != NULL){
                free(json->data.object.objects);
                json->data.object.length = 0;
                json->data.object.objects = NULL;
            }
            
            break;
    }
}

/*
* Deep Copys a json object
*
* Parameters:
*   json: cJson
*       The json to copy
*
* Returns:
*   a json value
*/
cJson cJsonCopy(cJson json){
    cJson new;
    new.type = json.type;
    switch(json.type){
        // Simple data
        case JNULL:
        case JBOOL:
        case JNUMBER:
        case JERROR:
            return json;
        
        // Dup string
        case JSTRING:
            new.data.string = strdup(json.data.string);
            return new;
        
        // Dup array
        case JARRAY:
            new.data.array.length = 0;
            new.data.array.objects = NULL;
            for (size_t i = 0; i < json.data.array.length; i++){
                if (cJsonAdd(&new.data.array, i, json.data.array.objects[i])){
                    LogErrorTraceReturn(((cJson){JERROR, {0}}));
                }
            }
            return new;

        // Dup object
        case JOBJECT:
            new.data.object.length = 0;
            new.data.object.objects = NULL;
            for (size_t i = 0; i < json.data.object.length; i++){
                cJsonPut(&new.data.object, json.data.object.objects[i].key, json.data.object.objects[i].value);
            }
            return new;
        
        default:
            return (cJson){JERROR, {0}};
    }
}
