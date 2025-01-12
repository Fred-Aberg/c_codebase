#include "common.h"
#include "stdio.h"

void print_array(int *arr){
    printf("[LEN/CAP]: [%ld, %ld]", array_capacity(arr), array_length(arr));
    for (size_t i = 0; i < array_length(arr); i++)
    {
        printf("\n%ld: %ld", i, arr[i]);
    }
}

int main(){
    Allocator_t int_allocator = create_allocator(malloc, free, NULL);
    int *array = array(int, int_allocator);

    print_array(array);

    array_append(array, 1);
    array_append(array, 2);
    array_append(array, 3);
    array_append(array, 4);

    print_array(array);

    array_append(array, 5);
    array_append(array, 6);
    array_append(array, 7);

    print_array(array);

    array_remove(array, 2);

    print_array(array);

    return 1;
}