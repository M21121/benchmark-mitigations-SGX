// app/ocall_handlers.cpp
#include "enclave_u.h"
#include <cstdio>
#include <cstdlib>

void empty_ocall() {
    volatile int counter = 0;
    for (int i = 0; i < 100; ++i) {
        counter += i % 123;
    }
}

void ocall_print_string(const char* str) {
    printf("%s", str);
}

void pong_ocall(int iteration) {
    volatile int counter = 0;
    for (int i = 0; i < 100; ++i) {
        counter += i % 123;
    }
    (void)iteration;
}

size_t ocall_read_file(const char* filename, char* buf, size_t buf_len) {
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;
    size_t bytes_read = fread(buf, 1, buf_len, file);
    fclose(file);
    return bytes_read;
}

size_t ocall_read_sealed_file(const char* filename, uint8_t* sealed_buf, size_t buf_len) {
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;

    size_t bytes_read = fread(sealed_buf, 1, buf_len, file);
    fclose(file);
    return bytes_read;
}

int ocall_write_sealed_file(const char* filename, const uint8_t* sealed_data, size_t data_len) {
    FILE* file = fopen(filename, "wb");
    if (!file) return -1;

    size_t written = fwrite(sealed_data, 1, data_len, file);
    fclose(file);
    return (written == data_len) ? 0 : -1;
}
