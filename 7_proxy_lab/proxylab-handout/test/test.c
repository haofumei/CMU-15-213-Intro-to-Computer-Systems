#include <stdio.h>
#include "../csapp.h"

int parse_uri(char *uri, char *hostname, char *port, char *path);

int main(int argc, char **argv)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], 
         version[MAXLINE], hostname[MAXLINE], path[MAXLINE], port[MAXLINE];
   

    sprintf(uri, "http://www.cmu.edu/hub/index.html");
    //sprintf(uri, "http://www.cmu.edu:8080/hub/index.html");
    sprintf(buf, NULL);
    if (parse_uri(uri, hostname, port, path))
        printf("%s\n%s\n%s\n", hostname, port, path);
    else {
        printf("error");
    }
    printf("%ld", strlen(buf));
    
}

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

