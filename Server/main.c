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
#define APPL_STR "MRNA"
#define TWTR_STR "PFE"
#define PRICES_STR "Prices"
#define MAXPROFIT_STR "MaxProfit"
#define DATA_NUM 503
#define DATE_LEN 11
#define UNKNOWN_STR "Unknown"

struct STOCK
{
    char date[DATA_NUM][DATE_LEN];
    float close[DATA_NUM];
};

struct STOCK twtr_stock;
struct STOCK appl_stock;

void read_file(char *file1, char *file2)
{
    FILE *fd1 = fopen(file1, "r");
    FILE *fd2 = fopen(file2, "r");
    char buf[1000];

    // if(fd1 < 0){
    //     fprintf(STDERR_FILENO, "Can not open %s", file1);
    //     exit(1);
    // }

    // if(fd2 < 0){
    //     fprintf(STDERR_FILENO, "Can not open %s", file2);
    //     exit(1);
    // }

    fgets(buf, 100, fd1);
    int i = 0;
    while (fgets(buf, 100, fd1))
    {
        char *buf2;
        buf2 = strtok(buf, ",");
        strcpy(appl_stock.date[i], buf2);
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        appl_stock.close[i] = atof(buf2);
        i++;
    }
    fgets(buf, 100, fd2);
    i = 0;
    while (fgets(buf, 100, fd2))
    {
        char *buf2;
        buf2 = strtok(buf, ",");
        strcpy(twtr_stock.date[i], buf2);
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        buf2 = strtok(NULL, ",");
        twtr_stock.close[i] = atof(buf2);
        i++;
    }
}

float getPrice(char *stock, char *date)
{
    int i;

    if (strncmp(stock, APPL_STR, 4) == 0)
    {
        for (i = 0; i < DATA_NUM; i++)
        {
            if (strncmp(appl_stock.date[i], date, 10) == 0)
                return appl_stock.close[i];
        }
    }
    else if (strncmp(stock, TWTR_STR, 4) == 0)
    {
        for (i = 0; i < DATA_NUM; i++)
        {
            if (strncmp(twtr_stock.date[i], date, 10) == 0)
                return twtr_stock.close[i];
        }
    }
    return -1.0;
}

float maxProfit(char *stock)
{
    float max_profit[DATA_NUM];
    float min_close[DATA_NUM];
    int i;
    struct STOCK *s;
    float temp_profit;

    if (strncmp(stock, APPL_STR, 4) == 0)
    {
        s = &appl_stock;
    }
    else if (strncmp(stock, TWTR_STR, 4) == 0)
    {
        s = &twtr_stock;
    }
    else
    {
        return -1.0;
    }

    max_profit[0] = 0;
    min_close[0] = s->close[0];
    for (i = 1; i < DATA_NUM; i++)
    {
        min_close[i] = min_close[i - 1] < s->close[i] ? min_close[i - 1] : s->close[i];
        temp_profit = s->close[i] - min_close[i - 1];
        max_profit[i] = temp_profit > max_profit[i - 1] ? temp_profit : max_profit[i - 1];
    }

    return max_profit[DATA_NUM - 1];
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
    size_t n;
    int i = 0;
    int size;
    char input[MAXLINE];
    float result;
    char *stock;
    char *date;
    char *buffer;
    char *spliter = " \n";
    char output[MAXLINE];

    while ((n = read(connfd, input, MAXLINE)) != 0)
    {
        printf("%s\n", &input[1]);

        buffer = strtok(&input[1], spliter);

        if (strcmp(buffer, PRICES_STR) == 0)
        {
            stock = strtok(NULL, spliter);
            date = strtok(NULL, spliter);
            result = getPrice(stock, date);

            if (result < 0)
            {
                sprintf(output, "%s\n", UNKNOWN_STR);
            }
            else
            {
                sprintf(output, "%.2f\n", result);
            }
        }
        else if (strcmp(buffer, MAXPROFIT_STR) == 0)
        {
            stock = strtok(NULL, spliter);
            stock[4] = '\0';
            result = maxProfit(stock);
            if (result < 0)
            {
                sprintf(output, "%s\n", UNKNOWN_STR);
            }
            else
            {
                sprintf(output, "Maximum Profit for %s: %.2f\n", stock, result);
            }
        }
        size = strlen(output);
        for (i = size; i > 0; i--)
        {
            output[i] = output[i - 1];
        }
        output[0] = size;

        write(connfd, output, strlen(output));
        for (i = 0; i < MAXLINE; i++)
        {
            input[i] = '\0';
            output[i] = '\0';
        }
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
