#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_RESET "\x1b[0m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RED "\x1b[31m"

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

typedef struct {
    char question[256];
    char options[4][100];
    int correctOption;
} Question;

typedef struct {
    char name[50];
    float score;
} LeaderboardEntry;

void initializeLeaderboard() {
    FILE *file = fopen("leaderboard.txt", "r");
    if (!file) {  
        file = fopen("leaderboard.txt", "w");
        if (file) {
            fprintf(file, "Ale 5.0\nAnto 4.0\nSara 3.5\n");
            fclose(file);
        }
    } else {
        fclose(file);
    }
}

void loadQuestions(Question questions[]); 
void useFiftyFifty(Question q, int *remainingOptions);

void readUserName(char *name) {
    printf(COLOR_CYAN "Enter your name: " COLOR_RESET);
    scanf("%s", name);
}

int askQuestion(Question q, int qNumber, float *score) {
    int choice, use5050;
    int remainingOptions[4] = {1, 1, 1, 1};

    printf("\n" COLOR_YELLOW "Question %d: %s\n" COLOR_RESET, qNumber, q.question);
    for (int j = 0; j < 4; j++) {
        if (remainingOptions[j])
            printf("%d. %s\n", j + 1, q.options[j]);
    }

    printf(COLOR_CYAN "Use 50/50? (1 = Yes, 0 = No): " COLOR_RESET);
    scanf("%d", &use5050);
    while (use5050 != 0 && use5050 != 1) {
        printf(COLOR_RED "Invalid input. Use 50/50? (1 = Yes, 0 = No): " COLOR_RESET);
        scanf("%d", &use5050);
    }

    if (use5050) {
        useFiftyFifty(q, remainingOptions);
        printf("\nAfter 50/50 lifeline:\n");
        for (int j = 0; j < 4; j++) {
            if (remainingOptions[j])
                printf("%d. %s\n", j + 1, q.options[j]);
        }
    }

    printf(COLOR_CYAN "Your answer (1-4): " COLOR_RESET);
    scanf("%d", &choice);
    while (choice < 1 || choice > 4 || !remainingOptions[choice - 1]) {
        printf(COLOR_RED "Invalid choice. Please choose a valid option (1-4): " COLOR_RESET);
        scanf("%d", &choice);
    }

    if (choice - 1 == q.correctOption) {
        printf(COLOR_GREEN "Correct!\n" COLOR_RESET);
        *score += use5050 ? 0.5 : 1;
        return 1;  // Continue playing
    } else {
        printf(COLOR_RED "Wrong! The correct answer was: %s\n" COLOR_RESET, q.options[q.correctOption]);
        return 0;  // End game
    }
}


void startGame() {
    clearScreen();
    printf("▄▀▄▀▄ " COLOR_GREEN "START GAME" COLOR_RESET " ▄▀▄▀▄\n");
    Question questions[15];
    loadQuestions(questions);
    float score = 0;
    char name[50];

    readUserName(name);

    for (int i = 0; i < 15; i++) {
        int keepPlaying = askQuestion(questions[i], i + 1, &score);
        if (!keepPlaying) {
            printf(COLOR_RED "\nGame over! You answered incorrectly.\n" COLOR_RESET);
            break;
        }
    }

    FILE *leaderboard = fopen("leaderboard.txt", "a");
    if (leaderboard) {
        fprintf(leaderboard, "%s %.1f\n", name, score);
        fclose(leaderboard);
    }

    FILE *output = fopen("output.txt", "w");
    if (output) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strcspn(timestamp, "\n")] = 0;

        fprintf(output, "˗ˏˋ ★ ˎˊ˗ Game Session Summary ˗ˏˋ ★ ˎˊ˗\n");
        fprintf(output, "Player Name:「 ✦ %s ✦ 」\n", name);
        fprintf(output, "Final Score: %.1f\n", score);
        fprintf(output, "Date & Time: %s\n", timestamp);
        fclose(output);
    }

    printf("\n" COLOR_YELLOW "Your final score: %.1f\n" COLOR_RESET, score);
    printf("Press Enter to return to the main menu...\n");
    getchar();
    getchar();
}

void viewLeaderboard() {
    clearScreen();
    printf("▄▀▄▀▄ " COLOR_GREEN "LEADERBOARD" COLOR_RESET " ▄▀▄▀▄\n");

    FILE *file = fopen("leaderboard.txt", "r");
    if (!file) {
        printf("No scores recorded yet.\n");
    } else {
        LeaderboardEntry entries[100];
        int count = 0;

        
        while (fscanf(file, "%s %f", entries[count].name, &entries[count].score) == 2) {
            count++;
            if (count >= 100) break;  
        }
        fclose(file);

        if (count == 0) {
            printf("No scores recorded yet.\n");
        } else {
            
            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (entries[j].score > entries[i].score) {
                        LeaderboardEntry temp = entries[i];
                        entries[i] = entries[j];
                        entries[j] = temp;
                    }
                }
            }
            file = fopen("leaderboard.txt", "w");
            if (file) {
                for (int i = 0; i < count; i++) {
                    fprintf(file, "%s %.1f\n", entries[i].name, entries[i].score);
                }
                fclose(file);
                printf("\nLeaderboard file has been sorted and saved.\n");
            }

            for (int i = 0; i < count; i++) {
                printf(COLOR_YELLOW "%s" COLOR_RESET " - %.1f points\n", entries[i].name, entries[i].score);
            }
        }
    }

    printf("Press Enter to return to the main menu...\n");
    getchar();
    getchar();
}


void viewGameHistory() {
    clearScreen();
    printf("▄▀▄▀▄ " COLOR_GREEN "GAME HISTORY" COLOR_RESET " ▄▀▄▀▄\n");

    char searchName[50];
    printf(COLOR_CYAN "Enter username: " COLOR_RESET);
    scanf("%s", searchName);

    FILE *file = fopen("leaderboard.txt", "r");
    if (!file) {
        printf("No game history available.\n");
    } else {
        LeaderboardEntry entries[100];
        int count = 0;

        while (fscanf(file, "%s %f", entries[count].name, &entries[count].score) == 2) {
            if (strcmp(entries[count].name, searchName) == 0) {
                count++;
            }
        }
        fclose(file);

        if (count == 0) {
            printf(COLOR_RED "No records found for %s.\n" COLOR_RESET, searchName);
        } else {
            
            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (entries[j].score > entries[i].score) {
                        LeaderboardEntry temp = entries[i];
                        entries[i] = entries[j];
                        entries[j] = temp;
                    }
                }
            }

            for (int i = 0; i < count; i++) {
                printf(COLOR_YELLOW "%s" COLOR_RESET " - %.1f points\n", entries[i].name, entries[i].score);
            }
        }
    }

    printf("Press Enter to return to the main menu...\n");
    getchar();
    getchar();
}

void displayMenu() {
    int choice;
    do {
        clearScreen();
        printf("'*•.¸♡ " COLOR_GREEN "QUIZ GAME MENU" COLOR_RESET " ♡¸.•*'\n");
        printf("1. Start Game\n");
        printf("2. View Leaderboard\n");
        printf("3. View Game History\n");
        printf("4. Exit\n");
        printf(COLOR_CYAN "Enter your choice (1-4): " COLOR_RESET);
        scanf("%d", &choice);
        while (choice < 1 || choice > 4) {
            printf(COLOR_RED "Invalid input. Enter a number between 1 and 4: " COLOR_RESET);
            scanf("%d", &choice);
        }
        switch (choice) {
            case 1:
                startGame();
                break;
            case 2:
                viewLeaderboard();
                break;
            case 3:
                viewGameHistory();
                break;
            case 4:
                clearScreen();
                printf(COLOR_YELLOW "Exiting game...\n" COLOR_RESET);
                exit(0);
        }
    } while (1);
}

int main() {
    initializeLeaderboard();
    displayMenu();
    return 0;
}
void loadQuestions(Question questions[]) {
    strcpy(questions[0].question, "What is the capital of Spain?");
    strcpy(questions[0].options[0], "Berlin");
    strcpy(questions[0].options[1], "Madrid");
    strcpy(questions[0].options[2], "Paris");
    strcpy(questions[0].options[3], "Rome");
    questions[0].correctOption = 1; 

    strcpy(questions[1].question, "Which planet is the closest to the sun?");
    strcpy(questions[1].options[0], "Earth");
    strcpy(questions[1].options[1], "Mercury");
    strcpy(questions[1].options[2], "Jupiter");
    strcpy(questions[1].options[3], "Venus");
    questions[1].correctOption = 1; 

    strcpy(questions[2].question, "Who was the Ancient Greek God of the Sun?");
    strcpy(questions[2].options[0], "Ares");
    strcpy(questions[2].options[1], "Poseidon");
    strcpy(questions[2].options[2], "Apollo");
    strcpy(questions[2].options[3], "Artemis");
    questions[2].correctOption = 2; 
    
    strcpy(questions[3].question, "How many dots appear on a pair of dice?");
    strcpy(questions[3].options[0], "50");
    strcpy(questions[3].options[1], "46");
    strcpy(questions[3].options[2], "38");
    strcpy(questions[3].options[3], "42");
    questions[3].correctOption = 3; 
    
    strcpy(questions[4].question, "What city is known as The Eternal City?");
    strcpy(questions[4].options[0], "Roma");
    strcpy(questions[4].options[1], "Bucharest");
    strcpy(questions[4].options[2], "London");
    strcpy(questions[4].options[3], "Amsterdam");
    questions[4].correctOption = 0; 
    
    strcpy(questions[5].question, "On which continent would you find the world’s largest desert?");
    strcpy(questions[5].options[0], "Africa");
    strcpy(questions[5].options[1], "Australia");
    strcpy(questions[5].options[2], "Antarctica");
    strcpy(questions[5].options[3], "Asia");
    questions[5].correctOption = 2; 
    
    strcpy(questions[6].question, "What is the capital of Ireland?");
    strcpy(questions[6].options[0], "Dublin");
    strcpy(questions[6].options[1], "London");
    strcpy(questions[6].options[2], "Paris");
    strcpy(questions[6].options[3], "Rome");
    questions[6].correctOption = 0;
    
    strcpy(questions[7].question, "How many European capitals does the Danube flow through?");
    strcpy(questions[7].options[0], "5");
    strcpy(questions[7].options[1], "6");
    strcpy(questions[7].options[2], "3");
    strcpy(questions[7].options[3], "4");
    questions[7].correctOption = 3;
    
    strcpy(questions[8].question, "On which continent would you find the city of Baku?");
    strcpy(questions[8].options[0], "Europe");
    strcpy(questions[8].options[1], "Asia");
    strcpy(questions[8].options[2], "America");
    strcpy(questions[8].options[3], "Australia");
    questions[8].correctOption = 1;
    
    strcpy(questions[9].question, "In what year did World War II end?");
    strcpy(questions[9].options[0], "1943");
    strcpy(questions[9].options[1], "1944");
    strcpy(questions[9].options[2], "1945");
    strcpy(questions[9].options[3], "1946");
    questions[9].correctOption = 2;
    
    strcpy(questions[10].question, "Which country has won the most World Cups?");
    strcpy(questions[10].options[0], "Brazil");
    strcpy(questions[10].options[1], "UK");
    strcpy(questions[10].options[2], "France");
    strcpy(questions[10].options[3], "Spain");
    questions[10].correctOption = 0;
    
    strcpy(questions[11].question, "What color are Mickey Mouse's shoes?");
    strcpy(questions[11].options[0], "Red");
    strcpy(questions[11].options[1], "Black");
    strcpy(questions[11].options[2], "Blue");
    strcpy(questions[11].options[3], "Yellow");
    questions[11].correctOption = 3;
    
    strcpy(questions[12].question, "How many colors are in the rainbow?");
    strcpy(questions[12].options[0], "8");
    strcpy(questions[12].options[1], "6");
    strcpy(questions[12].options[2], "7");
    strcpy(questions[12].options[3], "9");
    questions[12].correctOption = 2;
    
    strcpy(questions[13].question, "What is the name of the dog who was one of the first animals in space?");
    strcpy(questions[13].options[0], "Rosco");
    strcpy(questions[13].options[1], "Laika");
    strcpy(questions[13].options[2], "Layla");
    strcpy(questions[13].options[3], "Leo");
    questions[13].correctOption = 1;
    
    strcpy(questions[14].question, "When was the Romanian Revolution?");
    strcpy(questions[14].options[0], "1989");
    strcpy(questions[14].options[1], "1990");
    strcpy(questions[14].options[2], "1998");
    strcpy(questions[14].options[3], "1988");
    questions[14].correctOption = 0;
}

void useFiftyFifty(Question q, int *remainingOptions) {
    int removed = 0;
    for (int i = 0; i < 4; i++) {
        if (i != q.correctOption && removed < 2) {
            remainingOptions[i] = 0;
            removed++;
        }
    }
}
