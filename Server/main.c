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
#define DATA_NUM 503
#define DATE_LEN 11
#define UNKNOWN_STR "Unknown"

struct STOCK
{
    char date[DATA_NUM][DATE_LEN];
    float close[DATA_NUM];
    float low[DATA_NUM];
    float high[DATA_NUM];
};

struct STOCK PFE_stock;
struct STOCK MRNA_stock;

// initialize the stock data

char **split_data(char *line)
{
    int token_length = 0;
    int capacity = 16;

    char **tokens = malloc(capacity * sizeof(char *));

    char *delimiters = ",";
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

char **split_line(char *line)
{
    int token_length = 0;
    int capacity = 16;

    char **tokens = malloc(capacity * sizeof(char *));

    char *delimiters = "\0";
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
    char line[256];

    int count_1 = 0;
    int count_2 = 0;
    fgets(line, 100, fd1);
    while (fgets(line, 100, fd1))
    {
        char **tokens = split_data(line);
        strcpy(PFE_stock.date[count_1], tokens[0]);
        PFE_stock.close[count_1] = atof(tokens[4]);
        count_1++;
    }
    fgets(line, 100, fd2);
    while (fgets(line, 100, fd2))
    {
        char **tokens1 = split_data(line);
        strcpy(MRNA_stock.date[count_2], tokens1[0]);
        MRNA_stock.close[count_2] = atof(tokens1[4]);
        count_2++;
    }
}

void revert_str(char *date)
{
    // convert 2020-01-01 to 01/01/2020 and eliminate the 0 in front of the month and day
    char *year = strtok(date, "-");
    char *month = strtok(NULL, "-");
    char *day = strtok(NULL, "-");
    if (month[0] == '0')
    {
        month[0] = month[1];
        month[1] = '\0';
    }
    if (day[0] == '0')
    {
        day[0] = day[1];
        day[1] = '\0';
    }
    char *new_date = malloc(10 * sizeof(char));
    strcpy(new_date, month);
    strcat(new_date, "/");
    strcat(new_date, day);
    strcat(new_date, "/");
    strcat(new_date, year);
    strcpy(date, new_date);
}

float getPrice(char *stock, char *date)
{
    printf("stock: %s, date: %s\n", stock, date);
    if (!strcmp(stock, "MRNA"))
    {
        printf("reach here\n");
        for (int i = 0; i < DATA_NUM; i++)
        {
            // match the date
            if (!strcmp(MRNA_stock.date[i], date))
            {
                printf("reach here!!!!!!\n");
                printf("%d  %f\n", i, MRNA_stock.close[i]);
                // copy new_date to date
                printf("%s\n", date);
                return MRNA_stock.close[i];
            }
        }
    }
    else if (!strcmp(stock, "PFE"))
    {
        for (int i = 0; i < DATA_NUM; i++)
        {
            // match the date
            if (!strcmp(PFE_stock.date[i], date))
            {
                return PFE_stock.close[i];
            }
        }
    }
    return 0;
}

int start_index;
int end_index;

float max(float num1, float num2)
{
    if (num1 > num2)
    {
        return num1;
    }
    return num2;
}

float min(float num1, float num2)
{
    if (num1 < num2)
    {
        return num1;
    }
    return num2;
}

void find_index(char *stock, char *start_date, char *end_date)
{
    start_index = 0;
    end_index = 0;
    if (!strcmp(stock, "MRNA"))
    {
        for (int i = 0; i < DATA_NUM; i++)
        {
            if (!strcmp(MRNA_stock.date[i], start_date))
            {
                start_index = i;
            }
            if (!strcmp(MRNA_stock.date[i], end_date))
            {
                end_index = i;
            }
        }
    }
    else if (!strcmp(stock, "PFE"))
    {
        for (int i = 0; i < DATA_NUM; i++)
        {
            if (!strcmp(PFE_stock.date[i], start_date))
            {
                start_index = i;
            }
            if (!strcmp(PFE_stock.date[i], end_date))
            {
                end_index = i;
            }
        }
    }
}
float MinProfit(float arr[], int left, int right)
{
    float result;
    float max_ = 0;
    float min_ = 999;
    if (right <= left)
    {
        return 0;
    }
    int mid = (left + right) / 2;
    for (unsigned int x = mid + 1; x <= right; x++)
    {
        if (arr[x] < min_)
        {
            min_ = arr[x];
        }
    }
    for (unsigned int x = left; x <= mid; x++)
    {
        if (arr[x] > max_)
        {
            max_ = arr[x];
        }
    }
    result = min_ - max_;
    printf("result: %f    left:%d    right: %d\n", result, left, right);
    float max_value, max_value2;
    max_value2 = min(result, MinProfit(arr, left, mid));
    max_value = min(max_value2, MinProfit(arr, mid + 1, right));
    printf("min_value: %f\n  ", max_value);
    return max_value;
}

float MaxProfit(float arr[], int left, int right)
{
    float result;
    float max_ = 0;
    float min_ = 999;
    if (right <= left )
    {
        return 0;
    }
    int mid = (left + right) / 2;
    for (unsigned int x = mid+1; x <= right ; x++)
    {
        if (arr[x] > max_)
        {
            max_ = arr[x];
        }
    }
    for (unsigned int x = left; x <= mid ; x++)
    {
        if (arr[x] < min_)
        {
            min_ = arr[x];
        }
    }
    result = max_ - min_;
    printf("result: %f    left:%d    right: %d\n", result,left,right);
    float max_value, max_value2;
    max_value2 = max(result, MaxProfit(arr, left, mid));
    max_value = max(max_value2, MaxProfit(arr, mid+1,right));
    printf("max_value: %f\n  ", max_value);
    return max_value;
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
    char *date;
    char *buffer;
    char *spliter = " \n";
    char output[MAXLINE];
    float mrna_price;
    float pfe_price;

    while ((n = read(connfd, input, MAXLINE)) != 0)
    {
        printf("%s", &input[1]);

        buffer = strtok(&input[1], spliter);
        printf("%s", buffer);

        if (strcmp(buffer, "PricesOnDate") == 0)
        {
            printf("reach here\n");
            date = strtok(NULL, spliter);
            revert_str(date);
            printf("%s\n", date);
            mrna_price = getPrice("MRNA", date);
            pfe_price = getPrice("PFE", date);
            // PFE: 187.18 | MRNA: 44.98
            sprintf(output, "PFE: %.2f | MRNA: %.2f", pfe_price, mrna_price);
        }
        else if (strcmp(buffer, "MaxPossible") == 0)
        {
            char *flag = strtok(NULL, spliter);
            char *stock = strtok(NULL, spliter);
            char *start_date = strtok(NULL, spliter);
            char *end_date = strtok(NULL, spliter);
            revert_str(start_date);
            revert_str(end_date);

            if (!strcmp(flag, "profit"))
            {
                if (!strcmp(stock, "MRNA"))
                {
                    find_index(stock, start_date, end_date);
                    float arr[end_index - start_index + 1];
                    for(int i = start_index; i <= end_index; i++)
                    {
                        arr[i-start_index] = MRNA_stock.close[i];
                    }
                    //print arr
                    for(int i = 0; i < end_index - start_index + 1; i++)
                    {
                        printf("%f ", arr[i]);
                    }
                    // result = max_profit(stock, start_date, end_date);
                    result = MaxProfit(arr, 0, end_index - start_index);
                    sprintf(output, "%.2f", result);
                }
                else if (!strcmp(stock, "PFE"))
                {
                    find_index(stock, start_date, end_date);
                    printf("start_index: %d    end_index: %d\n", start_index, end_index);
                    float arr[end_index - start_index + 1];
                    for(int i = start_index; i <= end_index; i++)
                    {
                        arr[i-start_index] = PFE_stock.close[i];
                    }
                    //copy the close price to an array
                    // result = max_profit(stock, start_date, end_date);
                    result = MaxProfit(arr, 0, end_index - start_index);
                    sprintf(output, "%.2f", result);
                }
            }
            else if (!strcmp(flag, "loss"))
            {
                if (!strcmp(stock, "MRNA"))
                {
                    find_index(stock, start_date, end_date);
                    printf("start_index: %d    end_index: %d\n", start_index, end_index);
                    float arr[end_index - start_index + 1];
                    for (int i = start_index; i <= end_index; i++)
                    {
                        arr[i - start_index] = MRNA_stock.close[i];
                    }
                    // copy the close price to an array
                    //  result = max_profit(stock, start_date, end_date);
                    result = - MinProfit(arr, 0, end_index - start_index);
                    sprintf(output, "%.2f", result);
                }
                else if (!strcmp(stock, "PFE"))
                {
                    find_index(stock, start_date, end_date);
                    printf("start_index: %d    end_index: %d\n", start_index, end_index);
                    float arr[end_index - start_index + 1];
                    for (int i = start_index; i <= end_index; i++)
                    {
                        arr[i - start_index] = PFE_stock.close[i];
                    }
                    // copy the close price to an array
                    //  result = max_profit(stock, start_date, end_date);
                    result =  - MinProfit(arr, 0, end_index - start_index);
                    sprintf(output, "%.2f", result);
                }
            }
            sprintf(output, "%.2f", result);
        }
        char original_length = strlen(output);
        char buf3[MAXLINE];
        buf3[0] = original_length; // set first byte to size
        buf3[1] = '\0';
        strcat(buf3, output);
        write(connfd, buf3, strlen(buf3));
        memset(output, 0, sizeof(output));
    }
}

int main(int argc, const char *argv[])
{
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
