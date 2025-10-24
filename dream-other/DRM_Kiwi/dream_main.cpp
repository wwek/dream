// Standalone main for dream DRM receiver
// This replaces the KiwiSDR extension interface

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// Include dream receiver
#include "dream/DRM_main.h"
#include "dream/DRMReceiver.h"

static volatile bool running = true;

void signal_handler(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    running = false;
}

void print_usage(const char *prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  -i <file>    Input IQ file (required)\n");
    printf("  -o <file>    Output audio file (default: output.wav)\n");
    printf("  -r <rate>    Sample rate in Hz (default: 48000)\n");
    printf("  -h           Show this help\n");
}

int main(int argc, char **argv) {
    const char *input_file = NULL;
    const char *output_file = "output.wav";
    int sample_rate = 48000;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "i:o:r:h")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'r':
                sample_rate = atoi(optarg);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                return (opt == 'h') ? 0 : 1;
        }
    }

    if (!input_file) {
        fprintf(stderr, "Error: Input file required\n");
        print_usage(argv[0]);
        return 1;
    }

    printf("Dream DRM Receiver (Standalone)\n");
    printf("Input:  %s\n", input_file);
    printf("Output: %s\n", output_file);
    printf("Sample rate: %d Hz\n", sample_rate);

    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // TODO: Initialize DRM receiver
    // This would need proper integration with the dream receiver code

    printf("\nDRM receiver initialized\n");
    printf("Press Ctrl+C to stop\n\n");

    // Main processing loop
    while (running) {
        // TODO: Process DRM data
        sleep(1);
    }

    printf("\nShutdown complete\n");
    return 0;
}
