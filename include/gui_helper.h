#ifndef gui_helper_H_INCLUDED
#define gui_helper_H_INCLUDED

#include "raylib.h"
#include "gui_window_file_dialog.h"
#include "gui_color_eyedropper.h"
#include "limits.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#ifndef NULL
  #define NULL ((void *)0)
#endif
#define CHAR_LIST_MAX_SIZE USHRT_MAX

void generate_transparent_pattern();
void add_zoom(float delta_zoom, Vector2 center_cords);
void update_zoom_and_pos();
void update_pos_on_resize();
void draw_side_menu();
void draw_grid();
void set_window_icon();

extern float zoom;
extern Vector2 draw_pos;
extern Texture2D transparent_pattern;
extern Vector2 prev_win_size;
extern GuiWindowFileDialogState fileDialogState;
extern GuiColorEyedropperState eyedropperState;
extern bool failed_to_load_file;

// send to another file maybe
extern Texture2D font_atlas_texture;
extern Image font_atlas_image;
extern int cell_x;
extern int cell_y;
extern char char_list[CHAR_LIST_MAX_SIZE];
extern bool view_grid;
extern Color tex_background_color;

#endif