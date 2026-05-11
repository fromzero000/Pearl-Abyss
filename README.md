# C++ & System Programming Portfolio

C++ 및 시스템 프로그래밍(Linux/Windows) 역량 향상을 위해 진행한 프로젝트들의 소스 코드 저장소입니다. 상용 엔진의 기능에 의존하기보다, 언어의 표준 기능(STL)과 OS 레벨의 API를 직접 사용하여 프로그램의 동작 원리를 파악하는 것에 중점을 두었습니다.

## 📂 Project List

### 1. tetris_ncurses (C++ / Ncurses)
* **내용:** 터미널 환경에서 작동하는 테트리스 게임
* **주요 포인트:**
  * `std::chrono`를 활용한 델타 타임 기반의 게임 루프 구현
  * `std::remove_if`와 람다를 활용한 Erase-Remove 관용구 적용 (컨테이너 인덱스 무효화 해결)

### 2. unix_system (C / POSIX API)
* **내용:** 리눅스 환경에서의 시스템 프로그래밍 학습 기록
* **주요 포인트:**
  * `pthread`와 Mutex를 이용한 멀티스레드 동기화 제어
  * `mmap` 및 Semaphore를 활용한 프로세스 간 통신(IPC) 구현

### 3. winsock_gomoku (C++ / Winsock API)
* **내용:** TCP/IP 소켓 통신 기반의 네트워크 오목 게임
* **주요 포인트:**
  * Winsock API를 활용한 클라이언트-서버 구조 설계
  * 네트워크 바이트 오더링(`htonl`, `ntohl`) 처리를 통한 데이터 직렬화

### 4. db_manager (C++ / PostgreSQL)
* **내용:** `libpqxx`를 이용한 C++와 데이터베이스 연동 프로젝트
* **주요 포인트:**
  * RAII 패턴을 적용하여 트랜잭션의 안전한 시작과 종료 관리
  * 예외 상황 발생 시 객체 소멸자를 통한 자동 롤백 구현

---
* **개발 환경:** VMWare Ubuntu, MSYS2 MinGW64, Vim, Git
