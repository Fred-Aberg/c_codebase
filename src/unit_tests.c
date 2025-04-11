#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "common.h"
#include "string.h"
#include "../Unity/unity.h"
// str is valid if
	// Min len is 1
	// Length cannot exceed capacity
	// Str's char array must be null terminated
#define TEST_VALIDATE_STRING(str) 						\
TEST_ASSERT_TRUE(str->length != 0); 					\
TEST_ASSERT_TRUE(str->length <= str->capacity); 		\
TEST_ASSERT_EQUAL_INT(str_get(str, str->length - 1), 0)	

// used to simulate bad things happening and junk data
#define sneaky_write(str, i, val) (&str->c)[i] = val

void setUp(void) {};

void tearDown(void) {};

void str_initialization()
{
	str_t *str = str_empty(10);
	
	TEST_ASSERT_EQUAL_UINT(str->capacity, 10);
	str->length = 8;
	str_realloc(&str, 5);
	TEST_ASSERT_EQUAL_UINT(str->capacity, 5);
	TEST_ASSERT_EQUAL_UINT(str->length, 5);

	free(str);

	char buf[36] = "not 36 long";
	str = str_from_buf(buf, 36);
	TEST_ASSERT_FALSE(str->length == 36);
	TEST_ASSERT_EQUAL_UINT(12, str->length);
	TEST_VALIDATE_STRING(str);

	free(str);

	str = str("abc");
	TEST_ASSERT_EQUAL_UINT(4, str->length);
	TEST_ASSERT_FALSE(strcmp(str_charr(str), "abc"));
	TEST_VALIDATE_STRING(str);
	
	free(str);
}

void str_writing()
{
	str_t *str = str_empty(5);
	TEST_ASSERT_FALSE(strcmp(str_charr(str), ""));
	TEST_ASSERT_EQUAL_UINT(1, str->length);

	sneaky_write(str, 1, '$'); // simulate junk data -> "\0$"
	
	str_append(&str, 'a');	
	TEST_ASSERT_TRUE(str_get(str, 0) == 'a');
	TEST_ASSERT_TRUE(str_get(str, 1) == '\0'); // "\0$" -> "a\0"
	
	str_append(&str, 'b');
	TEST_ASSERT_TRUE(str_get(str, 1) == 'b');
	TEST_VALIDATE_STRING(str);
	TEST_ASSERT_EQUAL_UINT(3, str->length);
	

	TEST_ASSERT_FALSE(strcmp(str_charr(str), "ab"));
	
	str_write(&str, "ABCD");
	TEST_VALIDATE_STRING(str);
	TEST_ASSERT_EQUAL_UINT(5, str->length);
	TEST_ASSERT_EQUAL_UINT(5, str->capacity);

	char buf[36] = "not 36 long";
	str_write_from_buf(&str, buf, 36);
	TEST_ASSERT_FALSE(str->length == 36);
	TEST_ASSERT_EQUAL_UINT(12, str->length);
	TEST_VALIDATE_STRING(str);

	free(str);
	str = str_empty(0);
	char buf_2[10] = "Hej";

	str_write_from_buf(&str, buf_2, 10);
	TEST_ASSERT_EQUAL_UINT(4, str->length); // str = {H,e,j,\0}

	free(str);
	str = str_empty(3);
	str_t *str_2 = str("long text");
	
	str_write_from_str(&str, str_2);
	TEST_ASSERT_EQUAL_UINT(10, str->length); // str = {H,e,j,\0}


	
	free(str);
	free(str_2);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(str_initialization);
    RUN_TEST(str_writing);
    return UNITY_END();
}
