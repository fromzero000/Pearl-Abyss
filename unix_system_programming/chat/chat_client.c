#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include <pthread.h>
#include <wchar.h>
#include <time.h>
#include "chat_common.h"

#define COLOR_BASE 1

int my_id = -1;
ChatSharedMemory *shm = NULL;
WINDOW *chat_win, *input_win, *user_win;
int chat_lines = 0;

// 색상 초기화
void init_colors() {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
}

// 유저별 색상 선택
int pick_color(int idx) {
    return COLOR_BASE + (idx % 5);
}

// 유저 목록 창 갱신
void update_user_win() {
    werase(user_win);
    box(user_win, 0, 0);
    mvwprintw(user_win, 0, 1, " Users ");
    int row = 1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm->users[i].active) {
            wattron(user_win, COLOR_PAIR(shm->users[i].color_pair));
            mvwaddwstr(user_win, row++, 1, shm->users[i].nickname);
            wattroff(user_win, COLOR_PAIR(shm->users[i].color_pair));
        }
    }
    wrefresh(user_win);
}

// 메시지 출력
void print_message(const Message *msg) {
    char timebuf[20];
    struct tm *tm_info = localtime(&msg->timestamp);
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

    wattron(chat_win, COLOR_PAIR(shm->users[msg->user_id].color_pair));
    wprintw(chat_win, "[%s][", timebuf);
    waddwstr(chat_win, shm->users[msg->user_id].nickname);
    wprintw(chat_win, "] ");
    waddwstr(chat_win, msg->content);
    wprintw(chat_win, "\n");
    wattroff(chat_win, COLOR_PAIR(shm->users[msg->user_id].color_pair));
    wrefresh(chat_win);
    chat_lines++;
    if (chat_lines > LINES - 5) {
        wscrl(chat_win, 1);
        chat_lines--;
    }
}

// 메시지 수신 스레드
void *recv_thread_func(void *arg) {
    int last_msg = 0;
    while (1) {
        sem_wait(&shm->client_sem[my_id]);
        sem_wait(&shm->mutex);
        for (; last_msg < shm->msg_count; last_msg++) {
            print_message(&shm->messages[last_msg]);
        }
        update_user_win();
        sem_post(&shm->mutex);
    }
    return NULL;
}

// 유저 등록
int register_user() {
    wchar_t nickname[NICKNAME_LEN];
    echo();
    mvwprintw(input_win, 1, 1, "닉네임 입력: ");
    wrefresh(input_win);
    wgetn_wstr(input_win, nickname, NICKNAME_LEN - 1);
    noecho();

    sem_wait(&shm->mutex);
    int idx = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!shm->users[i].active) {
            idx = i;
            wcsncpy(shm->users[i].nickname, nickname, NICKNAME_LEN);
            shm->users[i].nickname[NICKNAME_LEN - 1] = L'\0';
            shm->users[i].color_pair = pick_color(i);
            shm->users[i].active = true;
            shm->user_count++;
            break;
        }
    }
    sem_post(&shm->mutex);
    return idx;
}

// 유저 로그아웃
void unregister_user() {
    if (my_id >= 0) {
        sem_wait(&shm->mutex);
        shm->users[my_id].active = false;
        shm->user_count--;
        sem_post(&shm->mutex);
    }
}

// 창 초기화
void init_windows() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    chat_win = newwin(rows - 4, cols - 20, 0, 0);
    user_win = newwin(rows - 4, 20, 0, cols - 20);
    input_win = newwin(4, cols, rows - 4, 0);
    scrollok(chat_win, TRUE);
    box(user_win, 0, 0);
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 1, " 메시지 입력 ");
    wrefresh(chat_win);
    wrefresh(user_win);
    wrefresh(input_win);
}

void cleanup() {
    unregister_user();
    endwin();
    munmap(shm, sizeof(ChatSharedMemory));
}

// 메인
int main() {
    setlocale(LC_ALL, "");
    // 공유 메모리 연결
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("서버가 실행 중인지 확인하세요.\n");
        exit(1);
    }
    shm = mmap(NULL, sizeof(ChatSharedMemory),
               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_colors();
    init_windows();

    // 유저 등록
    my_id = register_user();
    if (my_id < 0) {
        mvwprintw(input_win, 1, 1, "채팅방이 가득 찼습니다. 종료합니다.");
        wrefresh(input_win);
        sleep(2);
        cleanup();
        exit(0);
    }
    update_user_win();

    // 수신 스레드 시작
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_thread_func, NULL);

    // 메시지 입력 루프
    wchar_t input[MSG_LEN];
    while (1) {
        werase(input_win);
        box(input_win, 0, 0);
        mvwprintw(input_win, 0, 1, " 메시지 입력 (이모지: 복사/붙여넣기) ");
        mvwprintw(input_win, 1, 1, "> ");
        wrefresh(input_win);

        wgetn_wstr(input_win, input, MSG_LEN - 1);
        input[MSG_LEN - 1] = L'\0';
        if (wcscmp(input, L"/quit") == 0) {
            break;
        }

        sem_wait(&shm->mutex);
        if (shm->msg_count < MAX_MSG) {
            Message *msg = &shm->messages[shm->msg_count++];
            wcsncpy(msg->content, input, MSG_LEN);
            msg->content[MSG_LEN - 1] = L'\0';
            msg->timestamp = time(NULL);
            msg->user_id = my_id;
        }
        sem_post(&shm->server_sem);
        sem_post(&shm->mutex);
    }

    cleanup();
    return 0;
}
