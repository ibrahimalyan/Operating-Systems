#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define BUFFER_SIZE 1280
#define MAX_POLYNOMIALS 10


struct Polynomial {
    int degree;
    int* coefficients;
};

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};


void tokenizeWithoutSpaces(const char* input, char* tokens[]) {
    int tokenIndex = 0;
    int startIndex = 0;
    int endIndex = 0;
    int insideParentheses = 0;

    for (int i = 0; i < strlen(input); i++) {
        if (input[i] == '(') {
            insideParentheses = 1;
            startIndex = i;
        } else if (input[i] == ')') {
            insideParentheses = 0;
            endIndex = i;
            tokens[tokenIndex] = (char*)malloc((endIndex - startIndex + 2) * sizeof(char));
            strncpy(tokens[tokenIndex], input + startIndex, endIndex - startIndex + 1);
            tokens[tokenIndex][endIndex - startIndex + 1] = '\0';
            tokenIndex++;
        } else if (!insideParentheses && input[i] != ' ') {
            startIndex = i;
            while (input[i] != '(' && input[i] != ')' && input[i] != ' ' && i < strlen(input)) {
                i++;
            }
            endIndex = i - 1;
            tokens[tokenIndex] = (char*)malloc((endIndex - startIndex + 2) * sizeof(char));
            strncpy(tokens[tokenIndex], input + startIndex, endIndex - startIndex + 1);
            tokens[tokenIndex][endIndex - startIndex + 1] = '\0';
            tokenIndex++;
            i--;
        }
    }
}



void parsePolynomial(char* str, struct Polynomial* poly) {
    // Find the degree
    char* degreeStr = strchr(str, '(') +1;
    sscanf(degreeStr, "%d", &(poly->degree));
    // Allocate memory for coefficients array
    poly->coefficients = (int*)calloc(poly->degree + 1, sizeof(int));

    // Find the coefficients
    char* coefficientStr = strchr(str, ':') + 1;
    char* token;
    int i = 0;

    token = strtok(coefficientStr, ",)");
    while (token != NULL) {
        sscanf(token, "%d", &(poly->coefficients[i]));

        i++;
        token = strtok(NULL, ",)");

    }

}

void printPolynomial(struct Polynomial poly) {
    if (poly.degree == 0) {
        printf("%d\n", poly.coefficients[0]);
        return;
    }

    printf("%dx^%d", poly.coefficients[0], poly.degree);

    for (int i = 1; i <= poly.degree; i++) {
        if (poly.coefficients[i] == 0) {
            continue;
        } else if (poly.coefficients[i] > 0) {
            printf(" + ");
        } else {
            printf(" - ");
        }

        if (i == poly.degree) {
            printf("%d", abs(poly.coefficients[i]));
        } else if (i == poly.degree - 1) {
            if (abs(poly.coefficients[i]) == 1) {
                printf("x");
            } else {
                printf("%dx", abs(poly.coefficients[i]));
            }
        } else {
            if (abs(poly.coefficients[i]) == 1) {
                printf("x^%d", poly.degree - i);
            } else {
                printf("%dx^%d", abs(poly.coefficients[i]), poly.degree - i);
            }
        }
    }

    printf("\n");
}


void performOperation(char* operation, struct Polynomial poly1, struct Polynomial poly2) {
    struct Polynomial result;
    int i, j;

    if (strcmp(operation, "ADD") == 0) {
        // Addition
        int maxDegree;
        if (poly1.degree > poly2.degree) {
            maxDegree = poly1.degree;
        } else {
            maxDegree = poly2.degree;
        }
        result.degree = maxDegree;
        result.coefficients = (int*)calloc((result.degree + 1), sizeof(int));

        int start1 = result.degree - poly1.degree;
        int start2 = result.degree - poly2.degree;

        for (i = start1; i <= result.degree; i++) {
            result.coefficients[i] += poly1.coefficients[i - start1];
        }

        for (i = start2; i <= result.degree; i++) {
            result.coefficients[i] += poly2.coefficients[i - start2];
        }
    } else if (strcmp(operation, "SUB") == 0) {
        // Subtraction
        int maxDegree;
        if (poly1.degree > poly2.degree) {
            maxDegree = poly1.degree;
        } else {
            maxDegree = poly2.degree;
        }        result.degree = maxDegree;
        result.coefficients = (int*)calloc((result.degree + 1), sizeof(int));

        int start1 = result.degree - poly1.degree;
        int start2 = result.degree - poly2.degree;

        for (i = start1; i <= result.degree; i++) {
            result.coefficients[i] += poly1.coefficients[i - start1];
        }

        for (i = start2; i <= result.degree; i++) {
            result.coefficients[i] -= poly2.coefficients[i - start2];
        }
    } else if (strcmp(operation, "MUL") == 0) {
        // Multiplication
        result.degree = poly1.degree + poly2.degree;
        result.coefficients = (int*)calloc((result.degree + 1), sizeof(int));

        for (i = 0; i <= poly1.degree; i++) {
            for (j = 0; j <= poly2.degree; j++) {
                result.coefficients[i + j] += poly1.coefficients[i] * poly2.coefficients[j];
            }
        }
    } else {
        printf("Invalid operation: %s\n", operation);
        return;
    }



    // Print the adjusted polynomial
    printPolynomial(result);

    // Free dynamically allocated memory
    free(result.coefficients);
}


int main() {
    key_t key = ftok(".", 'S');  // Generate a unique key

    // Create shared memory segment
    int shmid = shmget(key, BUFFER_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }




    // Attach shared memory segment
    struct Polynomial *buffer = (struct Polynomial *) shmat(shmid, NULL, 0);
    if (buffer == (struct Polynomial *) (-1)) {
        perror("shmat");
        exit(1);
    }

    // Create semaphore for empty buffer slots
    int semid_empty = semget(key, 1, IPC_CREAT | 0666);
    if (semid_empty == -1) {
        perror("semget");
        exit(1);
    }



    // Initialize semaphore for empty buffer slots to the maximum size
    union semun sem_arg;
    sem_arg.val = MAX_POLYNOMIALS;
    if (semctl(semid_empty, 0, SETVAL, sem_arg) == -1) {
        perror("semctl");
        exit(1);
    }

    // Create semaphore for full buffer slots
    int semid_full = semget(key, 1, IPC_CREAT | 0666);
    if (semid_full == -1) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphore for full buffer slots to 0 (no polynomials initially)
    sem_arg.val = 0;
    if (semctl(semid_full, 0, SETVAL, sem_arg) == -1) {
        perror("semctl");
        exit(1);
    }


    while (1) {
        char input[128];

        fgets(input, sizeof(input), stdin);

        if (strcmp(input, "END\n") == 0) {
            break;
        }

        struct Polynomial polynomial1;
        struct Polynomial polynomial2;

        // Parse the input
        char *tokens[3];
        tokenizeWithoutSpaces(input, tokens);


        parsePolynomial(tokens[0], &polynomial1);
        parsePolynomial(tokens[2], &polynomial2);

// Wait for an empty buffer slot
        struct sembuf sem_op_wait_empty = {0, -1, 0};  // Wait on semaphore 0 (empty buffer)
        if (semop(semid_empty, &sem_op_wait_empty, 1) == -1) {
            perror("semop");
            exit(EXIT_FAILURE);
        }
        // Write polynomials to the shared memory buffer
        int index;
        for (index = 0; index < MAX_POLYNOMIALS; index++) {
            if (buffer[index].degree == -1) {
                break;
            }
        }

        if (index == MAX_POLYNOMIALS) {
            printf("Buffer is full. Cannot write polynomial.\n");
        } else {
            buffer[index] = polynomial1;
            buffer[index + MAX_POLYNOMIALS] = polynomial2;

            // Signal a full buffer slot
            struct sembuf sem_op_signal_full = {0, 1, 0};  // Signal semaphore 0 (full buffer)
            if (semop(semid_full, &sem_op_signal_full, 1) == -1) {
                perror("semop");
                exit(1);
            }
        }
    }

// Detach shared memory segment
    if (shmdt(buffer) == -1) {
        perror("shmdt");
        exit(1);
    }

// Delete shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

// Delete semaphores
    if (semctl(semid_empty, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(1);
    }

    if (semctl(semid_full, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(1);
    }

    return 0;
}