#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <string.h>

typedef struct {
    int* data;
    int start;
    int end;
} ThreadArgs;

int compare_int(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void* thread_sort(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    qsort(args->data + args->start, args->end - args->start, sizeof(int), compare_int);
    return NULL;
}

void merge(int* data, int* temp, int num_threads, const int* starts, const int* ends, int size) {
    int* current_pos = malloc(num_threads * sizeof(int));
    for (int i = 0; i < num_threads; i++) {
        current_pos[i] = starts[i];
    }

    int index = 0;
    while (index < size) {
        int min_val = INT_MAX;
        int min_thread = -1;

        for (int i = 0; i < num_threads; i++) {
            if (current_pos[i] < ends[i]) {
                int val = data[current_pos[i]];
                if (val < min_val) {
                    min_val = val;
                    min_thread = i;
                }
            }
        }

        if (min_thread == -1) break;

        temp[index++] = min_val;
        current_pos[min_thread]++;
    }

    memcpy(data, temp, size * sizeof(int));
    free(current_pos);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <array_size> <num_threads>\n", argv[0]);
        return 1;
    }

    int size = atoi(argv[1]);
    int num_threads = atoi(argv[2]);

    if (num_threads <= 0 || size <= 0) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    int* data = malloc(size * sizeof(int));
    int* temp = malloc(size * sizeof(int));
    if (!data || !temp) {
        perror("malloc failed");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        data[i] = rand() % 1000;
    }

    ThreadArgs* thread_args = malloc(num_threads * sizeof(ThreadArgs));
    int* starts = malloc(num_threads * sizeof(int));
    int* ends = malloc(num_threads * sizeof(int));

    int chunk_size = size / num_threads;
    int remainder = size % num_threads;
    int current_start = 0;

    for (int i = 0; i < num_threads; i++) {
        int current_end = current_start + chunk_size + (i < remainder ? 1 : 0);
        starts[i] = current_start;
        ends[i] = current_end;
        thread_args[i].data = data;
        thread_args[i].start = current_start;
        thread_args[i].end = current_end;
        current_start = current_end;
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    struct timespec start_time, end_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_sort, &thread_args[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    merge(data, temp, num_threads, starts, ends, size);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1]) {
            printf("Array is not sorted correctly at index %d\n", i);
            break;
        }
    }

    long long elapsed_ns = (end_time.tv_sec - start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
    printf("Time taken: %.2f milliseconds\n", elapsed_ns / 1000000.0);

    free(data);
    free(temp);
    free(thread_args);
    free(starts);
    free(ends);
    free(threads);

    return 0;
}