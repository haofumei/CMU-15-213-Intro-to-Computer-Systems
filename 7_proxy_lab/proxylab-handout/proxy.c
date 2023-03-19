#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"
#include "LRUCache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define NTHREADS 4
#define SBUFSIZE 16

/*************************
 * Value******************
 *************************/

static sbuf_t sbuf; /* Shared buffer of connected descriptors */

/* valuables for readers-writers */
volatile int incnt = 0;
volatile int outcnt = 0;
volatile int wrtwait = 0;
sem_t in, out, wrt; 

static LRUCache *cache;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/*************************
 * Function***************
 *************************/

/* regular function */
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
int parse_uri(char *uri, char *hostname, char *port, char *path);
void forward_server(rio_t *rp, char *method, char *path, 
                      char *hostname, int clientfd);
void forward_client(int clientfd, rio_t *rs, char *key);

/* concurrency */
void *thread(void *vargp);
Node *read_cache(char *key, LRUCache *cache);
void write_cache(char *key, byte *val, int len, LRUCache *cache);

/* signal */
void sigpipe_handler(int signal);


int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* Initiate the sbuf and readers-writers */
    sbuf_init(&sbuf, SBUFSIZE);
    Sem_init(&in, 0, 1);
    Sem_init(&out, 0, 1);
    Sem_init(&wrt, 0, 0);

    /* Initiate the cache */
    cache = initLRUCache(MAX_CACHE_SIZE);

    /* Install the signal handlers */
    Signal(SIGPIPE, sigpipe_handler);

    for (int i = 0; i < NTHREADS; i++) /* create worker threads */
        Pthread_create(&tid, NULL, thread, NULL);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */

        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    }

    freeCache(cache);
}

/*************************
 * Regular Function*******
 *************************/

/*
 * take care one request from client
 */
void doit(int fd)
{
    rio_t rc, rs; /* rc communicate with client, rs communicate with server */
    int serverfd;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], 
         version[MAXLINE], hostname[MAXLINE], path[MAXLINE], port[MAXLINE];
    Node *node;
    
    /* read request first line header */
    Rio_readinitb(&rc, fd);
    if (!Rio_readlineb(&rc, buf, MAXLINE))
        return;
    sscanf(buf, "%s %s %s", method, uri, version);

    /* only support GET method for now */
    if (strcasecmp(method, "GET")) { 
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }

    /* invalid uri */
    if (!parse_uri(uri, hostname, port, path)) { 
        clienterror(fd, method, "400", "Bad request",
                    "Host name is invalid");
        return;
    }
    
    printf("Client request (%s, %s)\n", hostname, port);
    /* if cache exist */
    if ((node = read_cache(buf, cache))) { 
    	printf("Read from cache: %d\n", node->len);
        Rio_writen(fd, node->val, node->len);
        return;
    }

    /* handle page not found */
    if ((serverfd = open_clientfd(hostname, port)) < 0) { 
        clienterror(fd, method, "404", "Not found",
                    "Connection refused");
        return;
    }

    Rio_readinitb(&rs, serverfd);

    forward_server(&rc, method, path, hostname, serverfd);
    forward_client(fd, &rs, buf);
    
    Close(serverfd);
}

/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/*
 * parse uri into hostname, port and path
 * http://www.cmu.edu/hub/index.html -> 
 * http://www.cmu.edu and /hub/index.html
 * http://www.cmu.edu:8080/hub/index.html -> 
 * http://www.cmu.edu, 8080 and /hub/index.html
 */
int parse_uri(char *uri, char *hostname, char *port, char *path)
{
    if (sscanf(uri, "%*[^:]%*[:/]%[^:]:%[^/]%s", hostname, port, path) == 3) { /* with port in the uri */
        return 1;
    } else if (sscanf(uri, "%*[^:]%*[:/]%[^/]%s", hostname, path) == 2) { /* not with port in the uri */
        sprintf(port, "80");
        return 1;
    } else {
        return 0; /* unvalid uri */
    }
}

/*
 * receive the rest of header from client,
 * modify the HTTP header and forward to server
 */
void forward_server(rio_t *rp, char *method, char *path, 
                      char *hostname, int clientfd)
{
    char buf[MAXLINE];
    
    sprintf(buf, "%s %s %s\r\n", method, path, "HTTP/1.0");
    sprintf(buf, "%sHost: %s\r\n", buf, hostname);
    sprintf(buf, "%sUser-Agent: %s", buf, user_agent_hdr);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sProxy-Connection: close\r\n", buf);

    Rio_writen(clientfd, buf, strlen(buf)); /* write the fixed part of header to origin server */

    /* read the rest of header */
    Rio_readlineb(rp, buf, MAXLINE);         
    while (strcmp(buf, "\r\n")) {
        if (!strstr(buf, "Host") && !strstr(buf, "User-Agent") &&
                !strstr(buf, "Connection") && !strstr(buf, "Proxy-Connection")) {
            Rio_writen(clientfd, buf, strlen(buf));
        }
        Rio_readlineb(rp, buf, MAXLINE);
    }
    strcpy(buf, "\r\n"); /* new line indicates end of header */
    Rio_writen(clientfd, buf, strlen(buf));
}

/*
 * receive the response from server
 * forward to the origin client 
 */
void forward_client(int clientfd, rio_t *rs, char *key)
{
    byte buf[MAX_OBJECT_SIZE];
    int size, sum = 0;

    while ((size = Rio_readnb(rs, buf, MAX_OBJECT_SIZE)) != 0) {
        if (size < 0 && errno == EPIPE) { /* handle SIGPIPE */
            printf("Socket closed by remote host\n");
            break;
        } else {
            Rio_writen(clientfd, buf, size);
            sum += size;
        }
    }

    if (sum <= MAX_OBJECT_SIZE) { /* store in the cache */
        write_cache(key, buf, sum, cache);
        printf("Write %d into cache\n and size is %d\n", sum, cache->size);
        printCache(cache);
    }
}

/*************************
 * Concurrency************
 *************************/

/*
 * thread routine for dealing socket on the sbuf
 */
void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    while(1) {
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
    }
    return NULL;
}


/*
 * Read val from cache with given header
 */
Node *read_cache(char *key, LRUCache *cache)
{
    /* Create the header */
    Node *node;

    P(&in);
    incnt++;
    V(&in);
    /* Critical section */
    node = get(cache, key);
    P(&out);
    outcnt++;
    if (wrtwait == 1 && incnt == outcnt)
        V(&wrt);
    V(&out);
    return node;
}

/*
 * Write the val with given key into cache
 */
void write_cache(char *key, byte *val, int len, LRUCache *cache)
{
    P(&in);
    P(&out);
    if (incnt == outcnt) {
        V(&out);
    } else {
        wrtwait = 1;
        V(&out);
        P(&wrt);
        wrtwait = 0;
    }
    /* Critical section for writer */
    put(cache, key, val, len);
    V(&in);
}

/*************************
 * Signal*****************
 *************************/

void sigpipe_handler(int signal) {
    printf("Received SIGPIPE signal\n");
}
