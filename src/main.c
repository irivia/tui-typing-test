#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <termios.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define CTRL_W 23
#define DEL 127

typedef struct {
    char *data;
    size_t size;
    size_t pos;
} Buffer;

Buffer buffer_new(size_t buffer_size)
{
    enum { arena_size = 1024 };
    static char arena[arena_size];
    static size_t arena_ptr = 0;

    const size_t new_arena_ptr = arena_ptr + buffer_size;

    if (new_arena_ptr > arena_size) {
        fprintf(stderr, "Arena(size: %d, ptr: %zu) can't fit Buffer(size: %zu)\n",
                arena_size, arena_ptr, buffer_size);
        exit(1);
    }

    Buffer buffer = {
        .data = &arena[arena_ptr],
        .size = buffer_size,
    };

    arena_ptr = new_arena_ptr;

    return buffer;
}

void buffer_write(Buffer *buffer, char *data, size_t sz)
{
    assert(buffer != NULL && "Buffer is NULL");
    assert(data != NULL && "Data is NULL");
    
    if (buffer->pos + sz > buffer->size) {
        fprintf(stderr, "Data of size: %zu exceeds buffer of size: %zu and pos: %zu\n",
                sz, buffer->size, buffer->pos);
        exit(1);
    }

    memcpy(&buffer->data[buffer->pos], data, sz);
    buffer->pos += sz;
}

void buffer_putchar(Buffer *buffer, char c)
{
    assert(buffer != NULL && "Buffer is NULL");
    if (buffer->pos + 1 > buffer->size) {
        fprintf(stderr, "Data of size: 1 exceeds buffer of size: %zu and pos: %zu\n",
                buffer->size, buffer->pos);
        exit(1);
    }

    buffer->data[buffer->pos++] = c;
}

void increment_wrong_counter(uint8_t *accuracy_map, size_t len, int i)
{
    assert(i >= 0 && i < len && "Index is out of bounds");
    accuracy_map[i] >>= 1;
    accuracy_map[i] += 1;
    accuracy_map[i] <<= 1;
}

void decrement_wrong_counter(uint8_t *accuracy_map, size_t len, int i)
{
    assert(i >= 0 && i < len && "Index is out of bounds");
    accuracy_map[i] >>= 1;
    accuracy_map[i] -= 1;
    accuracy_map[i] <<= 1;
}

int get_wrong_counter(uint8_t *accuracy_map, size_t len, int i)
{
    assert(i >= 0 && i < len && "Index is out of bounds");
    return accuracy_map[i] >> 1;
}

void build_test_buffer(Buffer *test_buffer, char *test, size_t test_len, uint8_t *accuracy_map)
{
    test_buffer->pos = 0;
    buffer_putchar(test_buffer, '\r');
    for (size_t i = 0; i < test_len; i++) {
        int n = get_wrong_counter(accuracy_map, test_len, i);
        if (n > 0) {
            for (uint8_t j = 0; j <= n; j++) {
                buffer_putchar(test_buffer, ' ');
            }
        }
        else {
            buffer_putchar(test_buffer, test[i]);
        }
    }
    for (size_t i = test_buffer->pos; i < test_buffer->size; i++) {
        buffer_putchar(test_buffer, ' ');
    }
}

int main()
{
    char test[] = "the quick brown fox jumps over the lazy dog";
    const size_t test_len = sizeof(test);
    uint8_t accuracy_map[test_len];
    for (size_t i = 0; i < test_len; i++) {
        accuracy_map[i] = 0;
    }
    Buffer test_buffer = buffer_new(test_len * 4);
    Buffer typed_buffer = buffer_new(test_len * 4);
    buffer_putchar(&typed_buffer, '\r');

    int test_pos = 0;
    int correct_chars = 0;
    int wrong_chars = 0;
    int extra_chars = 0;

    time_t start_time, end_time;
    struct termios old_termios, termios;

    tcgetattr(STDIN_FILENO, &old_termios);
    termios = old_termios;

    /* disable echo */
    termios.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);

    bool started = false;
    while (test_pos < test_len) {
        build_test_buffer(&test_buffer, test, test_len, accuracy_map);
        write(STDIN_FILENO, test_buffer.data, test_buffer.pos);
        write(STDIN_FILENO, typed_buffer.data, typed_buffer.pos);
        char c = getchar();
        if (!started) {
            start_time = time(0);
            started = true;
        }
        if (c == DEL || c == CTRL_W) {
            do {
                if (test_pos <= 0 || typed_buffer.pos <= 1) break;
                typed_buffer.pos -= 1;
                if (get_wrong_counter(accuracy_map, test_len, test_pos) > 0) {
                    decrement_wrong_counter(accuracy_map, test_len, test_pos);
                }
                else {
                    test_pos -= 1;
                }
            } while (c == CTRL_W && test[test_pos - 1] != ' ');
            continue;
        }
        if (c == test[test_pos]) {
            buffer_putchar(&typed_buffer, c);
            accuracy_map[test_pos] |= true;
            correct_chars++;
        }
        else if (test_pos < test_len - 1) {
            buffer_putchar(&typed_buffer, 'X');
            if (test[test_pos] != ' ') {
                accuracy_map[test_pos] = false;
                wrong_chars++;
            }
            else {
                increment_wrong_counter(accuracy_map, test_len, test_pos);
                test_pos -= 1;
                extra_chars++;
            }
        }
        test_pos++;
    }
    end_time = time(0);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

    const int total_chars = correct_chars + wrong_chars + extra_chars;
    const time_t seconds = end_time - start_time;
    const double speed = (correct_chars / 5.f) * (60.f/seconds);
    const double accuracy = 100.f/total_chars * correct_chars;

    printf("\n\n");
    for (size_t i = 0; i < test_len; i++) {
        char c = test[i];
        if (accuracy_map[i] & 1) {
            printf("\e[1;32;40m%c", c);
        }
        else {
            if (c == ' ')
                printf("\e[7;31;40m%c", c);
            else
                printf("\e[1;31;40m%c", c);
        }
    }
    printf("\e[0m\n------------------------------------\n");
    printf("                 WPM: %0.3lf\n", speed);
    printf("            Accuracy: %0.3lf%%\n", accuracy);
    printf("       Total seconds: %ld\n", seconds);
    printf("    Characters typed: %d\n", total_chars);
    printf("  Correct characters: %d\n", correct_chars);
    printf("    Wrong characters: %d\n", wrong_chars); 
    printf("    Extra characters: %d\n", extra_chars); 


    return 0;
}
