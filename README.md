# cJson
A simple JSON parser for C

Example useage:

```c
int main(){
    // Loading json
    char* json_string = "{\"a\": 4}";
    cJson object = cJsonRead(json_string);
    if (object.type == JERROR){
        LogError("Cannot load JSON, invalid charater at index: %i '%c'", (int) object.data.number, json_string[(int) object.data.number]);
        return 1;
    }

    // Geting values
    cJson a = cJsonFind(&object.data.object, "a");
    if (object.type == JERROR){
        LogError("Cannot find key 'a'");
        return 1;
    }
    LogAssert(a.type != JNUMBER);

    // Setting values
    a.data.number = 100;
    cJsonPut(&object.data.object, "a", a);

    //Creating vales
    cJson b = cJsonArray();
    cJsonAdd(&b.data.array, b.data.array.length, (cJson) cJsonNull());
    cJsonAdd(&b.data.array, b.data.array.length, (cJson) cJsonString("hello world"));
    cJsonAdd(&b.data.array, b.data.array.length, (cJson) cJsonNumber(1234));
    cJsonAdd(&b.data.array, b.data.array.length, (cJson) cJsonBoolean(true));

    cJsonPut(&object.data.object, "b", b);

    // Outputing Json
    char buffer[cJsonDump(object, NULL) + 1];
    cJsonDump(object, buffer);
    printf("%s\n", buffer);

    char buffer2[cJsonFormatDump(object, 0, NULL) + 1];
    cJsonFormatDump(object, 0, buffer2);
    printf("%s\n", buffer2);

    // Clean up
    cJsonDestroy(&object);
}
```

## Compile tests

To compile `test.c` run:
- `gcc -D_LOG_COLOR_ENABLED *.c -o test` for colored logging
- `gcc -D_NO_LOGGING *.c -o test` for no logging
