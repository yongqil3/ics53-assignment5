#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 8192 /* Max text line length */
#define MAXBUF 8192  /* Max I/O buffer size */
#define LISTENQ 1024 /* Second argument to listen() */

int check_date(char *date)
{
    if (strlen(date) != 10)
        return 0;
    char fdate[11];
    strncpy(fdate, date, 10);
    char *buf;
    buf = strtok(fdate, "/");
    if (!buf)
    {
        return 0;
    }
    int month = atoi(buf);
    buf = strtok(NULL, "/");
    if (!buf)
    {
        return 0;
    }
    int day = atoi(buf);
    buf = strtok(NULL, "/");
    if (!buf)
    {
        return 0;
    }
    int year = atoi(buf);
    if (year < 1000 || year > 9999 || day < 1 || day > 31 || month < 1 || month > 12)
        return 0; // general case
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        if (day > 31)
            return 0;
        break;
    case 2:
        if (year % 4 != 0)
        {
            if (day > 28)
                return 0;
        }
        else
        {
            if (day > 29)
                return 0;
        }
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        if (day > 30)
            return 0;
        break;
    default:
        return 0;
    }
    return 1;
}

int open_clientfd(char *hostname, char *port)
{
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV; /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0)
    {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break; /* Success */
        if (close(clientfd) < 0)
        { /* Connect failed, try another */ // line:netp:openclientfd:closefd
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else /* The last connect succeeded */
        return clientfd;
}

int clientfd = 0;

int main(int argc, const char *argv[])
{
    // insert code here...
    char *host, *port, input[MAXLINE], buf2[MAXLINE], *buffer;
    // rio_t rio;
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];
    clientfd = open_clientfd(host, port);
    char *spliter = " \n";
    while (1)
    { // while loop to get user input
        printf("> ");
        memset(input, 0, 80);
        fgets(input, (sizeof input / sizeof input[0]), stdin);
        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = 0;
        strcpy(buf2, input);
        if (strcmp(input, "quit") == 0)
        {
            break;
        }
        buffer = strtok(buf2, spliter);
        if (strcmp(buffer, "MaxPossible") == 0)
        {
            buffer = strtok(NULL, spliter);
            if (strcmp(buffer, "MRNA") != 0 && strcmp(buffer, "PFE") != 0)
            {
                printf("Invalid Syntax!\n");
                continue;
            }
            char original_length = strlen(input);
            char buf3[MAXLINE];
            buf3[0] = original_length; // set first byte to size
            buf3[1] = '\0';
            strcat(buf3, input);
            // printf("%s\n", buf3);
            write(clientfd, buf3, strlen(buf3));
            memset(input, 0, sizeof(input));
            read(clientfd, input, MAXLINE);
            fputs(&input[1], stdout);
            continue;
        }
        if (strcmp(buffer, "PricesOnDate") == 0)
        {
            buffer = strtok(NULL, spliter);
            if (strcmp(buffer, "MRNA") != 0 && strcmp(buffer, "PFE") != 0)
            {
                printf("Invalid Syntax!\n");
                continue;
            }
            buffer = strtok(NULL, spliter);
            if (!check_date(buffer))
            {
                printf("Invalid Syntax!\n");
                continue;
            }

            char original_length = strlen(input);
            char buf3[MAXLINE];
            buf3[0] = original_length; // set first byte to size
            buf3[1] = '\0';
            strcat(buf3, input);
            // printf("%s\n", buf3);
            write(clientfd, buf3, strlen(buf3));
            memset(input, 0, sizeof(input));
            read(clientfd, input, MAXLINE);
            fputs(&input[1], stdout);

            continue;
        }
        else
        {
            printf("Invalid Syntax!\n");
            continue;
        }
    }
    close(clientfd); // line:netp:echoclient:close
    exit(0);
    return 0;
}
