/*
 * MT25057
 * PA02: Analysis of Network I/O primitives using "perf" tool
 * Part A1: Two-Copy Implementation (Baseline) - Client
 * 
 * This client uses standard send()/recv() socket primitives
 * which involves two copies: user space to kernel space, then kernel to NIC
 * 
 * Author: Aayush Amritesh (MT25057)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#define DEFAULT_PORT 8081
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_DURATION 10
#define DEFAULT_THREADS 1
#define DEFAULT_MSG_SIZE 1024

/* Global configuration */
static char g_host[256] = DEFAULT_HOST;
static int g_port = DEFAULT_PORT;
static int g_duration = DEFAULT_DURATION;
static int g_message_size = DEFAULT_MSG_SIZE;
static volatile int g_running = 1;

/* Thread statistics structure */
typedef struct {
    int thread_id;
    unsigned long long bytes_received;
    unsigned long long messages_received;
    double elapsed_time;
    double latency_sum;
    unsigned long long latency_count;
} ThreadStats;

/* Global statistics */
static pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static ThreadStats *g_thread_stats;
static int g_num_threads;

/* Signal handler */
void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

/* Client thread function */
void* client_thread(void *arg) {
    int thread_id = *(int*)arg;
    free(arg);
    
    ThreadStats *stats = &g_thread_stats[thread_id];
    stats->thread_id = thread_id;
    stats->bytes_received = 0;
    stats->messages_received = 0;
    stats->latency_sum = 0;
    stats->latency_count = 0;
    
    /* Create socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return NULL;
    }
    
    /* Set TCP_NODELAY */
    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    
    /* Connect to server */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(g_port);
    
    if (inet_pton(AF_INET, g_host, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return NULL;
    }
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return NULL;
    }
    
    printf("[Thread %d] Connected to server\n", thread_id);
    
    /* Allocate receive buffer */
    char *buffer = (char*)malloc(g_message_size);
    if (!buffer) {
        perror("Failed to allocate buffer");
        close(sockfd);
        return NULL;
    }
    
    struct timespec start, end, msg_start, msg_end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Receive data for specified duration */
    while (g_running) {
        clock_gettime(CLOCK_MONOTONIC, &msg_start);
        
        ssize_t total_received = 0;
        while (total_received < g_message_size && g_running) {
            ssize_t received = recv(sockfd, buffer + total_received,
                                   g_message_size - total_received, 0);
            if (received <= 0) {
                if (received < 0 && errno != EINTR) {
                    perror("recv error");
                }
                g_running = 0;
                break;
            }
            total_received += received;
        }
        
        if (total_received > 0) {
            clock_gettime(CLOCK_MONOTONIC, &msg_end);
            
            stats->bytes_received += total_received;
            stats->messages_received++;
            
            /* Calculate latency for this message */
            double latency = (msg_end.tv_sec - msg_start.tv_sec) * 1e6 +
                           (msg_end.tv_nsec - msg_start.tv_nsec) / 1e3;
            stats->latency_sum += latency;
            stats->latency_count++;
        }
        
        /* Check duration */
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;
        if (elapsed >= g_duration) {
            break;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    stats->elapsed_time = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    
    free(buffer);
    close(sockfd);
    
    return NULL;
}

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-h host] [-p port] [-t threads] [-d duration] [-s msg_size]\n", prog);
    fprintf(stderr, "  -h host     : Server host (default: %s)\n", DEFAULT_HOST);
    fprintf(stderr, "  -p port     : Server port (default: %d)\n", DEFAULT_PORT);
    fprintf(stderr, "  -t threads  : Number of client threads (default: %d)\n", DEFAULT_THREADS);
    fprintf(stderr, "  -d duration : Test duration in seconds (default: %d)\n", DEFAULT_DURATION);
    fprintf(stderr, "  -s msg_size : Message size in bytes (default: %d)\n", DEFAULT_MSG_SIZE);
}

int main(int argc, char *argv[]) {
    g_num_threads = DEFAULT_THREADS;
    int opt;
    
    while ((opt = getopt(argc, argv, "h:p:t:d:s:H")) != -1) {
        switch (opt) {
            case 'h':
                strncpy(g_host, optarg, sizeof(g_host) - 1);
                break;
            case 'p':
                g_port = atoi(optarg);
                break;
            case 't':
                g_num_threads = atoi(optarg);
                break;
            case 'd':
                g_duration = atoi(optarg);
                break;
            case 's':
                g_message_size = atoi(optarg);
                break;
            case 'H':
            default:
                print_usage(argv[0]);
                return (opt == 'H') ? 0 : 1;
        }
    }
    
    signal(SIGINT, signal_handler);
    
    printf("A1 Two-Copy Client\n");
    printf("Configuration: host=%s, port=%d, threads=%d, duration=%ds, msg_size=%d\n",
           g_host, g_port, g_num_threads, g_duration, g_message_size);
    printf("Using recv() - Standard two-copy mechanism\n\n");
    
    /* Allocate thread statistics array */
    g_thread_stats = (ThreadStats*)calloc(g_num_threads, sizeof(ThreadStats));
    if (!g_thread_stats) {
        perror("Failed to allocate thread stats");
        return 1;
    }
    
    /* Create threads */
    pthread_t *threads = (pthread_t*)malloc(g_num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("Failed to allocate threads array");
        free(g_thread_stats);
        return 1;
    }
    
    struct timespec global_start, global_end;
    clock_gettime(CLOCK_MONOTONIC, &global_start);
    
    for (int i = 0; i < g_num_threads; i++) {
        int *tid = (int*)malloc(sizeof(int));
        *tid = i;
        if (pthread_create(&threads[i], NULL, client_thread, tid) != 0) {
            perror("Failed to create thread");
            free(tid);
        }
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < g_num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &global_end);
    double global_elapsed = (global_end.tv_sec - global_start.tv_sec) +
                           (global_end.tv_nsec - global_start.tv_nsec) / 1e9;
    
    /* Aggregate statistics */
    unsigned long long total_bytes = 0;
    unsigned long long total_messages = 0;
    double total_latency = 0;
    unsigned long long total_latency_count = 0;
    
    printf("\n--- Per-Thread Statistics ---\n");
    for (int i = 0; i < g_num_threads; i++) {
        ThreadStats *s = &g_thread_stats[i];
        double throughput = (s->bytes_received * 8.0) / (s->elapsed_time * 1e9);
        double avg_latency = s->latency_count > 0 ? s->latency_sum / s->latency_count : 0;
        
        printf("[Thread %d] Received: %.2f MB, Throughput: %.2f Gbps, Avg Latency: %.2f us\n",
               i, s->bytes_received / 1e6, throughput, avg_latency);
        
        total_bytes += s->bytes_received;
        total_messages += s->messages_received;
        total_latency += s->latency_sum;
        total_latency_count += s->latency_count;
    }
    
    /* Print aggregate statistics */
    double total_throughput = (total_bytes * 8.0) / (global_elapsed * 1e9);
    double avg_latency = total_latency_count > 0 ? total_latency / total_latency_count : 0;
    
    printf("\n--- Aggregate Statistics ---\n");
    printf("Total bytes received: %.2f MB\n", total_bytes / 1e6);
    printf("Total messages: %llu\n", total_messages);
    printf("Total throughput: %.4f Gbps\n", total_throughput);
    printf("Average latency: %.2f us\n", avg_latency);
    printf("Elapsed time: %.2f seconds\n", global_elapsed);
    
    /* Output CSV-friendly format */
    printf("\n--- CSV Output ---\n");
    printf("implementation,threads,msg_size,throughput_gbps,latency_us,bytes_total,elapsed_s\n");
    printf("two_copy,%d,%d,%.4f,%.2f,%llu,%.2f\n",
           g_num_threads, g_message_size, total_throughput, avg_latency, total_bytes, global_elapsed);
    
    free(threads);
    free(g_thread_stats);
    
    return 0;
}

/* This code was generated with the assistance of Claude Opus 4.5 by Anthropic. */
