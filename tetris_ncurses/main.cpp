#include "ui.h"
#include <chrono>
#include <thread>
#include <ncurses.h>



int main() {
    GameState state;
    ui_init();
    init_game(state);
    
    const auto frame_duration = std::chrono::milliseconds(50);
    auto last_frame_time = std::chrono::steady_clock::now();
    
    while (state.running) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - last_frame_time;
        
        if (elapsed >= frame_duration) {
            ui_draw(state);
            int action = ui_get_input();
            handle_game_action(state, action);
            update_game(state);
            last_frame_time = current_time;
        } else {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
    }
    nodelay(stdscr,FALSE);
    getch();

    ui_cleanup(state);
    return 0;
}
