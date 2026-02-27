#include "ascui.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "common.h"
#include "string.h"
#include "../Unity/unity.h"
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
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

void pos_lists()
{
	pos8_t p = pos8(0,1);
	TEST_ASSERT_EQUAL_UINT(pos8_to_ui16(p), 256);
	p = pos8(1,0);
	TEST_ASSERT_EQUAL_UINT(pos8_to_ui16(p), 1);
	p = pos8(1,1);
	TEST_ASSERT_EQUAL_UINT(pos8_to_ui16(p), 257);

	TEST_ASSERT_FALSE(pos8_to_ui16(pos8(1,10)) == pos8_to_ui16(pos8(34,5)));
}

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

void print_reg32(reg_t *sa)
{
    return;
    putchar('\n');
    printf("IND: ");
    for (uint32_t i = 0; i < sa->id_loc_count; i++) {
        printf("%u=", i);
    }
    putchar('\n');
    printf("LOC: ");
    for (uint32_t i = 0; i < sa->id_loc_count; i++) {
        printf("%u:", sa->data_loc[i]);
    }
    putchar('\n');
    printf("IDS: ");
    for (uint32_t i = 0; i < sa->id_loc_count; i++) {
        printf("%u.", sa->data_id[i]);
    }
    putchar('\n');
    printf("DAT: ");
    for (uint32_t i = 0; i < sa->count; i++) {
        printf("%u-", ((uint32_t *)sa->data)[i]);
    }
    putchar('\n');
}

void smarray_baba()
{
    reg_t *sa = reg_create(sizeof(uint32_t), 5);

    for (uint32_t i = 0; i < 10; i++)
    {
        reg_add(sa, &i);            // ID = 0 -> val = 0
        print_reg32(sa);
    }

    print_reg32(sa);

    reg_rem(sa, 8);
    print_reg32(sa);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 9), 9);
    reg_rem(sa, 2);
    print_reg32(sa);
    reg_rem(sa, 1);

    print_reg32(sa);

    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 0), 0);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 3), 3);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 4), 4);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 5), 5);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 6), 6);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 7), 7);
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 9), 9);

    return; // All remaining tests should generate errors
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 1), 1);     // Deleted
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 2), 2);     // Deleted
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 8), 8);     // Deleted
    TEST_ASSERT_EQUAL_UINT(*(uint32_t *)reg_get(sa, 10), 10);   // Overflow
}

void smarray_shenanigans()
{
    reg_t *sa = reg_create(sizeof(uint32_t), 5);
    uint32_t id;

    // Add one million elements
    for (uint32_t i = 0; i < 1e6; i++)
    {
        id = reg_add(sa, &i);
        TEST_ASSERT_EQUAL_INT(id, i);
    }

    // Remove 10 000 random elements
    uint32_t data_num_to_remove;
    for (uint32_t i = 0; i < 1e3; i++)
    {
        data_num_to_remove = GetRandomValue(0, 1e5);
        reg_iter_rem(sa, data_num_to_remove);
    }

    uint32_t *val;
    for (uint32_t i = 0; i < sa->count; i++)
    {
        val = (uint32_t *)reg_iter_get(sa, i);
        id = sa->data_id[i];
        TEST_ASSERT_EQUAL_INT(id, *val);
    }
}

void memory_grid()
{
    uint16_t **g = (uint16_t **)allocate_memory_grid(10, 10, sizeof(uint16_t));
    uint16_t k = 0;
    for (uint32_t i = 0; i < 10; i++)
    {
        for (uint32_t j = 0; j < 10; j++)
        {
            g[i][j] = k;
            k++;
        }
    }
    TEST_ASSERT_EQUAL_INT(0, g[0][0]);
    TEST_ASSERT_EQUAL_INT(10, g[1][0]);
    TEST_ASSERT_EQUAL_INT(99, g[9][9]);
    free(g);
}
void memory_grid_2()
{
    uint16_t **g_2 = (uint16_t **)allocate_memory_grid(10, 100, sizeof(uint16_t));
    uint16_t k = 0;
    for (uint32_t i = 0; i < 10; i++)
    {
        for (uint32_t j = 0; j < 100; j++)
        {
            g_2[i][j] = k;
            k++;
        }
    }

    TEST_ASSERT_EQUAL_INT(0, g_2[0][0]);
    TEST_ASSERT_EQUAL_INT(10, g_2[0][10]);
    TEST_ASSERT_EQUAL_INT(99, g_2[0][99]);
    TEST_ASSERT_EQUAL_INT(100, g_2[1][0]);
    TEST_ASSERT_EQUAL_INT(199, g_2[1][99]);
    TEST_ASSERT_EQUAL_INT(599, g_2[5][99]);
    TEST_ASSERT_EQUAL_INT(999, g_2[9][99]);
    free(g_2);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(str_initialization);
    RUN_TEST(str_writing);
    RUN_TEST(pos_lists);
    RUN_TEST(smarray_shenanigans);
    RUN_TEST(smarray_baba);
    RUN_TEST(memory_grid);
    RUN_TEST(memory_grid_2);
    return UNITY_END();
}
