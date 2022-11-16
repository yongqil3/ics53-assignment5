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
    char line[9999];
    char *host, *port, input[MAXLINE], buf2[MAXLINE], *buffer;
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
        memset(line, 0, sizeof(line));
        fgets(line, 256, stdin);
        strcpy(input, line);
        char **tokens = split_line(line);
        //copy line
        if (!strcmp(tokens[0], "quit"))
        {
            if (token_length != 1)
            {
                printf("Invalid Syntax!\n");
            }
            exit(0);
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
                    char original_length = strlen(input);
                    char buf3[MAXLINE];
                    buf3[0] = original_length; // set first byte to size
                    buf3[1] = '\0';
                    strcat(buf3, input);
                    write(clientfd, buf3, strlen(buf3));
                    memset(input, 0, sizeof(input));
                    read(clientfd, input, MAXLINE);
                    printf("%s\n", input);
                    continue;
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
                        printf("token[3] = %s, token[4] = %s\n", tokens[3], tokens[4]);
                        //copy tokens[3]  to new string array
                        char date1[11];
                        strcpy(date1, tokens[3]);
                        char date2[11];
                        strcpy(date2, tokens[4]);
                        if (check_date(tokens[3]) && check_date(tokens[4]))
                        {
                            //spilt date1 and date2
                            char *date1_year = strtok(date1, "-");
                            char *date1_month = strtok(NULL, "-");
                            char *date1_day = strtok(NULL, "-");
                            char *date2_year = strtok(date2, "-");
                            char *date2_month = strtok(NULL, "-");
                            char *date2_day = strtok(NULL, "-");
                            //convert to int
                            int date1_year_int = atoi(date1_year);
                            int date1_month_int = atoi(date1_month);
                            int date1_day_int = atoi(date1_day);
                            int date2_year_int = atoi(date2_year);
                            int date2_month_int = atoi(date2_month);
                            int date2_day_int = atoi(date2_day);
                            printf("date1_year_int = %d, date1_month_int = %d, date1_day_int = %d\n", date1_year_int, date1_month_int, date1_day_int);
                            //check previous date is before next date
                           
                           //check if date1 is before date2
                           if(date1_year_int < date2_year_int)
                           {
                               char original_length = strlen(input);
                               char buf3[MAXLINE];
                               buf3[0] = original_length; // set first byte to size
                               buf3[1] = '\0';
                               strcat(buf3, input);
                               write(clientfd, buf3, strlen(buf3));
                               memset(input, 0, sizeof(input));
                               read(clientfd, input, MAXLINE);
                               printf("%s\n", input);
                               continue;
                           }
                            else if(date1_year_int == date2_year_int)
                            {
                                if(date1_month_int < date2_month_int)
                                {
                                    char original_length = strlen(input);
                                    char buf3[MAXLINE];
                                    buf3[0] = original_length; // set first byte to size
                                    buf3[1] = '\0';
                                    strcat(buf3, input);
                                    write(clientfd, buf3, strlen(buf3));
                                    memset(input, 0, sizeof(input));
                                    read(clientfd, input, MAXLINE);
                                    printf("%s\n", input);
                                    continue;
                                }
                                else if(date1_month_int == date2_month_int)
                                {
                                    if(date1_day_int < date2_day_int)
                                    {
                                        char original_length = strlen(input);
                                        char buf3[MAXLINE];
                                        buf3[0] = original_length; // set first byte to size
                                        buf3[1] = '\0';
                                        strcat(buf3, input);
                                        write(clientfd, buf3, strlen(buf3));
                                        memset(input, 0, sizeof(input));
                                        read(clientfd, input, MAXLINE);
                                        printf("%s\n", input);
                                        continue;
                                    }
                                    else
                                    {
                                        printf("Invalid Syntax!\n");
                                        continue;
                                    }
                                }
                                else
                                {
                                    printf("Invalid Syntax!\n");
                                    continue;
                                }
                            }
                            else
                            {
                                printf("Invalid Syntax!\n");
                                continue;
                            }
                            printf("Arrived here\n");
                            // previous date must be before the latter date
                            
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
    return 0;
}
