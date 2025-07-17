#ifndef GUI_COLOR_EYEDROPPER_H
#define GUI_COLOR_EYEDROPPER_H

#include "raylib.h"

typedef struct {

    bool colorPickerActive;
    bool hexColorEditModeRed, hexColorEditModeGreen, hexColorEditModeBlue, hexColorEditModeAlpha;

    Color* color;
    Color tempValue;
    int red, green, blue, alpha;

} GuiColorEyedropperState;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

GuiColorEyedropperState InitGuiColorEyedropper(Color* color);
int GuiColorEyedropper(Rectangle eyedropperButtonBounds, Rectangle hexColorRedTexBoxBounds, Rectangle hexColorGreenTexBoxBounds,
     Rectangle hexColorBlueTexBoxBounds, Rectangle hexColorAlphaTexBoxBounds, GuiColorEyedropperState* state, Image* imageToPickColorFrom, Vector2 (*mousePosToImagePos)(int, int));

#ifdef __cplusplus
}
#endif

#endif // GUI_COLOR_EYEDROPPER_H

#ifdef GUI_COLOR_EYEDROPPER_IMPLEMENTATION

#include "raygui.h"

GuiColorEyedropperState InitGuiColorEyedropper(Color* color) {
    GuiColorEyedropperState state = {0};
    state.color = color;
    state.tempValue = *color;
    state.red = state.tempValue.r;
    state.green = state.tempValue.g;
    state.blue = state.tempValue.b;
    state.alpha = state.tempValue.a;
    state.colorPickerActive = false;
    state.hexColorEditModeRed = false;
    state.hexColorEditModeGreen = false;
    state.hexColorEditModeBlue = false;
    state.hexColorEditModeAlpha = false;
    return state;
}

int GuiColorEyedropper(Rectangle eyedropperButtonBounds, Rectangle hexColorRedTexBoxBounds, Rectangle hexColorGreenTexBoxBounds,
     Rectangle hexColorBlueTexBoxBounds, Rectangle hexColorAlphaTexBoxBounds, GuiColorEyedropperState* state, Image* imageToPickColorFrom, Vector2 (*mousePosToImagePos)(int, int)) {
    int result = 0;

    if (state->colorPickerActive) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            state->colorPickerActive = false;
            state->tempValue = *state->color;
        } else {
            Vector2 pos = mousePosToImagePos(GetMouseX(), GetMouseY());
            state->tempValue = pos.x < imageToPickColorFrom->width && pos.y < imageToPickColorFrom->height && pos.x >= 0 && pos.y >= 0 ?
                GetImageColor(*imageToPickColorFrom, pos.x, pos.y) : BLANK;
            DrawRing((Vector2){GetMouseX(), GetMouseY()}, 25.f, 35.f, 0.f, 360.f, 4, state->tempValue);
            DrawRingLines((Vector2){GetMouseX(), GetMouseY()}, 25.f, 35.f, 0.f, 360.f, 4, BLACK);
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                state->colorPickerActive = false;
                *state->color = state->tempValue;
            }
        }
    }
    
    bool wasLocked = GuiIsLocked();
    if (state->colorPickerActive)
        GuiUnlock();
    if (GuiButton(eyedropperButtonBounds, "#27#")) {
        state->colorPickerActive = !state->colorPickerActive;
        result = 1;
    }
    if (wasLocked)
        GuiLock();

    if (!(state->hexColorEditModeRed | state->hexColorEditModeGreen | state->hexColorEditModeBlue | state->hexColorEditModeAlpha)) {
        state->red = state->tempValue.r;
        state->green = state->tempValue.g;
        state->blue = state->tempValue.b;
        state->alpha = state->tempValue.a;
    }
    if (GuiValueBox(hexColorRedTexBoxBounds, "R:", &state->red, 0, 255, state->hexColorEditModeRed)) {
        state->hexColorEditModeRed = !state->hexColorEditModeRed;
        if (!state->hexColorEditModeRed) {
            state->tempValue.r = state->red;
            *state->color = state->tempValue;
        }
        result = 2;
    } else if (GuiValueBox(hexColorGreenTexBoxBounds, "G:", &state->green, 0, 255, state->hexColorEditModeGreen)) {
        state->hexColorEditModeGreen = !state->hexColorEditModeGreen;
        if (!state->hexColorEditModeGreen) {
            state->tempValue.g = state->green;
            *state->color = state->tempValue;
        }
        result = 3;
    } else if (GuiValueBox(hexColorBlueTexBoxBounds, "B:", &state->blue, 0, 255, state->hexColorEditModeBlue)) {
        state->hexColorEditModeBlue = !state->hexColorEditModeBlue;
        if (!state->hexColorEditModeBlue) {
            state->tempValue.b = state->blue;
            *state->color = state->tempValue;
        }
        result = 4;
    } else if (GuiValueBox(hexColorAlphaTexBoxBounds, "A:", &state->alpha, 0, 255, state->hexColorEditModeAlpha)) {
        state->hexColorEditModeAlpha = !state->hexColorEditModeAlpha;
        if (!state->hexColorEditModeAlpha) {
            state->tempValue.a = state->alpha;
            *state->color = state->tempValue;
        }
        result = 5;
    }
    return result;
}

#endif