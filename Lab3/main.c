#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "reverse.h"

#define MAX_BUFFER_SIZE (10 * 1024 * 1024) // 10 МБ

int main(int argc, char *argv[]) {
    char *input_file = NULL;
    char *output_file = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "i:o:")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i input] [-o output]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        if (input_file == NULL) {
            input_file = argv[optind];
            optind++;
        }
        if (optind < argc && output_file == NULL) {
            output_file = argv[optind];
            optind++;
        }
        if (optind < argc) {
            fprintf(stderr, "Too many arguments\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *in_stream = stdin;
    FILE *out_stream = stdout;

    if (input_file != NULL) {
        in_stream = fopen(input_file, "rb");
        if (!in_stream) {
            perror("Failed to open input file");
            exit(EXIT_FAILURE);
        }
    }

    if (output_file != NULL) {
        out_stream = fopen(output_file, "wb");
        if (!out_stream) {
            perror("Failed to open output file");
            if (in_stream != stdin) fclose(in_stream);
            exit(EXIT_FAILURE);
        }
    } else if (isatty(fileno(stdout))) {
        fprintf(stderr, "Error: Output file not specified and stdout is a terminal\n");
        if (in_stream != stdin) fclose(in_stream);
        exit(EXIT_FAILURE);
    }

    unsigned char *buffer = malloc(MAX_BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        if (in_stream != stdin) fclose(in_stream);
        if (out_stream != stdout) fclose(out_stream);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(buffer, 1, MAX_BUFFER_SIZE, in_stream);
    if (bytes_read == 0) {
        fprintf(stderr, "No data read from input\n");
        free(buffer);
        if (in_stream != stdin) fclose(in_stream);
        if (out_stream != stdout) fclose(out_stream);
        exit(EXIT_FAILURE);
    }

    reverse_buffer(buffer, bytes_read);

    size_t bytes_written = fwrite(buffer, 1, bytes_read, out_stream);
    if (bytes_written != bytes_read) {
        perror("Error writing output");
        free(buffer);
        if (in_stream != stdin) fclose(in_stream);
        if (out_stream != stdout) fclose(out_stream);
        exit(EXIT_FAILURE);
    }

    free(buffer);
    if (in_stream != stdin) fclose(in_stream);
    if (out_stream != stdout) fclose(out_stream);

    return EXIT_SUCCESS;
}
