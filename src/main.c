#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <termios.h>

#define CTRL_W 23

int main()
{
    char test[] = "the quick brown fox jumps over the lazy dog";
    const size_t test_len = sizeof(test);
    bool accuracy_map[test_len];
    write(STDIN_FILENO, test, test_len);
    write(STDIN_FILENO, "\r", 1);
    int cursor = 0;
    int typed = 0;
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
        if (c == CTRL_W) {
            while (cursor > 0 && test[cursor - 1] != ' ') {
                cursor -= 1;
                write(STDIN_FILENO, "\b", 1);
                write(STDIN_FILENO, &test[cursor], 1);
                write(STDIN_FILENO, "\b", 1);
            }
            continue;
        }
        typed++;
        if (c == test[cursor]) {
            write(STDIN_FILENO, &test[cursor], 1);
            accuracy_map[cursor] = true;
            correct++;
        }
        else if (cursor < test_len - 1) {
            if (test[cursor] != ' ') {
                write(STDIN_FILENO, "X", 1);
                accuracy_map[cursor] = false;
            }
            else {
                write(STDIN_FILENO, "X", 1);
                int len = &test[test_len] - &test[cursor + 1];
                write(STDIN_FILENO, &test[cursor], len);
                for (int i = len; i > 0; i--) {
                    write(STDIN_FILENO, "\b", 1);
                }
                cursor -= 1;
            }
        }
        cursor++;
    }
    end_time = time(0);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

    const time_t seconds = end_time - start_time;
    const double speed = (correct / 5.f) * (60.f/seconds);
    const double accuracy = 100.f/typed * correct;

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
    printf("\n------------------------------------\n");
    printf("               WPM: %0.3lf\n", speed);
    printf("          Accuracy: %0.3lf%%\n", accuracy);
    printf("     Total seconds: %ld\n", seconds);
    printf("  Characters typed: %d\n", typed);
    printf("  Valid characters: %d\n", correct);
    printf("Invalid characters: %d\n", typed - correct); 


    return 0;
}
