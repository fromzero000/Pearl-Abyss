#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <vector>

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

// 게임 액션 정의
#define ACTION_QUIT      1
#define ACTION_LEFT      2
#define ACTION_RIGHT     3
#define ACTION_DOWN      4
#define ACTION_ROTATE    5
#define ACTION_DROP      6
#define ACTION_NONE      0

// 게임 상태 구조체 (필요한 것만 사용)
struct GameState {
    std::vector<std::vector<char>> board;
    std::vector<std::vector<int>> current_block;
    std::vector<std::vector<std::vector<std::vector<int>>>> tetrominoes;
    int current_tetromino;
    int current_rotation;
    int block_x;
    int block_y;
    int score;              // 2200
    int level;              // 1
    int lines_cleared;
    int high_score;
    bool running;
    int next_tetromino;
    int fall_timer;
    int fall_interval;
};

// public
void init_game(GameState& state);
void handle_game_action(GameState& state, int action);
void update_game(GameState& state);
bool move_block(GameState& state, int dx, int dy);
void spawn_new_block(GameState& state);
void drop_block_to_bottom(GameState& state);

void check_lines(GameState& state);
bool is_valid_position(const std::vector<std::vector<int>>& block, int x, int y, const std::vector<std::vector<char>>& board);
void rotate_block(GameState& state);
void land_block(GameState& state);
//int load_high_score();
//void save_high_score(int score);

#endif
