/*
 * MT25057
 * PA02: Analysis of Network I/O primitives using "perf" tool
 * Part A1: Two-Copy Implementation (Baseline) - Server
 * 
 * This server uses standard send()/recv() socket primitives
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
#define NUM_FIELDS 8
#define DEFAULT_MSG_SIZE 1024
#define BACKLOG 128

/* Global configuration */
static int g_message_size = DEFAULT_MSG_SIZE;
static volatile int g_running = 1;

/* Message structure with 8 dynamically allocated string fields */
typedef struct {
    char *fields[NUM_FIELDS];
    size_t field_sizes[NUM_FIELDS];
} Message;

/* Thread argument structure */
typedef struct {
    int client_fd;
    int thread_id;
    struct sockaddr_in client_addr;
} ThreadArg;

/* Statistics structure */
typedef struct {
    unsigned long long bytes_sent;
    unsigned long long messages_sent;
    double elapsed_time;
} Stats;

/* Signal handler for graceful shutdown */
void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

/* Allocate and initialize message structure */
Message* create_message(size_t total_size) {
    Message *msg = (Message*)malloc(sizeof(Message));
    if (!msg) {
        perror("Failed to allocate message structure");
        return NULL;
    }
    
    /* Distribute size among 8 fields */
    size_t field_size = total_size / NUM_FIELDS;
    size_t remainder = total_size % NUM_FIELDS;
    
    for (int i = 0; i < NUM_FIELDS; i++) {
        size_t size = field_size + (i < (int)remainder ? 1 : 0);
        msg->field_sizes[i] = size;
        msg->fields[i] = (char*)malloc(size);
        if (!msg->fields[i]) {
            perror("Failed to allocate message field");
            /* Clean up previously allocated fields */
            for (int j = 0; j < i; j++) {
                free(msg->fields[j]);
            }
            free(msg);
            return NULL;
        }
        /* Initialize with pattern data */
        memset(msg->fields[i], 'A' + i, size);
    }
    
    return msg;
}

/* Free message structure */
void destroy_message(Message *msg) {
    if (msg) {
        for (int i = 0; i < NUM_FIELDS; i++) {
            free(msg->fields[i]);
        }
        free(msg);
    }
}

/* Serialize message into a contiguous buffer for sending */
char* serialize_message(Message *msg, size_t *total_size) {
    *total_size = 0;
    for (int i = 0; i < NUM_FIELDS; i++) {
        *total_size += msg->field_sizes[i];
    }
    
    char *buffer = (char*)malloc(*total_size);
    if (!buffer) {
        return NULL;
    }
    
    size_t offset = 0;
    for (int i = 0; i < NUM_FIELDS; i++) {
        memcpy(buffer + offset, msg->fields[i], msg->field_sizes[i]);
        offset += msg->field_sizes[i];
    }
    
    return buffer;
}

/* Client handler thread function */
void* client_handler(void *arg) {
    ThreadArg *targ = (ThreadArg*)arg;
    int client_fd = targ->client_fd;
    int thread_id = targ->thread_id;
    
    printf("[Thread %d] Client connected from %s:%d\n",
           thread_id,
           inet_ntoa(targ->client_addr.sin_addr),
           ntohs(targ->client_addr.sin_port));
    
    /* Create message structure */
    Message *msg = create_message(g_message_size);
    if (!msg) {
        close(client_fd);
        free(targ);
        return NULL;
    }
    
    /* Serialize message for sending */
    size_t buffer_size;
    char *buffer = serialize_message(msg, &buffer_size);
    if (!buffer) {
        destroy_message(msg);
        close(client_fd);
        free(targ);
        return NULL;
    }
    
    Stats stats = {0, 0, 0.0};
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Set TCP_NODELAY to disable Nagle's algorithm */
    int flag = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    
    /* Send messages continuously until client disconnects */
    while (g_running) {
        ssize_t sent = send(client_fd, buffer, buffer_size, 0);
        if (sent <= 0) {
            if (sent < 0 && errno != EPIPE && errno != ECONNRESET) {
                perror("send error");
            }
            break;
        }
        stats.bytes_sent += sent;
        stats.messages_sent++;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    stats.elapsed_time = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    
    /* Print statistics */
    double throughput_gbps = (stats.bytes_sent * 8.0) / (stats.elapsed_time * 1e9);
    printf("[Thread %d] Stats: %.2f GB sent, %.2f Gbps, %llu messages in %.2f seconds\n",
           thread_id,
           stats.bytes_sent / 1e9,
           throughput_gbps,
           stats.messages_sent,
           stats.elapsed_time);
    
    /* Cleanup */
    free(buffer);
    destroy_message(msg);
    close(client_fd);
    free(targ);
    
    return NULL;
}

void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [-p port] [-s message_size]\n", prog);
    fprintf(stderr, "  -p port         : Server port (default: %d)\n", DEFAULT_PORT);
    fprintf(stderr, "  -s message_size : Message size in bytes (default: %d)\n", DEFAULT_MSG_SIZE);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int opt;
    
    while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                g_message_size = atoi(optarg);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                return (opt == 'h') ? 0 : 1;
        }
    }
    
    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    /* Create socket */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        return 1;
    }
    
    /* Set socket options */
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    
    /* Bind to address */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }
    
    /* Listen for connections */
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    
    printf("A1 Two-Copy Server started on port %d (message size: %d bytes)\n",
           port, g_message_size);
    printf("Using send()/recv() - Standard two-copy mechanism\n");
    printf("Press Ctrl+C to stop\n\n");
    
    int thread_id = 0;
    
    /* Accept connections and spawn threads */
    while (g_running) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept failed");
            continue;
        }
        
        /* Create thread argument */
        ThreadArg *targ = (ThreadArg*)malloc(sizeof(ThreadArg));
        if (!targ) {
            perror("Failed to allocate thread argument");
            close(client_fd);
            continue;
        }
        
        targ->client_fd = client_fd;
        targ->thread_id = thread_id++;
        targ->client_addr = client_addr;
        
        /* Spawn client handler thread */
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        
        if (pthread_create(&thread, &attr, client_handler, targ) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(targ);
        }
        
        pthread_attr_destroy(&attr);
    }
    
    printf("\nServer shutting down...\n");
    close(server_fd);
    
    return 0;
}

/* This code was generated with the assistance of Claude Opus 4.5 by Anthropic. */
