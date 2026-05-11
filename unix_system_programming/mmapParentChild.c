#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <stdlib.h>

int main()
{
    size_t length = 4096;
    void *shared_memory = mmap(NULL, length, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if(shared_memory == MAP_FAILED){
        perror("mmap failed");
        return 1;
    }

    pid_t pid = fork();

    if(pid == -1) {
        perror("fork failed");
        munmap(shared_memory, length);
        return 1;
    }else if(pid == 0) {
        strcpy((char *)shared_memory, "2 + 3\n3 + 4\n25 * 6\n4 - 3\n10 / 5");
        //printf("Child wrote to share memory.\n");
    }else {
        wait(NULL);
        char *token = strtok((char *)shared_memory, "\n");
        while(token != NULL){
            int a, b, result;
            char operator;
            sscanf(token, "%d %c %d", &a, &operator, &b);
            switch(operator){
                case '+': result = a + b;break;
                case '-': result = a - b;break;
                case '*': result = a * b;break;
                case '/': result = a / b;break;
                default: break;
            }
            printf("%d\n", result);
            token = strtok(NULL,"\n");
        }
    }
    munmap(shared_memory, length);
    return 0;
}
