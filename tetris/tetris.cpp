#include "tetris.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <algorithm>

int load_high_score(){
    return 2200;
}

void save_high_score(int score){
    //TODO
}

void init_game(GameState& state) {
    srand(888);
    state.board = std::vector<std::vector<char>>(BOARD_HEIGHT, std::vector<char>(BOARD_WIDTH, '.'));
    state.current_block = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
    state.tetrominoes = {
            // I-tetromino (2 rotations)
            {
                    {
                            {0, 0, 0, 0},
                            {1, 1, 1, 1},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 0, 1, 0},
                            {0, 0, 1, 0},
                            {0, 0, 1, 0},
                            {0, 0, 1, 0}
                    }
            },
            // O-tetromino (1 rotation)
            {
                    {
                            {0, 1, 1, 0},
                            {0, 1, 1, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    }
            },
            // T-tetromino (4 rotations)
            {
                    {
                            {0, 1, 0, 0},
                            {1, 1, 1, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 0, 0},
                            {0, 1, 1, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 0, 0, 0},
                            {1, 1, 1, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 0, 0},
                            {1, 1, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    }
            },
            // S-tetromino (2 rotations)
            {
                    {
                            {0, 1, 1, 0},
                            {1, 1, 0, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 0, 0},
                            {0, 1, 1, 0},
                            {0, 0, 1, 0},
                            {0, 0, 0, 0}
                    }
            },
            // Z-tetromino (2 rotations)
            {
                    {
                            {1, 1, 0, 0},
                            {0, 1, 1, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 0, 1, 0},
                            {0, 1, 1, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    }
            },
            // J-tetromino (4 rotations)
            {
                    {
                            {1, 0, 0, 0},
                            {1, 1, 1, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 1, 0},
                            {0, 1, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 0, 0, 0},
                            {1, 1, 1, 0},
                            {0, 0, 1, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 0, 0},
                            {0, 1, 0, 0},
                            {1, 1, 0, 0},
                            {0, 0, 0, 0}
                    }
            },
            // L-tetromino (4 rotations)
            {
                    {
                            {0, 0, 1, 0},
                            {1, 1, 1, 0},
                            {0, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 1, 0, 0},
                            {0, 1, 0, 0},
                            {0, 1, 1, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {0, 0, 0, 0},
                            {1, 1, 1, 0},
                            {1, 0, 0, 0},
                            {0, 0, 0, 0}
                    },
                    {
                            {1, 1, 0, 0},
                            {0, 1, 0, 0},
                            {0, 1, 0, 0},
                            {0, 0, 0, 0}
                    }
            }
    };

    state.current_tetromino = 0;
    state.current_rotation = 0;
    state.block_x = 0;
    state.block_y = 0;
    state.score = 0; //실제로 점수 증가 구현
    state.level = 1; //레벨 증가 구현
    state.lines_cleared = 0; //지워진 줄 카운트
    state.high_score = load_high_score();
    state.running = true; //게임의 실행 상태
    state.next_tetromino = rand() % state.tetrominoes.size(); //다음 테트로미노 블럭(인덱스)
    state.fall_timer = 0; //
    state.fall_interval = 10 - (state.level - 1);
    if (state.fall_interval < 1) state.fall_interval = 1;
    spawn_new_block(state);
}


void handle_game_action(GameState& state, int action) {
    switch (action) {
        case ACTION_QUIT:
            state.running = false;
            break;
        case ACTION_LEFT:
            move_block(state, -1, 0);
            break;
        case ACTION_RIGHT:
            move_block(state, 1, 0);
            break;
        case ACTION_DOWN:
            move_block(state, 0, 1);
            break;
        case ACTION_ROTATE:
            rotate_block(state);
            break;
        case ACTION_DROP:
            drop_block_to_bottom(state);
            break;
        default:
            break;
    }
}

// Implement your code
bool is_valid_position(const std::vector<std::vector<int>>& block, int x, int y, const std::vector<std::vector<char>>& board){
    for(int i = 0; i < 4; i++){
        int block_y = y+i;
        for(int j = 0; j < 4; j++){
            int block_x = x+j;
            if(block[i][j] == 1){
                if(block_y < 0 || block_y > BOARD_HEIGHT-1
                   || block_x < 0 || block_x > BOARD_WIDTH-1
                   || board[block_y][block_x] == '#') return false;
            }
        }
    }
    return true;
}

void update_game(GameState& state) {
    if (++state.fall_timer >= state.fall_interval) {
        // move block
        // check landing
        move_block(state, 0, 1);
        state.fall_timer = 0;
        state.fall_interval = 10 - (state.level - 1);
        if (state.fall_interval < 1) state.fall_interval = 1;
    }
}

void land_block(GameState& state){
    for(int i = 0; i < 4; i++){
        int y = state.block_y+i;
        for(int j = 0; j < 4; j++){
            int x = state.block_x+j;
            if(state.current_block[i][j] == 1){
                state.board[y][x] = '#';
            }
        }
    }
}

bool move_block(GameState& state, int dx, int dy) {
    int temp_x = state.block_x+dx;
    int temp_y = state.block_y+dy;
    if(is_valid_position(state.current_block, temp_x, temp_y, state.board)){
        state.block_x = temp_x;
        state.block_y = temp_y;
        return true;
    }
    else {
        if(dx == 0 && dy == 1){
            land_block(state);
            spawn_new_block(state);
            check_lines(state);
            return false;
        }
        return false;
    }
//    if(dx == 0 && dy == 1){
//        if(!is_valid_position(state.current_block,temp_x,temp_y,state.board)){
//            land_block(state);
//            check_lines(state);
//            spawn_new_block(state);
//            return false;
//        }
//    }
//    if(is_valid_position(state.current_block, temp_x, temp_y, state.board)){
//        state.block_x = temp_x;
//        state.block_y = temp_y;
//        return true;
//    }
//    else return false;
}

void drop_block_to_bottom(GameState& state) {
    while(move_block(state, 0, 1));
}

void spawn_new_block(GameState& state) {
    state.block_x = BOARD_WIDTH/2-2;
    state.block_y = 0;
    state.current_tetromino = state.next_tetromino;
    state.current_rotation = 0;
    state.current_block = state.tetrominoes[state.current_tetromino][0];
//    state.next_terminfo = rand()%state.tetrominoes.size();
    state.next_tetromino = (state.current_tetromino+1)%state.tetrominoes.size();
    if(!is_valid_position(state.current_block,state.block_x,state.block_y,state.board)){
        state.running = false;
    }
}

void check_lines(GameState& state){
    //check full lines
    // erase them
    // update score
    auto count_full_lines = std::count_if(state.board.begin(), state.board.end(), [](const auto& line){
        return std::all_of(line.begin(), line.end(),[](auto cell){return cell=='#';});});
    auto removed = std::remove_if(state.board.begin(), state.board.end(), [&state] (const auto& line){
        if(std::all_of(line.begin(), line.end(), [state](const auto& cell){return cell=='#';})){
            state.score += 100*state.level;
            state.lines_cleared++;
            state.level = state.lines_cleared/10+1;
            state.fall_interval = 10 - (state.level - 1);
            if (state.fall_interval < 1) state.fall_interval = 1;
            return true;
        }
        return false;
    });
    state.board.erase(removed, state.board.end());
    std::vector<char> new_line(BOARD_WIDTH, '.');
    state.board.insert(state.board.begin(),count_full_lines,new_line);
}

void rotate_block(GameState& state){
    auto next_rotation = (state.current_rotation+1)%state.tetrominoes[state.current_tetromino].size();
    if(is_valid_position(state.tetrominoes[state.current_tetromino][next_rotation],state.block_x, state.block_y, state.board)){
        state.current_block = state.tetrominoes[state.current_tetromino][state.current_rotation = next_rotation];
    }
}
