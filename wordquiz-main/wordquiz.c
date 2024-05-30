#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

typedef enum {
    C_ZERO,
    C_LIST,
    C_SHOW,
    C_TEST,
    C_RETEST,
    C_EXIT,
} command_t;

char* read_a_line(FILE* fp) {
    static char buf[BUFSIZ];
    static int buf_n = 0;
    static int curr = 0;

    if (feof(fp) && curr == buf_n - 1)
        return 0x0;

    char* s = 0x0;
    size_t s_len = 0;
    do {
        int end = curr;
        while (!(end >= buf_n || !iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end && s != 0x0) {
            curr = end;
            break;
        }
        curr = end;
        while (!(end >= buf_n || iscntrl(buf[end]))) {
            end++;
        }
        if (curr < end) {
            if (s == 0x0) {
                s = strndup(buf + curr, end - curr);
                s_len = end - curr;
            } else {
                s = realloc(s, s_len + end - curr + 1);
                s = strncat(s, buf + curr, end - curr);
                s_len = s_len + end - curr;
            }
        }
        if (end < buf_n) {
            curr = end + 1;
            break;
        }

        buf_n = fread(buf, 1, sizeof(buf), fp);
        curr = 0;
    } while (buf_n > 0);
    return s;
}

void print_menu() {
    printf("1. List all wordbooks\n");
    printf("2. Show the words in a wordbook\n");
    printf("3. Test with a wordbook\n");
    printf("4. Retest incorrect problems\n");
    printf("5. Exit\n");
}

int get_command() {
    int cmd;

    printf(">");
    scanf("%d", &cmd);
    return cmd;
}

void list_wordbooks() {
    DIR* d = opendir("wordbooks");

    printf("\n  ----\n");

    struct dirent* wb;
    while ((wb = readdir(d)) != NULL) {
        if (strcmp(wb->d_name, ".") != 0 && strcmp(wb->d_name, "..") != 0) {
            printf("  %s\n", wb->d_name);
        }
    }
    closedir(d);

    printf("  ----\n");
}

void show_words() {
    char wordbook[128];
    char filepath[256];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n");
    printf(">");
    scanf("%s", wordbook);
/*when printing about getting the type of the wordbook, add available type of the word book. additionally add the example of typying method of word book.*/

    sprintf(filepath, "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "r");

    printf("\n  -----\n");
    char* line;
    while ((line = read_a_line(fp)) != NULL) {
        char* word = strtok(line, "\"");
        strtok(NULL, "\"");
        char* meaning = strtok(NULL, "\"");

        printf("  %s : %s\n", word, meaning);

        free(line);
    }
    printf("  -----\n\n");

    fclose(fp);
}

/* Add incorrect problem saving and retesting capabilities:
   After taking the quiz, add the ability to save the wrong questions and rewatch only the wrong questions. */
void save_incorrect(char* wordbook, char* word, char* meaning) {
    char filepath[256];
    sprintf(filepath, "wordbooks/%s_incorrect.txt", wordbook);

    FILE* fp = fopen(filepath, "a");
    fprintf(fp, "\"%s\" : \"%s\"\n", word, meaning);
    fclose(fp);
}

void run_test(char* wordbook) {
    char filepath[256];
    sprintf(filepath, "wordbooks/%s", wordbook);

    FILE* fp = fopen(filepath, "r");

    printf("\n-----\n");

    int n_questions = 0;
    int n_correct = 0;
    int hint_count = 3;  // Limit the number of hints provided


    char* line;
    while ((line = read_a_line(fp)) != NULL) {
        char* word = strtok(line, "\"");
        strtok(NULL, "\"");
        char* meaning = strtok(NULL, "\"");

        char correct_word[128];
        strcpy(correct_word, word);

        while (1) {
            printf("Q. %s\n", meaning);
            printf("?  ");

            char answer[128];
            scanf("%s", answer);

            if (strcmp(answer, "hint") == 0) {
                if (hint_count > 0) {
                    hint_count--;
                    printf("Hint: %.*s\n", (int)(strlen(correct_word) / 2), correct_word);  // Provide half of the word as a hint
                } else {
                    printf("No more hints available.\n");
                }
            } else if (strcmp(answer, correct_word) == 0) {
                printf("- correct\n");
                n_correct++;
                break;
            } else {
                printf("- wrong: %s\n", correct_word);
                save_incorrect(wordbook, correct_word, meaning);
                break;
            }
        }
        n_questions++;
        free(line);
    }

    printf("(%d/%d)\n", n_correct, n_questions);

    printf("-----\n\n");

    fclose(fp);
}

void retest_incorrect() {
    char wordbook[128];
    char filepath[256];

    list_wordbooks();

    printf("Type in the name of the wordbook?\n");
    printf(">");
    scanf("%s", wordbook);

    sprintf(filepath, "wordbooks/%s_incorrect.txt", wordbook);

    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        printf("No incorrect problems found for this wordbook.\n");
        return;
    }

    printf("\n-----\n");

    int n_questions = 0;
    int n_correct = 0;
    int hint_count = 3;  // Limit the number of hints provided

    char* line;
    while ((line = read_a_line(fp)) != NULL) {
        char* word = strtok(line, "\"");
        strtok(NULL, "\"");
        char* meaning = strtok(NULL, "\"");

        char correct_word[128];
        strcpy(correct_word, word);

        while (1) {
            printf("Q. %s\n", meaning);
            printf("?  ");

            char answer[128];
            scanf("%s", answer);

            if (strcmp(answer, "hint") == 0) {
                if (hint_count > 0) {
                    hint_count--;
                    printf("Hint: %.*s\n", (int)(strlen(correct_word) / 2), correct_word); // Provide half of the word as a hint
                } else {
                    printf("No more hints available!!!\n");
                }
            } else if (strcmp(answer, correct_word) == 0) {
                printf("- correct\n");
                n_correct++;
                break;
            } else {
                printf("- wrong: %s\n", correct_word);
                break;
            }
        }
        n_questions++;
        free(line);
    }

    printf("(%d/%d)\n", n_correct, n_questions);

    printf("-----\n\n");

    fclose(fp);
}

int main() {
    printf(" *** Word Quiz *** \n\n");

    int cmd;
    do {
        print_menu();

        cmd = get_command();
        switch (cmd) {
            case C_LIST: {
                list_wordbooks();
                break;
            }

            case C_SHOW: {
                show_words();
                break;
            }

            case C_TEST: {
                char wordbook[128];
                list_wordbooks();
                printf("Type in the name of the wordbook?\n");
                printf(">");
                scanf("%s", wordbook);
                run_test(wordbook);
                break;
            }

            case C_RETEST: {
                retest_incorrect();
                break;
            }

            case C_EXIT: {
                return EXIT_SUCCESS;
            }
        }
    } while (cmd != C_EXIT);

    return EXIT_SUCCESS;
}
