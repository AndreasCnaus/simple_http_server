/*
    server.c -- a stream socket based server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "gps_pack.h"
#include "gps_db.h"

// HTTP Server Configuration
#define PORT                "8080"   // TCP port to listen on for incoming HTTP POST requests
#define BACKLOG             2        // Maximum number of pending client connections in the queue

// HTTP Request / Response Buffers
#define HEADER_BUF_SIZE     2048     // Buffer size for parsing incoming HTTP headers (2 KB)
#define MAX_BODY_SIZE       4096     // Maximum allowed HTTP body size (4 KB; increase to 8192 if needed)
#define RESPONSE_BUF_SIZE   4096     // Buffer size for outgoing HTTP response (headers + body)

// GPS Data Storage
#define GPS_BUFFER_COUNT    60       // Number of GPS samples to store in memory (1 sample/sec for 1 minute)
gps_data_t gps_buffer[GPS_BUFFER_COUNT];

// -- Forward declarations -- 
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

void send_response(int fd, const char *body);
void send_binary_response(int fd, const unsigned char *data, size_t len);
void send_404(int fd);

int handle_post_request(unsigned char *data, int len);
int handle_http_request(int fd, unsigned char *header_buf, unsigned char *body_buf, int body_len);
int recv_http_request(int fd, unsigned char *header_buf, unsigned char **body_out);
int write_gpsdata_to_db(sqlite3 *db, const char *table_name, gps_data_t *gps_buf);

void sigchld_handler(int s)
{
    (void)s; // silence unused parameter warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void send_response(int fd, const char *body)
{
    char response[RESPONSE_BUF_SIZE];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        strlen(body), body);
    send(fd, response, strlen(response), 0);
}

void send_binary_response(int fd, const unsigned char *data, size_t len) {
    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: %zu\r\n"
        "\r\n", len);

    // send headers
    send(fd, header, header_len, 0);
    // send raw binary body
    send(fd, data, len, 0);
}

void send_404(int fd)
{
    send_response(fd, "404 Not Found");
}

int handle_post_request(unsigned char *data, int len)
{
    gps_data_t gps_data;
    int n = 0;

    for (int i = 0; i < len; i += GPS_PACKET_SIZE) {
        unpack_gps(data + i, &gps_data);
        if (n < GPS_BUFFER_COUNT) {
            gps_buffer[n++] = gps_data;
        } else {
            printf("Error: GPS buffer full!\n");
            // ToDo: handle the overflow 
            return -1;  // error
        }

        // print the data
        
        printf("Latitude: %.12f\n", gps_data.latitude);
        printf("Longitude: %.12f\n", gps_data.longitude);
        printf("Date: 20%02u-%02u-%02u %02u:%02u:%02u\n",
            gps_data.year, gps_data.month, gps_data.day,
            gps_data.hour, gps_data.minute, gps_data.second);
        printf("Altitude: %u m\n", gps_data.altitude);
        printf("Speed: %.2f km/h\n", (float)(gps_data.speed/100.0));
        printf("\n");
        
    }  
    
    return 0;   // success 
}

int handle_http_request(int fd, unsigned char *header_buf, unsigned char *body_buf, int body_len) 
{
    char method[8], path[256];
    int rc = 0;

    // parse method/path from header (ASCII)
    sscanf((char *)header_buf, "%7s %255s", method, path); 
    printf("Request: %s %s\n", method, path);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/") == 0)
            send_response(fd, "Hello from GET /");
        else if (strcmp(path, "/hello") == 0)
            send_response(fd, "Hello from GET /hello");
        else
            send_404(fd);
    } 
    else if (strcmp(method, "POST") == 0) {
        if (!body_buf) {
            fprintf(stderr, "POST received but no body\n");
            send_response(fd, "Missing body");
            return -1;
        }

        // handle binary POST data
        rc = handle_post_request(body_buf, body_len);
        
        // respond to client
        send_response(fd, "OK");
    }
    else {
        send_response(fd, "Method not supported");
    }

    return rc;
}

int recv_http_request(int fd, unsigned char *header_buf, unsigned char **body_out)
{
    int header_len = 0, n = 0;

    // read headers until \r\n\r\n
    while ((n = recv(fd, header_buf + header_len, HEADER_BUF_SIZE - header_len - 1, 0)) > 0) {
        header_len += n;
        header_buf[header_len] = '\0';  // null-terminate for strstr
        if (strstr((char *)header_buf, "\r\n\r\n"))
            break;
    }

    if (n <= 0) {
        perror("recv header");
        return -1;
    }

    // parse Content-Length
    int content_length = 0;
    char *cl = strstr((char *)header_buf, "Content-Length:");
    if (cl) sscanf(cl, "Content-Length: %d", &content_length);

    if (content_length <= 0 || content_length > MAX_BODY_SIZE) {
        fprintf(stderr, "Invalid Content-Length: %d\n", content_length);
        return -1;
    }

    // find start of body
    unsigned char *body_start = (unsigned char *)strstr((char *)header_buf, "\r\n\r\n");
    int already = 0;
    unsigned char *body = malloc(content_length);
    if (!body) {
        perror("malloc");
        return -1;
    }

    // copy partial body if already read
    if (body_start) {
        body_start += 4; // skip \r\n\r\n
        int body_in_header = header_len - (body_start - header_buf);
        if (body_in_header > 0) {
            memcpy(body, body_start, body_in_header);
            already = body_in_header;
        }
    }

    // read remaining bytes
    while (already < content_length) {
        n = recv(fd, body + already, content_length - already, 0);
        if (n <= 0) break;
        already += n;
    }

    if (already != content_length) {
        fprintf(stderr, "Warning: partial HTTP body received (%d/%d bytes)\n", already, content_length);
        free(body);
        return -1;
    }

    *body_out = body;
    return already;
}

int write_gpsdata_to_db(sqlite3 *db, const char *table_name, gps_data_t *gps_buf)
{
    int rc = 0;
    for (int i = 0; i < GPS_BUFFER_COUNT; i++) {
        gps_data_t gps = gps_buf[i];

        // write GPS data into the database 
        rc += db_write_entry(db, table_name, gps.latitude, gps.longitude, 
                    gps.day, gps.month, gps.year,
                    gps.hour, gps.minute, gps.second,
                    gps.altitude, gps.speed);
    }

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error occurred while writing GPS data to the database: %d\n", rc);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

int main(void)
{
    // database
    sqlite3 *db = NULL;
    const char *db_name = "tracker_board.db";
    const char *table_name = "gps_data";

    // socket 
    int sock_fd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rc;

    // initialize the SQL database
    rc = db_init(db_name, table_name, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to initialize database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // setup TCP socket 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;    // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    // fill in my IP for me

    if ((rc = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return 1;
    }

    // loop through all the results and bind to the first we can 
    for (p = servinfo; p != NULL; p= p->ai_next) {
        // try to create a socket forom the server information 
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // allow the program to reuse the port if used 
        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // bind the socket on the port
        if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure 

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // listen to incomming connections
    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // reap all dead processes, without causing the parent process to pause its main execution.
    sa.sa_handler= sigchld_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(sock_fd, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
    } else {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip));
        printf("Listening on %s:%d\n", ip, ntohs(sin.sin_port));
    }

    printf("server waiting for connections...\n");

    while (1) { // main accept loop
        sin_size = sizeof their_addr;
        new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1) {
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) {  // this is the child process
            close(sock_fd); // child noesn't need the listener

            unsigned char header_buf[HEADER_BUF_SIZE];
            unsigned char *body_buf = NULL;

            // receive HTTP request
            int content_length = recv_http_request(new_fd, header_buf, &body_buf);

            printf("Received %d bytes of binary data\n", content_length);

           if (content_length > 0) {
                rc = handle_http_request(new_fd, header_buf, body_buf, content_length);
                if (rc == 0) {
                    write_gpsdata_to_db(db, table_name, gps_buffer);
                }
            } else {
                fprintf(stderr, "Handling of partial or invalid HTTP body is not supported\n");
            }

            free(body_buf);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent does't need this
    }

    return 0;
}
