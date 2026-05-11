#ifndef UI_H
#define UI_H

#include "tetris.h"

void ui_init();
void ui_cleanup(const GameState& state);
void ui_draw(const GameState& state);
int ui_get_input();

#endif // UI_H