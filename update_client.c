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

int token_length = 0;

char **split_date(char *line)
{
    int date_token_length = 0;
    int capacity = 16;

    char **tokens = malloc(capacity * sizeof(char *));

    char *delimiters = "-";
    char *token = strtok(line, delimiters);

    while (token != NULL)
    {
        tokens[date_token_length] = token;
        date_token_length++;
        token = strtok(NULL, delimiters);
    }
    tokens[date_token_length] = NULL;
    return tokens;
}
int check_date(char *date)
{
    char **tokens = split_date(date);
    int year = atoi(tokens[0]);
    int month = atoi(tokens[1]);
    int day = atoi(tokens[2]);
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

char **split_line(char *line)
{
    token_length = 0;
    int capacity = 16;

    char **tokens = malloc(capacity * sizeof(char *));

    char *delimiters = " \t\r\n";
    char *token = strtok(line, delimiters);

    while (token != NULL)
    {
        tokens[token_length] = token;
        token_length++;
        token = strtok(NULL, delimiters);
    }
    tokens[token_length] = NULL;
    return tokens;
}

int main(int argc, const char *argv[])
{
    char line[256];
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

    while (1)
    { // while loop to get user input
        printf("> ");
        fgets(line, 256, stdin);
        char **tokens = split_line(line);
        if (!strcmp(tokens[0], "quit"))
        {
            if (token_length != 1)
            {
                printf("Invalid Syntax!\n");
            }
            continue;
        }
        if (!strcmp(tokens[0], "PricesOnDate"))
        {
            if (token_length != 2)
            {
                printf("Invalid Syntax!\n");
            }
            else
            {
                if (check_date(tokens[1]))
                {
                    write(clientfd, line, strlen(line));
                    memset(line, 0, 256);
                    read(clientfd, line, 256);
                    printf("%s", line);
                }
            }
            continue;
        }
        if (!strcmp(tokens[0], "MaxPossible"))
        {
            if (token_length != 5)
            {
                printf("Invalid Syntax!\n");
            }
            else
            {
                if (!strcmp(tokens[1], "profit") || !strcmp(tokens[1], "loss"))
                {
                    if (!strcmp(tokens[2], "MRNA") || !strcmp(tokens[2], "PFE"))
                    {
                        if (check_date(tokens[3]) && check_date(tokens[4]))
                        {
                            // previous date must be before the latter date
                            char **tokens3 = split_date(tokens[3]);
                            char **tokens4 = split_date(tokens[4]);
                            int year3 = atoi(tokens3[0]);
                            int month3 = atoi(tokens3[1]);
                            int day3 = atoi(tokens3[2]);
                            int year4 = atoi(tokens4[0]);
                            int month4 = atoi(tokens4[1]);
                            int day4 = atoi(tokens4[2]);
                            if (year3 > year4)
                            {
                                printf("Invalid Syntax!\n");
                            }
                            else if (year3 == year4)
                            {
                                if (month3 > month4)
                                {
                                    printf("Invalid Syntax!\n");
                                }
                                else if (month3 == month4)
                                {
                                    if (day3 > day4)
                                    {
                                        printf("Invalid Syntax!\n");
                                    }
                                    else
                                    {
                                        write(clientfd, line, strlen(line));
                                        memset(line, 0, 256);
                                        read(clientfd, line, 256);
                                        printf("%s", line);
                                    }
                                }
                                else
                                {
                                    write(clientfd, line, strlen(line));
                                    memset(line, 0, 256);
                                    read(clientfd, line, 256);
                                    printf("%s", line);
                                }
                            }
                            else
                            {
                                write(clientfd, line, strlen(line));
                                memset(line, 0, 256);
                                read(clientfd, line, 256);
                                printf("%s", line);
                            }
                        }
                    }
                }
            }
            continue;
        }
        else
        {
            printf("Invalid Syntax!\n");
            continue;
        }
        free(tokens);
    }
    close(clientfd);
    exit(0);
    return 0;
}
