#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <termios.h>

int main()
{
    char test[] = "the quick brown fox jumps over the lazy dog";
    const size_t test_len = sizeof(test);
    bool accuracy_map[test_len];
    write(STDIN_FILENO, test, test_len);
    write(STDIN_FILENO, "\r", 1);
    int cursor = 0;
    int correct = 0;
    bool started = false;
    time_t start_time;
    time_t end_time;
    struct termios old_termios, termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    termios = old_termios;

    /* disable echo */
    termios.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios);
    
    while (cursor < test_len) {
        char c = getchar();
        if (!started) {
            start_time = time(0);
            started = true;
        }
        if (c == 127) {
            if (cursor > 0) {
                cursor -= 1;
                write(STDIN_FILENO, "\b", 1);
                write(STDIN_FILENO, &test[cursor], 1);
                write(STDIN_FILENO, "\b", 1);
            }
            continue;
        }
        if (c == test[cursor]) {
            write(STDIN_FILENO, &test[cursor], 1);
            accuracy_map[cursor] = true;
            correct++;
        }
        else {
            if (cursor < test_len - 1) {
                write(STDIN_FILENO, "X", 1);
                accuracy_map[cursor] = false;
            }
        }
        cursor++;
    }
    end_time = time(0);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

    time_t seconds = end_time - start_time;
    double speed = (correct / 5.f) * (60.f/seconds);


    printf("\n\n");
    for (size_t i = 0; i < test_len; i++) {
        char c = test[i];
        if (accuracy_map[i]) {
            printf("\e[1;32;40m%c\e[0m", c);
        }
        else {
            if (c == ' ')
                printf("\e[7;31;40m%c\e[0m", c);
            else
                printf("\e[1;31;40m%c\e[0m", c);
        }
    }
    printf("\n%0.3lf WPM\n", speed);

    return 0;
}
