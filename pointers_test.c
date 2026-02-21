#include <stdio.h>

int main() {
    int sensor_data = 25;
    int* p_data = &sensor_data;

    printf("Data value: %d\n", sensor_data);
    printf("Memory address of data: %p\n", (void*)p_data);

    *p_data = 50;
    printf("New data value: %d\n", sensor_data);

    return 0;
}