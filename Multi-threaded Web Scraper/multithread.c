#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

#define BUFFER_SIZE 4096
#define NUM_THREADS 3

// Use ONLY HTTP URLs (HTTPS will fail with raw sockets)
char *urls[NUM_THREADS] = {
    "http://example.org",
    "http://httpbin.org/html",
    "http://jsonplaceholder.typicode.com/posts"
};

void *fetch_url(void *arg) {
    int thread_id = *(int *)arg;
    char *url = urls[thread_id];

    printf("Thread %d: fetching %s...\n", thread_id, url);

    char host[256], path[1024];
    int port = 80;

    // Parse URL manually
    sscanf(url, "http://%255[^/]/%1023[^\n]", host, path);

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        pthread_exit(NULL);
    }

    // Resolve host
    struct hostent *server = gethostbyname(host);
    if (!server) {
        printf("Thread %d: Failed to resolve host.\n", thread_id);
        close(sock);
        pthread_exit(NULL);
    }

    // Connect
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    memcpy(&serv.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        printf("Thread %d: Connection failed.\n", thread_id);
        close(sock);
        pthread_exit(NULL);
    }

    // Send GET request
    char request[2048];
    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, host);

    send(sock, request, strlen(request), 0);

    // Save response to file
    char filename[32];
    sprintf(filename, "output_%d.txt", thread_id);
    FILE *fp = fopen(filename, "w");

    char buffer[BUFFER_SIZE];
    int bytes;

    while ((bytes = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }

    fclose(fp);
    close(sock);

    printf("Thread %d: saved to %s\n", thread_id, filename);
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, fetch_url, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
