#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <termios.h>
#include <assert.h>
#include <string.h>

#define CTRL_W 23

typedef struct {
    char *data;
    size_t capacity;
    size_t pos;
} Buffer;

void write_to_buffer(Buffer *buffer, char *data, size_t sz)
{
    assert(buffer != NULL && "Buffer is NULL");
    assert(data != NULL && "Data is NULL");
    
    if (buffer->pos + sz > buffer->capacity) {
        fprintf(stderr, "Data of size: %zu exceeds buffer of size: %zu and pos: %zu\n",
                sz, buffer->capacity, buffer->pos);
        exit(1);
    }

    memcpy(&buffer->data[buffer->pos], data, sz);
    buffer->pos += sz;
}

void putchar_to_buffer(Buffer *buffer, char c)
{
    assert(buffer != NULL && "Buffer is NULL");
    if (buffer->pos + 1 > buffer->capacity) {
        fprintf(stderr, "Data of size: 1 exceeds buffer of size: %zu and pos: %zu\n",
                buffer->capacity, buffer->pos);
        exit(1);
    }

    buffer->data[buffer->pos++] = c;
}

int main()
{
    char test[] = "the quick brown fox jumps over the lazy dog";
    const size_t test_len = sizeof(test);
    bool accuracy_map[test_len];
    char data_buffer[256];
    Buffer buffer = {
        .data = data_buffer,
        .capacity = sizeof(data_buffer),
        .pos = 0
    };
    write_to_buffer(&buffer, test, test_len);
    putchar_to_buffer(&buffer, '\r');
    int test_pos = 0;
    int correct_chars = 0;
    int wrong_chars = 0;
    int extra_chars = 0;
    bool started = false;
    time_t start_time;
    time_t end_time;
    struct termios old_termios, termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    termios = old_termios;
    char input_buf[128];
    const size_t input_buf_sz = sizeof(input_buf);
    size_t input_buf_i = 0;

    /* disable echo */
    termios.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);

    while (test_pos < test_len) {
        write(STDIN_FILENO, buffer.data, buffer.pos);
        buffer.pos = 0;
        char c = getchar();
        if (!started) {
            start_time = time(0);
            started = true;
        }
        if (c == 127) {
            if (test_pos > 0) {
                test_pos -= 1;
                putchar_to_buffer(&buffer, '\b');
                putchar_to_buffer(&buffer, test[test_pos]);
                putchar_to_buffer(&buffer, '\b');
            }
            continue;
        }
        if (c == CTRL_W) {
            while (test_pos > 0 && test[test_pos - 1] != ' ') {
                test_pos -= 1;
                putchar_to_buffer(&buffer, '\b');
                putchar_to_buffer(&buffer, test[test_pos]);
                putchar_to_buffer(&buffer, '\b');
            }
            continue;
        }
        if (c == test[test_pos]) {
            putchar_to_buffer(&buffer, test[test_pos]);
            accuracy_map[test_pos] = true;
            correct_chars++;
        }
        else if (test_pos < test_len - 1) {
            if (test[test_pos] != ' ') {
                putchar_to_buffer(&buffer, 'X');
                accuracy_map[test_pos] = false;
                wrong_chars++;
            }
            else {
                putchar_to_buffer(&buffer, 'X');
                int len = &test[test_len] - &test[test_pos + 1];
                write_to_buffer(&buffer, &test[test_pos], len);
                for (int i = len; i > 0; i--) {
                    putchar_to_buffer(&buffer, '\b');
                }
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
        if (accuracy_map[i]) {
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
