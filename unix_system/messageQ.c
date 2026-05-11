#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <mqueue.h>
char msgQName[100] = {'\0',};

void server_function(){
    struct mq_attr attr;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    char result[100];

    mqd_t mfd;
    mfd = mq_open(msgQName, O_RDWR | O_CREAT | O_EXCL, 0644, &attr);

    int j = 1;
    if(mfd == -1)
    {
        perror("Server mq open error\n");
        exit(0);
    }
    while(j < 11)
    {
        if((mq_receive(mfd, result, attr.mq_msgsize, NULL)) == -1)
        {
            perror("Server Receive error\n");
            exit(-1);
        }
        printf("[Recv] %s\n", result);
        j++;
    }

    mq_close(mfd);
}
void client_function()
{
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 10;

    char buf[100] = {'\0',};

    mqd_t mfd;
    mfd = mq_open(msgQName, O_WRONLY, 0644, &attr);

    int i = 1;
    if(mfd==-1)
    {
        perror("Client mq open error\n");
    }

    while(i < 11)
    {
        sprintf(buf,"Msg %d", i);
        if((mq_send(mfd, buf, attr.mq_msgsize, 1)) == -1)
        {
            perror("Client Send error\n");
            exit(-1);
        }
        printf("[Send] %s\n", buf);
        i++;
    }
    mq_close(mfd);
}

int main(int argc, char** argv)
{
    char name[L_tmpnam];
    char *ptr = tmpnam(name);
    ptr = "/tmp/HW04_messageQ";
    const char *msgQname = "/HW04_messageQ";
    sprintf(msgQname, "/%s",ptr+5);


    int32_t pid = fork();

    if(pid > 0)
    {
        sleep(5);
        //printf("Parent Process: %d, %d\n", getpid(), pid);
        server_function();
    }
    else if(pid == 0)
    {
        //printf("Parent Process: %d, %d\n", getpid(), pid);
        client_function();
    }
    else if(pid == -1)
    {
        perror("fork error: ");
        exit(0);
    }

    return 0;
}
