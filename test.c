#include "json.h"
#include "logger.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

struct UnitTest{int mem_size; void* array; int length; int(*test)(void**); char*(*format)(void**);};

static int UnitTest(int mem_size, void* array, int length, int(*test)(void**), char*(*format)(void**)){
    int results[length];
    for (int i = 0; i < length; i++){
        LogTest("#Test %i:", i + 1);
        int result = test((void*)((uint8_t*)array + mem_size*i));
        LogTest("-Result %i: %i", i + 1, result);
        results[i] = result;
    }
    printf("--------------------- RESULTS ---------------------\n");
    int passed = 0;
    for (int i = 0; i < length; i++){
        if (results[i] == 0) passed++;
        char* fmt = format((void*)((uint8_t*)array + mem_size*i));
        
        #ifdef _LOG_COLOR_ENABLED
        printf("\033[%im", (results[i]? 91: 92));
        #endif

        printf("Test %5i %s exit-code %i: %s\n", i + 1, results[i]? "FAIL": "PASS", results[i], fmt);
        
        #ifdef _LOG_COLOR_ENABLED
        printf("\033[1;0m");
        #endif

        free(fmt);
    }
    printf("%i/%i passed (%i failed)\n", passed, length, length - passed);

    return passed;
}

static void UnitTests(struct UnitTest tests[], int n){
    int passed = 0;
    int n_tests = 0;
    for (int i = 0; i < n; i++){
        printf("---------------------SET %5i ---------------------\n", i + 1);

        passed += UnitTest(tests[i].mem_size, tests[i].array, tests[i].length, tests[i].test, tests[i].format);
        n_tests += tests[i].length;
    }
    printf("---------------------   TOTAL   ---------------------\n");
    printf("%i/%i passed (%i failed)\n", passed, n_tests, n_tests - passed);
}

char* parse_tests[] = {
    "null",
    "true",
    "false",
    "1234",
    "-1234",
    "1234.1234",
    "-1234.1234",
    "\"hello\"",
    "\"\"",
    
    "\"\\\"\"",
    "\"\\a\"",
    "\"\\\\a\"",
    "\"\\/\"",
    "\"\\u0000\"",
    "\"\\u0097\"",
    
    "[]",
    "[null]",
    "[true]",
    "[false]",
    "[1234]",
    "[-1234]",
    "[1234.1234]",
    "[-1234.1234]",
    "[\"hello\"]",

    "[null,null]",
    "[true,true]",
    "[false,false]",
    "[1234,1234]",
    "[-1234,-1234]",
    "[1234.1234,1234.1234]",
    "[-1234.1234,1234.1234]",
    "[\"hello\",\"hello\"]",

    "[null, null]",
    "[true, true]",
    "[false, false]",
    "[1234, 1234]",
    "[-1234, -1234]",
    "[1234.1234, 1234.1234]",
    "[-1234.1234, 1234.1234]",
    "[\"hello\", \"hello\"]",

    "[null, null,]",
    "[true, true,]",
    "[false, false,]",
    "[1234, 1234,]",
    "[-1234, -1234,]",
    "[1234.1234, 1234.1234,]",
    "[-1234.1234, 1234.1234,]",
    "[\"hello\", \"hello\",]",

    "[null, null, ]",
    "[true, true, ]",
    "[false, false, ]",
    "[1234, 1234, ]",
    "[-1234, -1234, ]",
    "[1234.1234, 1234.1234, ]",
    "[-1234.1234, 1234.1234, ]",
    "[\"hello\", \"hello\", ]",

    "{}",
    "{\"\":0}",
    "{\"\":0,}",
    "{\"\":0, }",
    "{\"\":0,\"\":0}",
    "{\"\":0,\"\":0,}",
    "{\"\":0, \"\":0, }",

    "{\"a\":null}",
    "{\"a\":true}",
    "{\"a\":false}",
    "{\"a\":1234}",
    "{\"a\":-1234}",
    "{\"a\":1234.1234}",
    "{\"a\":-1234.1234}",
    "{\"a\":\"hello\"}",
    
    "{\"a\": null }",
    "{\"a\": true }",
    "{\"a\": false }",
    "{\"a\": 1234 }",
    "{\"a\": -1234 }",
    "{\"a\": 1234.1234 }",
    "{\"a\": -1234.1234 }",
    "{\"a\": \"hello\" }",
    
    "{\"a\": null,}",
    "{\"a\": true,}",
    "{\"a\": false,}",
    "{\"a\": 1234,}",
    "{\"a\": -1234,}",
    "{\"a\": 1234.1234,}",
    "{\"a\": -1234.1234,}",
    "{\"a\": \"hello\",}",
    
    "{\"a\": null, }",
    "{\"a\": true, }",
    "{\"a\": false, }",
    "{\"a\": 1234, }",
    "{\"a\": -1234, }",
    "{\"a\": 1234.1234 }",
    "{\"a\": -1234.1234, }",
    "{\"a\": \"hello\", }",
    
    "{\"a\":null,\"b\":null}",
    "{\"a\":true,\"b\":true}",
    "{\"a\":false,\"b\":false}",
    "{\"a\":1234,\"b\":1234}",
    "{\"a\":-1234,\"b\":-1234}",
    "{\"a\":1234.1234,\"b\":1234.1234}",
    "{\"a\":-1234.1234,\"b\":-1234.1234}",
    "{\"a\":\"hello\",\"b\":\"hello\"}",
    
    "{\"a\":null, \"b\":null}",
    "{\"a\":true, \"b\":true}",
    "{\"a\":false, \"b\":false}",
    "{\"a\":1234, \"b\":1234}",
    "{\"a\":-1234, \"b\":-1234}",
    "{\"a\":1234.1234, \"b\":1234.1234}",
    "{\"a\":-1234.1234, \"b\":-1234.1234}",
    "{\"a\":\"hello\", \"b\":\"hello\"}",
    
    "{\"a\":null, \"b\":null,}",
    "{\"a\":true, \"b\":true,}",
    "{\"a\":false, \"b\":false,}",
    "{\"a\":1234, \"b\":1234,}",
    "{\"a\":-1234, \"b\":-1234,}",
    "{\"a\":1234.1234, \"b\":1234.1234,}",
    "{\"a\":-1234.1234, \"b\":-1234.1234,}",
    "{\"a\":\"hello\", \"b\":\"hello\",}",
    
    "{\"a\":null, \"b\":null, }",
    "{\"a\":true, \"b\":true, }",
    "{\"a\":false, \"b\":false, }",
    "{\"a\":1234, \"b\":1234, }",
    "{\"a\":-1234, \"b\":-1234, }",
    "{\"a\":1234.1234, \"b\":1234.1234, }",
    "{\"a\":-1234.1234, \"b\":-1234.1234, }",
    "{\"a\":\"hello\", \"b\":\"hello\", }",
};

char* string_tests[] = {
    "null",
    "true",
    "false",
    "1234",
    "-1234",
    "1234.123400",
    "-1234.123400",
    "\"hello\"",
    "\"\"",
    "\"\\\"\"",
    "\"\\\\a\"",
    
    "[]",
    "[null]",
    "[true]",
    "[false]",
    "[1234]",
    "[-1234]",
    "[1234.123400]",
    "[-1234.123400]",
    "[\"hello\"]",

    "[null, null]",
    "[true, true]",
    "[false, false]",
    "[1234, 1234]",
    "[-1234, -1234]",
    "[1234.123400, 1234.123400]",
    "[-1234.123400, 1234.123400]",
    "[\"hello\", \"hello\"]",

    "{}",
    "{\"\": 0}",
    "{\"a\": null}",
    "{\"a\": true}",
    "{\"a\": false}",
    "{\"a\": 1234}",
    "{\"a\": -1234}",
    "{\"a\": 1234.123400}",
    "{\"a\": -1234.123400}",
    "{\"a\": \"hello\"}",
    
    "{\"a\": null, \"b\": null}",
    "{\"a\": true, \"b\": true}",
    "{\"a\": false, \"b\": false}",
    "{\"a\": 1234, \"b\": 1234}",
    "{\"a\": -1234, \"b\": -1234}",
    "{\"a\": 1234.123400, \"b\": 1234.123400}",
    "{\"a\": -1234.123400, \"b\": -1234.123400}",
    "{\"a\": \"hello\", \"b\": \"hello\"}",
};

//from https://github.com/nst/JSONTestSuite/
char* fail_tests[] = {
    "[1 true]",
    "[a�]",
    "[\"\": 1]",
    "[\"\"],",
    "[,1]",
    "[1,,2]",
    "[\"x\",,]",
    "[\"x\"]]",
    "[\"\",]",
    "[\"x\"",
    "[x",
    "[3[4]]",
    "[�]",
    "[1:2]",
    "[,]",
    "[-]",
    "[   , \"\"]",
    "[\"a\",\n4\n,1,",
    "[1,]",
    "[1,,]",
    "[\"a\"\\f]",
    "[*]",
    "[\"\"",
    "[1,",
    "[1,\n1\n,1",
    "[{}",
    "[fals]",
    "[nul]",
    "[tru]",
    "123\0",
    "[++1234]",
    "[+1]",
    "[+Inf]",
    "[-01]",
    "[-1.0.]",
    "[-2.]",
    "[-NaN]",
    "[.-1]",
    "[.2e-3]",
    "[0.1.2]",
    "[0.3e+]",
    "[0.3e]",
    "[0.e1]",
    "[0e+]",
    "[0e]",
    "[0E+]",
    "[0E]",
    "[1.0e+]",
    "[1.0e-]",
    "[1.0e]",
    "[1eE2]",
    "[1 000.0]",
    "[2.e+3]",
    "[2.e-3]",
    "[2.e3]",
    "[9.e+]",
    "[1+2]",
    "[0x1]",
    "[0x42]",
    "[Inf]",
    "[Infinity]",
    "[0e+-1]",
    "[-123.123foo]",
    "[123�]",
    "[1e1�]",
    "[0�]\n",
    "[-Infinity]",
    "[-foo]",
    "[- 1]",
    "[NaN]",
    "[-012]",
    "[-.123]",
    "[-1x]",
    "[1ea]",
    "[1.]",
    "[1e�]",
    "[.123]",
    "[１]",
    "[1.2a-3]",
    "[1.8011670033376514H-308]",
    "[012]",
    "[\"x\", truth]",
    "{[: \"x\"}\n",
    "{\"x\", null}",
    "{\"x\"::\"b\"}",
    "{🇨🇭}",
    "{\"a\":\"a\" 123}",
    "{key: 'value'}",
    "{\"�\":\"0\",}",
    "{\"a\" b}",
    "{:\"b\"}",
    "{\"a\" \"b\"}",
    "{\"a\":",
    "{\"a\"",
    "{1:1}",
    "{9999E9999:1}",
    "{null:null,null:null}",
    "{\"id\":0,,,,,}",
    "{'a':0}",
    "{\"id\":0,}",
    "{\"a\":\"b\"}/**/",
    "{\"a\":\"b\"}/**//",
    "{\"a\":\"b\"}//",
    "{\"a\":\"b\"}/",
    "{\"a\":\"b\",,\"c\":\"d\"}",
    "{a: \"b\"}",
    "{\"a\":\"a",
    "{ \"foo\" : \"bar\", \"a\" }",
    "{\"a\":\"b\"}#",
    " ",
    "[\"\\uD800\\\"]",
    "[\"\\uD800\\u\"]",
    "[\"\\uD800\\u1\"]",
    "[\"\\uD800\\u1x\"]",
    "[é]",
    "[\"\\\0\"]",
    "[\"\\\\\\\"]",
    "[\"\\	\"]",
    "[\"\\🌀\"]",
    "[\"\\x00\"]",
    "[\"\\\"]",
    "[\"\\u00A\"]",
    "[\"\\uD834\\uDd\"]",
    "[\"\\uD800\\uD800\\x\"]",
    "[\"\\u�\"]",
    "[\"\\a\"]",
    "[\"\\uqqqq\"]",
    "[\"\\�\"]",
    "[\\u0020\"asd\"]",
    "[\\n]",
    "\"",
    "['single quote']",
    "abc",
    "[\"\\",
    "[\"a\0a\"]",
    "[\"new\nline\"]",
    "[\"	\"]",
    "\"\\UA66D\"",
    "\"\"x",
    "<.>",
    "[<null>]",
    "[1]x",
    "[1]]",
    "[\"asd]",
    "aå",
    "[True]",
    "1]",
    "{\"x\": true,",
    "[][]",
    "]",
    "�{}",
    "�",
    "[",
    "",
    "[\0]",
    "2@",
    "{}}",
    "{\"\":",
    "{\"a\":/*comment*/\"b\"}",
    "{\"a\": true} \"x\"",
    "['",
    "[,",
    "[{",
    "[\"a",
    "[\"a\"",
    "{",
    "{]",
    "{,",
    "{[",
    "{\"a",
    "{'a'",
    "[\"\\{[\"\\{[\"\\{[\"\\{",
    "�",
    "*",
    "{\"a\":\"b\"}#{}",
    "[\\u000A\"\"]",
    "[1",
    "[ false, nul",
    "[ true, fals",
    "[ false, tru",
    "{\"asd\":\"asd\"",
    "å",
    "﻿",
    "[]",
};

//from https://github.com/nst/JSONTestSuite/
char* pass_tests[] = {
    "[[]   ]",
    "[\"\"]",
    "[]",
    "[\"a\"]",
    "[false]",
    "[null, 1, \"1\", {}]",
    "[null]",
    "[1\n]",
    " [1]",
    "[1,null,null,null,2]",
    "[2] ",
    "[123e65]",
    "[0e+1]",
    "[0e1]",
    "[ 4]",
    "[-0.000000000000000000000000000000000000000000000000000000000000000000000000000001]\n",
    "[20e1]",
    "[-0]",
    "[-123]",
    "[-1]",
    "[-0]",
    "[1E22]",
    "[1E-2]",
    "[1E+2]",
    "[123e45]",
    "[123.456e78]",
    "[1e-2]",
    "[1e+2]",
    "[123]",
    "[123.456789]",
    "{\"asd\":\"sdf\", \"dfg\":\"fgh\"}",
    "{\"asd\":\"sdf\"}",
    "{\"a\":\"b\",\"a\":\"c\"}",
    "{\"a\":\"b\",\"a\":\"b\"}",
    "{}",
    "{\"\":0}",
    "{\"foo\\u0000bar\": 42}",
    "{ \"min\": -1.0e+28, \"max\": 1.0e+28 }",
    "{\"x\":[{\"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}], \"id\": \"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"}",
    "{\"a\":[]}",
    "{\"title\":\"\\u041f\\u043e\\u043b\\u0442\\u043e\\u0440\\u0430 \\u0417\\u0435\\u043c\\u043b\\u0435\\u043a\\u043e\\u043f\\u0430\" }",
    "{\n\"a\": \"b\"\n}",
    "[\"\\u0060\\u012a\\u12AB\"]",
    "[\"\\uD801\\udc37\"]",
    "[\"\\ud83d\\ude39\\ud83d\\udc8d\"]",
    "[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]",
    "[\"\\\\u0000\"]",
    "[\"\\\"\"]",
    "[\"a/*b*/c/*d//e\"]",
    "[\"\\\\a\"]",
    "[\"\\\\n\"]",
    "[\"\\u0012\"]",
    "[\"\\uFFFF\"]",
    "[\"asd\"]",
    "[ \"asd\"]",
    "[\"\\uDBFF\\uDFFF\"]",
    "[\"new\\u00A0line\"]",
    "[\"ï¿¿\"]",
    "[\"\\u0000\"]",
    "[\"\\u002c\"]",
    "[\"Ï€\"]",
    "[\"ð›¿¿\"]",
    "[\"asd \"]",
    "\" \"",
    "[\"\\uD834\\uDd1e\"]",
    "[\"\\u0821\"]",
    "[\"\\u0123\"]",
    "[\"â€¨\"]",
    "[\"â€©\"]",
    "[\"\\u0061\\u30af\\u30EA\\u30b9\"]",
    "[\"new\\u000Aline\"]",
    "[\"\"]",
    "[\"\\uA66D\"]",
    "[\"\\u005C\"]",
    "[\"\\u0022\"]",
    "[\"\\uDBFF\\uDFFE\"]",
    "[\"\\uD83F\\uDFFE\"]",
    "[\"\\u200B\"]",
    "[\"\\u2064\"]",
    "[\"\\uFDD0\"]",
    "[\"\\uFFFE\"]",
    "[\"aa\"]",
    "false",
    "42",
    "-0.1",
    "null",
    "\"asd\"",
    "true",
    "\"\"",
    "[\"a\"]\n",
    "[true]",
    " [] ",
};


static int TestParseJson(void** ptr_s){
    char* s = *ptr_s;
    cJson json = cJsonRead(s);
    if (json.type == JERROR) return 2;

    return 0;
}
static int TestNoParseJson(void** ptr_s){
    char* s = *ptr_s;
    cJson json = cJsonRead(s);
    if (json.type == JERROR) return 0;
    return 1;
}
static int TestStringJson(void** ptr_s){
    char* s = *ptr_s;
    cJson json = cJsonRead(s);
    if (json.type == JERROR) return 2;

    int new_size = cJsonDump(json, NULL);
    if (new_size <= 0) return 3;
    char json_string[new_size + 1];
    json_string[0] = '\0';

    if (cJsonDump(json, json_string) != new_size){
        return 4;
    }
    cJsonDestroy(&json);

    LogTest("EQ: %s==%s", s, json_string);
    return strcmp(json_string, s) != 0;
}
static char* StringCase(void** s){
    return strdup(*s);
}

static void TestJson(void){
    struct UnitTest tests[] = {
        {sizeof(char*), parse_tests, sizeof(parse_tests)/sizeof(parse_tests[0]), TestParseJson, StringCase},
        {sizeof(char*), string_tests, sizeof(string_tests)/sizeof(string_tests[0]), TestStringJson, StringCase},
        {sizeof(char*), pass_tests, sizeof(pass_tests)/sizeof(pass_tests[0]), TestParseJson, StringCase},
        {sizeof(char*), fail_tests, sizeof(fail_tests)/sizeof(fail_tests[0]), TestNoParseJson, StringCase},
    };
    UnitTests(tests, sizeof(tests)/sizeof(tests[0]));
}

int main(){
    TestJson();

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