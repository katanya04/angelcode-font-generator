#include "gui_helper.h"
#include "raygui.h"
#define GUI_COLOR_EYEDROPPER_IMPLEMENTATION
#include "gui_color_eyedropper.h"
#undef GUI_COLOR_EYEDROPPER_IMPLEMENTATION

Vector2 prev_win_size = {800.f, 450.f};

void generate_transparent_pattern() {
    UnloadTexture(transparent_pattern);
    int square_size = (GetScreenHeight() + GetScreenWidth()) / 2 / 50;
    Image square_pattern_image = GenImageChecked(GetScreenWidth(), GetScreenHeight(), square_size, square_size, LIGHTGRAY, WHITE);
    transparent_pattern = LoadTextureFromImage(square_pattern_image);
    UnloadImage(square_pattern_image);
}

void add_zoom(float delta_zoom, Vector2 center_cords) {
    Vector2 center_relative_to_image = {(center_cords.x - draw_pos.x) / zoom, (center_cords.y - draw_pos.y) / zoom};
    draw_pos.x -= delta_zoom * center_relative_to_image.x;
    draw_pos.y -= delta_zoom * center_relative_to_image.y;
    zoom += delta_zoom;
}

void update_zoom_and_pos() {
    if (GetMouseWheelMove()) {
        add_zoom((1 + GetMouseWheelMove() / 10) * zoom - zoom, GetMousePosition());
    }
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        Vector2 new_pos = {draw_pos.x + delta.x, draw_pos.y + delta.y};
        if (new_pos.x >= GetScreenWidth())
            new_pos.x = GetScreenWidth() - 1;
        else if (new_pos.x + font_atlas_texture.width * zoom <= 0)
            new_pos.x = -font_atlas_texture.width * zoom + 1;
        if (new_pos.y + delta.y >= GetScreenHeight())
            new_pos.y = GetScreenHeight() - 1;
        else if (new_pos.y + font_atlas_texture.height * zoom <= 0)
            new_pos.y = -font_atlas_texture.height * zoom + 1;
        draw_pos = new_pos;
    }
    if (IsKeyPressed(KEY_R)) {      // Reset zoom and pos
        zoom = 1.0f;
        draw_pos = (Vector2){GetScreenWidth() / 2 - font_atlas_texture.width / 2, GetScreenHeight() / 2 - font_atlas_texture.height / 2};
    }
}

void update_pos_on_resize() {
    generate_transparent_pattern();
    draw_pos.x = draw_pos.x * (GetScreenWidth() / prev_win_size.x);
    draw_pos.y = draw_pos.y * (GetScreenHeight() / prev_win_size.y);
    float fileDialogWidth = max(440, GetScreenWidth() / 1.25);
    float fileDialogHeight = max(310, GetScreenHeight() / 1.25);
    fileDialogState.windowBounds = (Rectangle){GetScreenWidth()/2 - fileDialogWidth/2, GetScreenHeight()/2 - fileDialogHeight/2, fileDialogWidth, fileDialogHeight};
}

Vector2 mouse_pos_relative_to_image(int mouse_x, int mouse_y) {
    return (Vector2){(mouse_x - draw_pos.x) / zoom, (mouse_y - draw_pos.y) / zoom};
}

bool character_width_edit_mode = false;
bool character_height_edit_mode = false;
bool text_box_edit_mode = false;
GuiColorEyedropperState eyedropperState;
// Draw and process scroll bar style edition controls
void draw_side_menu() {
    // Settings
    //--------------------------------------------------------------------------------------
    Color background_color = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    int x = GetScreenWidth() - 220 - 20;
    DrawRectangle(x - 5, 15, 230, 245, background_color);
    GuiGroupBox((Rectangle){x, 20, 220, 235}, "Settings");
    
    GuiLabel((Rectangle){x + 5, 35, 110, 10}, "Character width");
    if (GuiSpinner((Rectangle){x + 120, 30, 90, 20}, NULL, &cell_x, 1, INT_MAX, character_width_edit_mode))
        character_width_edit_mode = !character_width_edit_mode;
    
    GuiLabel((Rectangle){x + 5, 60, 110, 10}, "Character height");
    if (GuiSpinner((Rectangle){x + 120, 55, 90, 20}, NULL, &cell_y, 1, INT_MAX, character_height_edit_mode))
        character_height_edit_mode = !character_height_edit_mode;

    GuiCheckBox((Rectangle){x + 5, 85, 20, 20}, "Show grid", &view_grid);
    if (!(character_width_edit_mode || character_height_edit_mode) && (font_atlas_texture.width % cell_x || font_atlas_texture.height % cell_y)) {
        Color text_color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(RED));
        GuiLabel((Rectangle){x + 5, 105, 210, 40}, "#191#Warning: image size is not\n#0#divisible by character size");
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(text_color));
    }

    DrawLine(x + 10, 150, x + 220 - 10, 150,
        GetColor(GuiGetStyle(DEFAULT, (GuiGetState() == STATE_DISABLED)? (int)BORDER_COLOR_DISABLED : (int)LINE_COLOR)));

    GuiLabel((Rectangle){x + 5, 160, 110, 10}, "Characters: ");
    if (GuiTextBox((Rectangle){x + 5, 175, 210, 20}, char_list, CHAR_LIST_MAX_SIZE, text_box_edit_mode))
        text_box_edit_mode = !text_box_edit_mode;

    GuiColorEyedropper((Rectangle){x + 5, 210, 20, 20}, (Rectangle){x + 46, 210, 25, 20}, (Rectangle){x + 86, 210, 25, 20},
     (Rectangle){x + 126, 210, 25, 20}, (Rectangle){x + 166, 210, 25, 20}, &eyedropperState, &font_atlas_image, mouse_pos_relative_to_image);
    
    //Controls
    //--------------------------------------------------------------------------------------
    int y = GetScreenHeight() - 20 - 60;
    DrawRectangle(x - 5, y - 5, 230, 70, background_color);
    GuiGroupBox((Rectangle){x, y, 220, 60}, "Controls");

    GuiLabel((Rectangle){x + 5, y + 7, 205, 50}, "Move image:\nZoom image:\nReset zoom and pos:");
    int alignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);
    GuiLabel((Rectangle){x + 5 + 205 / 2, y + 7, 205 / 2, 50}, "Right click\nMouse wheel\nR key");
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, alignment);
    
}

void draw_grid() {
    if (cell_x != 0) {
        for (float i = 0; i < font_atlas_texture.width * zoom; i += cell_x * zoom) { // Vertical lines
            DrawLine(draw_pos.x + i, draw_pos.y, draw_pos.x + i, draw_pos.y + font_atlas_texture.height * zoom, GREEN);
        }
    }
    if (cell_y != 0) {
        for (float i = 0; i < font_atlas_texture.height * zoom; i += cell_y * zoom) { // Horizontal lines
            DrawLine(draw_pos.x, draw_pos.y + i, draw_pos.x + font_atlas_texture.width * zoom, draw_pos.y + i, RED);
        }
    }
}

#include "../resources/textures/icon_16px.h"
#include "../resources/textures/icon_32px.h"
#include "../resources/textures/icon_64px.h"
#include "../resources/textures/icon_128px.h"
#include "../resources/textures/icon_256px.h"
void set_window_icon() {
    Image icons[5] = {
        (Image){.data = ICON_16PX_DATA, .format = ICON_16PX_FORMAT, .height = ICON_16PX_HEIGHT, .width = ICON_16PX_WIDTH, .mipmaps = 1},
        (Image){.data = ICON_32PX_DATA, .format = ICON_32PX_FORMAT, .height = ICON_32PX_HEIGHT, .width = ICON_32PX_WIDTH, .mipmaps = 1},
        (Image){.data = ICON_64PX_DATA, .format = ICON_64PX_FORMAT, .height = ICON_64PX_HEIGHT, .width = ICON_64PX_WIDTH, .mipmaps = 1},
        (Image){.data = ICON_128PX_DATA, .format = ICON_128PX_FORMAT, .height = ICON_128PX_HEIGHT, .width = ICON_128PX_WIDTH, .mipmaps = 1},
        (Image){.data = ICON_256PX_DATA, .format = ICON_256PX_FORMAT, .height = ICON_256PX_HEIGHT, .width = ICON_256PX_WIDTH, .mipmaps = 1},
    };
    SetWindowIcons(icons, 5);
}