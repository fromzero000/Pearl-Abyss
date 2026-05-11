# 💻 Core System Programming Portfolio
> **"상용 엔진의 블랙박스에 의존하지 않고, 로우 레벨의 동작 원리를 직접 통제합니다."**

안녕하세요. C++의 강력한 성능과 운영체제의 하부 구조를 깊이 탐구하는 예비 게임 플레이 프로그래머입니다. 
이 저장소는 편리한 프레임워크나 상용 게임 엔진에 기대지 않고, **C++ STL, POSIX API, 소켓 통신 등을 활용해 바닥부터 직접 구현하며 겪은 트러블슈팅과 최적화의 기록**입니다.

## 🛠 Tech Stack
- **Language:** C++ (C++11/14/17), C
- **OS & Environment:** VMWare Ubuntu (Linux), Windows
- **System & Network:** POSIX API (pthreads, IPC), Winsock API
- **Tools:** Vim, MSYS2 MinGW64, Code::Blocks, Git

---

## 📂 Core Projects & Troubleshooting

### 1. [C++] Tetris with Ncurses & STL (자체 게임 루프 구현)
**📌 핵심 요약:** `ncurses`를 이용한 렌더링 최적화 및 C++ STL을 활용한 중력 로직 버그 해결
- **문제:** 라인 삭제 시 `std::vector`의 순회 중 요소 삭제로 인한 **인덱스 무효화(Iterator Invalidation)** 발생.
- **해결 및 최적화:** - `std::remove_if`와 `erase`를 결합한 **Erase-Remove 관용구**를 적용하여 메모리 레벨에서 안전하게 삭제.
  - 내부 조건자로 람다(Lambda)를 활용하여 줄 검사와 점수 갱신, 속도 증가를 단일 파이프라인으로 통합.
  - 람다 캡처 시 불필요한 구조체 복사를 방지하기 위해 캡처 리스트를 비워(`[]`) 오버헤드 최적화.

```cpp
// 💡 핵심 로직: Erase-Remove 관용구와 람다를 활용한 최적화
auto removed = std::remove_if(state.board.begin(), state.board.end(), [](const auto& line) {
    return std::all_of(line.begin(), line.end(), [](const auto& cell){ return cell == '#'; });
});
// 이후 로직에서 state 점수 갱신 및 빈 라인 최상단 insert 처리