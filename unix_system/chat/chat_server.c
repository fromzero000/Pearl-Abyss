#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "chat_common.h"

volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    running = 0;
}

int main() {
    signal(SIGINT, handle_sigint);

    // 공유 메모리 생성
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, sizeof(ChatSharedMemory));
    ChatSharedMemory *shm = mmap(NULL, sizeof(ChatSharedMemory),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // 세마포어 및 데이터 초기화
    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->server_sem, 1, 0);
    for (int i = 0; i < MAX_USERS; i++) {
        sem_init(&shm->client_sem[i], 1, 0);
        shm->users[i].active = false;
    }
    shm->msg_count = 0;
    shm->user_count = 0;

    printf("=== 채팅 서버 실행 중 ===\n");
    printf("Ctrl+C로 종료\n");

    // 서버 메시지 브로드캐스트 루프
    while (running) {
        sem_wait(&shm->server_sem);

        sem_wait(&shm->mutex);
        int latest_msg = shm->msg_count - 1;
        if (latest_msg >= 0) {
            // 모든 활성 클라이언트에 알림
            for (int i = 0; i < MAX_USERS; i++) {
                if (shm->users[i].active) {
                    sem_post(&shm->client_sem[i]);
                }
            }
        }
        sem_post(&shm->mutex);
    }

    // 정리
    munmap(shm, sizeof(ChatSharedMemory));
    shm_unlink(SHM_NAME);
    printf("\n서버 종료\n");
    return 0;
}


