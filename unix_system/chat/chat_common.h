#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include <semaphore.h>
#include <stdbool.h>
#include <time.h>
#include <wchar.h>

#define MAX_USERS 5
#define MAX_MSG 100
#define MSG_LEN 200
#define NICKNAME_LEN 32
#define SHM_NAME "/chat_shm"

// 사용자 정보 구조체
typedef struct {
    wchar_t nickname[NICKNAME_LEN];
    int color_pair;
    bool active;
} UserInfo;

// 메시지 정보 구조체
typedef struct {
    wchar_t content[MSG_LEN]; // 유니코드 문자열
    time_t timestamp;
    int user_id;           // 보낸 사용자 인덱스
} Message;

// 공유 메모리 구조체
typedef struct {
    sem_t mutex;                    // 공유 메모리 접근 동기화
    sem_t server_sem;               // 서버 알림용 세마포어
    sem_t client_sem[MAX_USERS];    // 클라이언트별 세마포어

    UserInfo users[MAX_USERS];      // 사용자 정보 배열
    Message messages[MAX_MSG];      // 메시지 배열

    int msg_count;                  // 메시지 개수
    int user_count;                 // 현재 접속자 수
} ChatSharedMemory;

#endif // CHAT_COMMON_H
