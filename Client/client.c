// name: yongqi liang id:75181206
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

int token_length = 0;
int clientfd = 0;

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
        return 0; 
    if (month == 2 && day > 29)
        return 0; // february
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
        return 0; // april, june, september, november
    if (month == 2 && day == 29 && year % 4 != 0)
        return 0; // february in a non-leap year
    return 1;
}

int open_clientfd(char *hostname, char *port)
{
    int clientfd;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV; /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */
    getaddrinfo(hostname, port, &hints, &listp);

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break; /* Success */

        close(clientfd); /* Connect failed, try another */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else /* The last connect succeeded */
        return clientfd;
}

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
    char line[MAXLINE];
    char *host, *port, input[MAXLINE], *buffer;
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
                    char length = strlen(input);
                    char temp[MAXLINE];
                    temp[0] = length; // set first byte to size
                    temp[1] = '\0';
                    strcat(temp, input);

                    write(clientfd, temp, strlen(temp));
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
                            //check previous date is before next date
                           
                           //check if date1 is before date2
                           if(date1_year_int < date2_year_int)
                           {
                               char length = strlen(input);
                               char temp[MAXLINE];
                               temp[0] = length; // set first byte to size
                               temp[1] = '\0';
                               strcat(temp, input);
                               write(clientfd, temp, strlen(temp));
                               memset(input, 0, sizeof(input));
                               read(clientfd, input, MAXLINE);
                               printf("%s\n", input);
                               continue;
                           }
                            else if(date1_year_int == date2_year_int)
                            {
                                if(date1_month_int < date2_month_int)
                                {
                                    char length = strlen(input);
                                    char temp[MAXLINE];
                                    temp[0] = length; // set first byte to size
                                    temp[1] = '\0';
                                    strcat(temp, input);
                                    write(clientfd, temp, strlen(temp));
                                    memset(input, 0, sizeof(input));
                                    read(clientfd, input, MAXLINE);
                                    printf("%s\n", input);
                                    continue;
                                }
                                else if(date1_month_int == date2_month_int)
                                {
                                    if(date1_day_int < date2_day_int)
                                    {
                                        char length = strlen(input);
                                        char temp[MAXLINE];
                                        temp[0] = length; // set first byte to size
                                        temp[1] = '\0';
                                        strcat(temp, input);
                                        write(clientfd, temp, strlen(temp));
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
