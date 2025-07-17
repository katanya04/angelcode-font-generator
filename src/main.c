#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_helper.h"
#undef GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION

#include "math.h"
#include <stdio.h>

float zoom = 1.0f;
Vector2 draw_pos;
Texture2D transparent_pattern;
Texture2D font_atlas_texture;
int cell_x = 10, cell_y = 10;
char char_list[CHAR_LIST_MAX_SIZE];
GuiWindowFileDialogState fileDialogState;
bool view_grid = false;
bool failed_to_load_file = false;
bool failed_to_generate_font = false;
char current_tex_file_name[512] = { 0 };
bool malformed_char_list = false;
Color tex_background_color = BLANK;
Image font_atlas_image;

void export_font(char* path);

int main(void) {
    InitWindow(800, 450, "Angelcode font generator");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    set_window_icon();
    SetExitKey(KEY_NULL);

    fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    eyedropperState = InitGuiColorEyedropper(&tex_background_color);
    char file_name[512] = { 0 };

    generate_transparent_pattern();

    while (!WindowShouldClose()) {
        if (fileDialogState.windowActive || failed_to_load_file || failed_to_generate_font)
            GuiLock();
        if (font_atlas_texture.id != 0 && !GuiIsLocked())
            update_zoom_and_pos();
        if (IsWindowResized())
            update_pos_on_resize();
        prev_win_size = (Vector2){GetScreenWidth(), GetScreenHeight()};

        if (fileDialogState.SelectFilePressed && !fileDialogState.overwriteDialogOpened) {
            strcpy(file_name, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
            if (!fileDialogState.saveFileMode) {
                UnloadImage(font_atlas_image);
                UnloadTexture(font_atlas_texture);
                font_atlas_image = LoadImage(file_name);
                font_atlas_texture = LoadTexture(file_name);
                if (font_atlas_texture.id != 0) {
                    draw_pos = (Vector2){GetScreenWidth() / 2 - font_atlas_texture.width / 2, GetScreenHeight() / 2 - font_atlas_texture.height / 2};
                    strcpy(current_tex_file_name, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                } else
                    failed_to_load_file = true;
            } else {
                export_font(file_name);
            }
            fileDialogState.SelectFilePressed = false;
        }

        BeginDrawing();
            Color background_color = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
            ClearBackground(background_color);
            if (font_atlas_texture.id != 0) {
                Vector2 draw_pos_int = draw_pos;
                draw_pos_int.x = roundf(draw_pos_int.x);
                draw_pos_int.y = roundf(draw_pos_int.y);
                DrawTexture(transparent_pattern, 0, 0, WHITE);
                DrawTextureEx(font_atlas_texture, draw_pos_int, 0.f, zoom, WHITE);
                if (view_grid)
                    draw_grid();

                DrawRectangle(0, 0, max(0.f, draw_pos_int.x), GetScreenHeight(), background_color);
                DrawRectangle(0, 0, GetScreenWidth(), max(0.f, draw_pos_int.y), background_color);
                DrawRectangle(0, draw_pos_int.y + round(font_atlas_texture.height * zoom), GetScreenWidth(), max(0, GetScreenHeight() - (draw_pos_int.y + round(font_atlas_texture.height * zoom)) + 1), background_color);
                DrawRectangle(draw_pos_int.x + round(font_atlas_texture.width * zoom), 0, max(0, GetScreenWidth() - (draw_pos_int.x + round(font_atlas_texture.width * zoom)) + 1), GetScreenHeight(), background_color);
                DrawRectangleLines(draw_pos_int.x, draw_pos_int.y, round(font_atlas_texture.width * zoom), round(font_atlas_texture.height * zoom), BLACK);

                draw_side_menu();
            }
            if (GuiButton((Rectangle){20, 20, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Open Image"))) {
                fileDialogState.saveFileMode = false;
                fileDialogState.windowActive = true;
                strcpy(fileDialogState.filterExt, "DIR;.png;.bmp;.tga;.gif;.jpg;.jpeg;.psd;.hdr;.qoi;.dds;.pkm;.ktx;.pvr;.astc");
            } else if (font_atlas_texture.id != 0 && GuiButton((Rectangle){180, 20, 140, 30}, GuiIconText(ICON_FILE_EXPORT, "Export .fnt"))) {
                if (font_atlas_texture.width % cell_x || font_atlas_texture.height % cell_y)
                    failed_to_generate_font = true;
                else {
                    fileDialogState.saveFileMode = true;
                    fileDialogState.windowActive = true;
                    strcpy(fileDialogState.filterExt, "DIR;.fnt");
                }
            }
            GuiUnlock();
            if (failed_to_load_file && GuiMessageBox((Rectangle){GetScreenWidth() / 2 - 125, GetScreenHeight() / 2 - 50, 250, 100},
                "#191#Error", "Failed to load Image", "Ok") != -1)
                failed_to_load_file = false;
            if (failed_to_generate_font && GuiMessageBox((Rectangle){GetScreenWidth() / 2 - 125, GetScreenHeight() / 2 - 50, 250, 100},
                "#191#Error", "Image size is not divisible by\ncharacter size", "Ok") != -1)
                failed_to_generate_font = false;
            GuiWindowFileDialog(&fileDialogState);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}

unsigned int get_num_chars();
unsigned int get_next_unicode(char**);
unsigned int get_width_of_char(Image, int, int);

void export_font(char* path) {
    FILE *file = fopen(path, "w");
    // Write header (info + common + page + chars)
    //--------------------------------------------------------------------------------------
    fprintf(file, "info face=\"generated_font\" size=12 bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=0 aa=1 padding=0,0,0,0 spacing=0,0 outline=0\n");
    char common[256];
    snprintf(common, 256, "common lineHeight=%u base=0 scaleW=%u scaleH=%u pages=1 packed=0 alphaChnl=0 redChnl=0 greenChnl=0 blueChnl=0\n", cell_y, font_atlas_texture.width, font_atlas_texture.height);
    fprintf(file, common);
    char page[512 + 18];
    snprintf(page, 512 + 18, "page id=0 file=\"%s\"\n", current_tex_file_name);
    fprintf(file, page);
    char chars[256];
    snprintf(chars, 256, "chars count=%u\n", get_num_chars());
    fprintf(file, chars);
    // Write every char
    //--------------------------------------------------------------------------------------
    char* current_char = char_list;
    Image sprite_atlas = LoadImageFromTexture(font_atlas_texture);
    for (int y = 0; y < font_atlas_texture.height && *current_char != '\0'; y += cell_y) {
        for (int x = 0; x < font_atlas_texture.width && *current_char != '\0'; x += cell_x) {
            unsigned int unicode_char = get_next_unicode(&current_char);
            unsigned int width = get_width_of_char(sprite_atlas, x, y);
            char char_line[512];
            snprintf(char_line, 512, "char id=%u x=%u y=%u width=%u height=%u xoffset=0 yoffset=0 xadvance=0 page=0 chnl=15\n",
                unicode_char, x, y, width, cell_y);
            fprintf(file, char_line);
        }
    }
    fclose(file);
}

unsigned int get_num_chars() {
    unsigned int num_chars = 0;
    char* current_char = char_list;
    while (*current_char != '\0') {
        if (!(*current_char & 0b10000000))
            current_char += 1;
        else if ((*current_char & 0b11100000) == 0b11000000)
            current_char += 2;
        else if ((*current_char & 0b11110000) == 0b11100000)
            current_char += 3;
        else if ((*current_char & 0b11111000) == 0b11110000)
            current_char += 4;
        num_chars++;
    }
    return num_chars;
}

unsigned int get_next_unicode(char** p) {   //U+uvwxyz
    unsigned int result;
    if (!(**p & 0b10000000)) { // 1 byte - 0b0yyyzzzz
        result = **p;
        (*p)++;
    } else if ((**p & 0b11100000) == 0b11000000) { // 2 bytes - 0b110xxxyy 0b10yyzzzz
        unsigned char bytes[2] = {**p, *(*p + 1)};
        if ((bytes[1] & 0b11000000) == 0b10000000) {
            char first_yy = bytes[0] & 0b00000011;
            char xxx =      bytes[0] & 0b00011100;
            result = (bytes[1] & 0b00111111) + (first_yy << 6) + (xxx << 6);
        } else
            malformed_char_list = true;
        *p += 2;
    } else if ((**p & 0b11110000) == 0b11100000) { // 3 bytes - 0b1110wwww 0b10xxxxyy 0b10yyzzzz
        unsigned char bytes[3] = {**p, *(*p + 1), *(*p + 2)};
        if ((bytes[1] & 0b11000000) == 0b10000000 && (bytes[2] & 0b11000000) == 0b10000000) {
            char first_yy = bytes[1] & 0b00000011;
            char xxxx =     bytes[1] & 0b00111100;
            char wwww =     bytes[0] & 0b00001111;
            result = (bytes[2] & 0b00111111) + (first_yy << 6) + (xxxx << 6) + (wwww << 12);
        } else
            malformed_char_list = true;
        *p += 3;
    } else if ((**p & 0b11111000) == 0b11110000) { // 4 bytes - 0b11110uvv 0b10vvwwww 0b10xxxxyy 0b10yyzzzz
        unsigned char bytes[4] = {**p, *(*p + 1), *(*p + 2), *(*p + 3)};
        if ((bytes[1] & 0b11000000) == 0b10000000 && (bytes[2] & 0b11000000) == 0b10000000 && (bytes[3] & 0b11000000) == 0b10000000) {
            char first_yy = bytes[2] & 0b00000011;
            char xxxx =     bytes[2] & 0b00111100;
            char wwww =     bytes[1] & 0b00001111;
            char last_vv =  bytes[1] & 0b00110000;
            char first_vv = bytes[0] & 0b00000011;
            char u =        bytes[0] & 0b00000100;
            result = (bytes[3] & 0b00111111) + (first_yy << 6) + (xxxx << 6) + (wwww << 12) + (last_vv << 12) + (first_vv << 18) + (u << 18);
        } else
            malformed_char_list = true;
        *p += 4;
    } else {
        malformed_char_list = true;
        result = 0;
        (*p)++;
    }
    return result;
}

unsigned int get_width_of_char(Image sprite_atlas, int x_in_tex, int y_in_tex) {
    // left sweep
    unsigned int blank = 0;
    for (int x = x_in_tex; x < x_in_tex + cell_x; x++) {
        for (int y = y_in_tex; y < y_in_tex + cell_y; y++) {
            if (!ColorIsEqual(GetImageColor(sprite_atlas, x, y), tex_background_color))
                goto break_outer_1;
        }
        blank++;
    }
    break_outer_1:
    if (blank != cell_x) {
        //right sweep
        for (int x = x_in_tex + cell_x - 1; x >= x_in_tex; x--) {
            for (int y = y_in_tex + cell_y - 1; y >= y_in_tex; y--) {
                if (!ColorIsEqual(GetImageColor(sprite_atlas, x, y), tex_background_color))
                    goto break_outer_2;
            }
            blank++;
        }
    }
    break_outer_2:
    return cell_x - blank;
}