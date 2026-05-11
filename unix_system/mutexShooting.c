#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_BULLETS 10

pthread_mutex_t mutex; //쓰레드 수행 간에 데이터 보호를 위해 Mutex 사용

typedef struct {
    int x;
    int y;
    int active;
} Bullet; //총알 구조체 정의

Bullet bullets[MAX_BULLETS]; // 총알 구조체 집합
int plane_x, plane_y; //비행기 위치
int target_x, target_y = 79; //초기 타겟 위치
bool hit = FALSE; //타겟 적중 플래그
void* bullet_thread(void* ptr) { //총알 쓰레드 로직 구현
    Bullet* bullet = (Bullet*)ptr;
    int prevY = bullet->y;

    while(bullet->y < COLS) { //ncurses에서는 COLS, LINES로
        pthread_mutex_lock(&mutex);

        mvaddch(bullet->x, prevY, ' ');
        mvaddch(bullet->x, ++bullet->y, '>');
        prevY = bullet->y;

        refresh();
        if(bullet->x==target_x&&bullet->y==target_y){
            hit = true;
            pthread_mutex_unlock(&mutex);
            return;
        }
        pthread_mutex_unlock(&mutex);
        usleep(20 * 1000); //총알 프레임 구현
    }

    pthread_mutex_lock(&mutex);
    mvaddch(bullet->x, prevY, ' ');
    refresh();
    bullet->active = 0;

    pthread_mutex_unlock(&mutex);
}

int main() {
    initscr();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); //입력하지 않아도 자연스럽게 시작
    srand(time(NULL));
    target_x = rand()%LINES;
    plane_x = rand()%LINES;
    plane_y = rand()%COLS;
    for(int i=0; i<MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    while(1) {
        int ch = getch();

        if(hit){// 타겟 적중시 동작
            mvprintw(LINES/2-2, COLS/2-5, "GAME OVER");
            mvprintw(LINES/2+1, COLS/2-7, "PRESS ANY KEY");
            refresh();
            nodelay(stdscr, FALSE); // 종료 대기를 위해 키 입력 대기
            getch();
            break;
        }
        //
        pthread_mutex_lock(&mutex);
        clear();
        mvaddch(target_x, target_y, 'X');
        mvaddch(plane_x, plane_y, 'A');
        switch(ch) {
            case KEY_UP:    if(plane_x > 0){ //비행기 위치 변경
                                mvaddch(plane_x--, plane_y,' ');
                                mvaddch(plane_x, plane_y,'A');
                            }
                            //continue;
                            break;
            case KEY_DOWN:  if(plane_x < COLS){ //비행기 위치 변경
                                mvaddch(plane_x++, plane_y,' ');
                                mvaddch(plane_x, plane_y, 'A');
                            }
                            //continue;
                            break;
            case KEY_LEFT:  if(plane_y > 0){ //비행기 위치 변경
                                mvaddch(plane_x, plane_y--,' ');
                                mvaddch(plane_x, plane_y,'A');
                            }
                            //continue;
                            break;
            case KEY_RIGHT: if(plane_y < LINES){ //비행기 위치 변경
                                mvaddch(plane_x, plane_y++,' ');
                                mvaddch(plane_x, plane_y,'A');
                            }
                            //continue;
                            break;
            case ' ': //총알 (최대 10개) 생성 및 쓰레드 실행
                for(int i=0; i<MAX_BULLETS; i++) {
                    if(!bullets[i].active) { //총알 초기화
                        bullets[i].x = plane_x;
                        bullets[i].y = plane_y + 1;
                        bullets[i].active = 1;
                        pthread_t pthread_bullet; //총알 쓰레드 선언 및 생성
                        pthread_create(&pthread_bullet, NULL, bullet_thread, (void *)&bullets[i]);
                        pthread_detach(pthread_bullet);//비행기 움직이면서 총알도 쏘기 위해서 detach 사용
                        break;
                    }
                }
                //continue;
                break;
            case 'q':// 메인 쓰레드 종료
                endwin();
                pthread_mutex_unlock(&mutex);
                pthread_mutex_destroy(&mutex);
                exit(0);
        }
        refresh(); // 객체들 쉘에 표시
        pthread_mutex_unlock(&mutex);
        usleep(100 * 1000); //게임 프레임 구현
    }

    endwin();
    return 0;
}
