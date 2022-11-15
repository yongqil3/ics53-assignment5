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
#define DATA_NUM 504
#define DATE_LEN 11
#define UNKNOWN_STR "Unknown"

struct STOCK
{
    char date[DATA_NUM][DATE_LEN];
    float close[DATA_NUM];
};

struct STOCK MRNA_stock;
struct STOCK PFE_stock;


char **split_data(char *line)
{
int token_length = 0;
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

void read_file(char *file1, char *file2)
{
    FILE *fd1 = fopen(file1, "r");
    FILE *fd2 = fopen(file2, "r");
    char line[1000];

    // if(fd1 < 0){
    //     fprintf(STDERR_FILENO, "Can not open %s", file1);
    //     exit(1);
    // }

    // if(fd2 < 0){
    //     fprintf(STDERR_FILENO, "Can not open %s", file2);
    //     exit(1);
    // }
    int count_1 = 0;
    int count_2 = 0;
    while (fgets(line, 100, fd1))
    {
        char **tokens = split_data(line);
        strcpy(MRNA_stock.date[count_1], tokens[0]);
        MRNA_stock.close[count_1] = atof(tokens[1]);
        count_1++;
    }
    while (fgets(line, 100, fd2))
    {
        char **tokens = split_data(line);
        strcpy(PFE_stock.date[count_2], tokens[0]);
        PFE_stock.close[count_2] = atof(tokens[1]);
        count_2++;
    }
}

float getPrice(char *stock, char *date)
{
   if(!strcmp(stock, "MRNA"))
   {
         for(int i = 0; i < DATA_NUM; i++)
         {
              if(!strcmp(MRNA_stock.date[i], date))
              {
                return MRNA_stock.close[i];
              }
         }
    }
    else if(!strcmp(stock, "PFE"))
    {
         for(int i = 0; i < DATA_NUM; i++)
         {
              if(!strcmp(PFE_stock.date[i], date))
              {
                return PFE_stock.close[i];
              }
         }
   }
    return -1;
}


int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval = 1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0)
    {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, // line:netp:csapp:setsockopt
                   (const void *)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0)
        { /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

void sendB(int connfd)
{

    //-----------------------//
    size_t n;
    char line[MAXLINE];
    char output[MAXLINE];
    char *command;
    char *date;
    char *start_date;
    char *end_date;
    char *stock; //stock name
    char *flag; //profit or loss
    while ((n = read(connfd, line, MAXLINE)) != 0)
    {
        char **tokens = split_data(line);
        command = tokens[0];
        if (!strcmp(command, "PricesOnDate"))
        {
            date = tokens[1];
            // to do get price
            float price = getPrice(stock, date);
            sprintf(output, "%f", price);
        }
        else if(!strcmp(command, "MaxProfit"))
        {
            flag = tokens[1];
            stock = tokens[2];
            start_date = tokens[3];
            end_date = tokens[4];
            // to do get max profit
            if(!strcmp(flag, "profit"))
            {
                // to do get max profit
            }
            if(!strcmp(flag, "loss"))
            {
                // to do get max loss
            }
        }
        write(connfd, output, strlen(output));
    }
        
}

int main(int argc, const char *argv[])
{
    // insert code here...
    // argv[0] == server
    // argv[1] == APPL.csv
    // argv[2] == TWTR.csv
    // argv[3] == port number
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s <filename1> <filename2> <port>\n", argv[0]);
        exit(1);
    }

    printf("server started\n");
    read_file(argv[1], argv[2]);
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Check command line args */

    listenfd = open_listenfd(argv[3]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen); // line:netp:tiny:accept
        getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        sendB(connfd);
        close(connfd); // line:netp:tiny:close
    }
    return 0;
}
