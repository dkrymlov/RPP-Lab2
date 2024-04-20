#include <iostream>
#include <string>
#include "mpi.h"
#include <chrono>
#include <random>
using namespace std;

void generateRandomString(char* str, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int charset_size = sizeof(charset) - 1;
    srand(time(NULL));

    for (int i = 0; i < length; ++i) {
        str[i] = charset[rand() % charset_size];
    }
}


int findSubstringNonParallel(const char* text, const char* pattern, int textLength, int patternLength) {
    for (int i = 0; i <= textLength - patternLength; ++i) {
        int j;
        for (j = 0; j < patternLength; ++j) {
            if (text[i + j] != pattern[j]) {
                break;
            }
        }
        if (j == patternLength)
            return i;
    }
    return -1;
}


void parallelSearch(const char* text, const char* pattern, int textLength, int patternLength, int& index, int rank, int size) {

    bool flag = false;
    double startTime;
    if (rank == 0)
    {
        startTime = MPI_Wtime();
    }
    {
        int i;
        for (i = rank; i < textLength - patternLength + 1; i += size) {
            int j;
            for (j = 0; j < patternLength; ++j) {
                if (text[i + j] != pattern[j])
                    break;
            }
            if (j == patternLength) {
                {
                    if (index == -1 || index > i) {
                        index = i;
                        flag = true;
                    }
                }
            }
//            fprintf(stdout, "index %d flag %d rank %d\n", index, flag, rank);
//            fflush(stdout);
            if (flag && index <= i) {
                break;
            }
            if (rank == 0)
            {
                for (int num = 1; num < size; num++){
                    MPI_Send(text, textLength, MPI_CHAR, num, 1, MPI_COMM_WORLD);
                }
            } else{
                MPI_Status status;
                char* rec_buf;
                rec_buf = (char *) malloc(textLength+1);
                MPI_Recv(rec_buf, textLength, MPI_CHAR, 0, 1, MPI_COMM_WORLD,&status);
                text = rec_buf;
            }
        }
    }
    for (int num = 0; num < size; num++){
        if (num != rank) {
            MPI_Send(&index, 1, MPI_INT, num, 0, MPI_COMM_WORLD);
            MPI_Send(&flag, 1, MPI_C_BOOL, num, 0, MPI_COMM_WORLD);
        }
    }
    for (int num = 0; num < size; num++){
        if (num != rank) {
            MPI_Status status;
            int temp_index;
            bool temp_flag;
            MPI_Recv(&temp_index, 1, MPI_INT, num, 0, MPI_COMM_WORLD,&status);
            MPI_Recv(&temp_flag, 1, MPI_C_BOOL, num, 0, MPI_COMM_WORLD,&status);
            if (temp_index != -1  && (index == -1 || temp_index < index)){
                index = temp_index;
            };
            if (temp_flag){
                flag = temp_flag;
            }
        }
    }
    if (rank == 0)
    {
        double endTime = MPI_Wtime();
        double elapsedTime = endTime - startTime;
        fprintf(stdout, "Elapsed parallel time:  %f\n", elapsedTime);
        fflush(stdout);
    }

    if (rank == 0) {
        if (index != -1) {
            fprintf(stdout, "Pattern found at index %d\n", index);
            fflush(stdout);
        } else {
            fprintf(stdout, "Pattern not found.\n");
            fflush(stdout);
        }
    }
}

void findSubstring(const char* text, const char* pattern, int textLength, int patternLength, int rank, int size) {
    int index = -1;
    parallelSearch(text, pattern, textLength, patternLength, index, rank, size);
}



int main(int argc, char **argv) {
    int textLength = 10000;
    char text[textLength];
    generateRandomString(text, textLength);
    int patternLength = 4;
    const char pattern[] = "HaaH";
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
//    fprintf(stdout, "Start\n");
//    fflush(stdout);

    findSubstring(text, pattern, textLength, patternLength, rank, size);

    if (rank == 0) {
        double startTime;
        startTime = MPI_Wtime();
        int index = findSubstringNonParallel(text, pattern, textLength, patternLength);
        double endTime = MPI_Wtime();
        double elapsedTime = endTime - startTime;
        fprintf(stdout, "Elapsed nonparallel time:  %f\n", elapsedTime);
        fflush(stdout);

        if (index != -1) {
            fprintf(stdout, "Pattern found at index %d\n", index);
            fflush(stdout);
        } else {
            fprintf(stdout, "Pattern not found.\n");
            fflush(stdout);
        }
    }

    MPI_Finalize();
    return 0;
}