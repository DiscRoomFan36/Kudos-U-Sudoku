
#include <time.h>

#include "Bested.h"
#undef Clamp // TODO we gotta do something about this...

#include <raylib.h>
#include <raymath.h>





///////////////////////////////////////////////////////////////////////////
//                          Raylib Helpers
///////////////////////////////////////////////////////////////////////////

#define DrawRectangleFrame(x, y, width, height, lineThick, color) \
    DrawRectangleFrameRec((Rectangle){x, y, width, height}, lineThick, color)

internal void DrawRectangleFrameRec(Rectangle rec, float lineThick, Color color) {
    Rectangle Top    = { rec.x,                         rec.y,                          rec.width, lineThick                };
    DrawRectangleRec(Top, color);

    Rectangle Bottom = { rec.x,                         rec.y + rec.height - lineThick, rec.width, lineThick                };
    DrawRectangleRec(Bottom, color);

    Rectangle Left   = { rec.x,                         rec.y + lineThick,              lineThick, rec.height - lineThick*2 };
    DrawRectangleRec(Left, color);

    Rectangle Right  = { rec.x + rec.width - lineThick, rec.y + lineThick,              lineThick, rec.height - lineThick*2 };
    DrawRectangleRec(Right, color);
}


internal inline Rectangle ShrinkRectangle(Rectangle rec, float value) {
    Rectangle result = {
        rec.x + value, rec.y + value,
        rec.width   - value*2,
        rec.height  - value*2,
    };
    return result;
}



typedef struct {
    Font font;
    s32 size;
} Font_And_Size;


global_variable const char *    dynamic_font_path = NULL;
global_variable Font_And_Size   dynamic_fonts_storage[512];
global_variable u32             dynamic_fonts_storage_count = 0;


internal void InitDynamicFonts(const char *path) {
    dynamic_font_path = path;
    dynamic_fonts_storage_count = 0;
}
internal void UnloadDynamicFonts(void) {
    dynamic_font_path = NULL;

    for (u32 i = 0; i < dynamic_fonts_storage_count; i++) {
        UnloadFont(dynamic_fonts_storage[i].font);
    }
    dynamic_fonts_storage_count = 0;
}

internal Font_And_Size GetFontWithSize(s32 font_size) {
    ASSERT(dynamic_font_path && "Call InitDynamicFonts() before this function");

    s32 index = -1;
    // linear search is bad? or it it good? I forget...
    for (u32 i = 0; i < dynamic_fonts_storage_count; i++) {
        if (dynamic_fonts_storage[i].size == font_size) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        ASSERT(dynamic_fonts_storage_count <= Array_Len(dynamic_fonts_storage));
        index = dynamic_fonts_storage_count;
        dynamic_fonts_storage[dynamic_fonts_storage_count++] = (Font_And_Size) {
            .font = LoadFontEx(dynamic_font_path, font_size, NULL, 0),
            .size = font_size,
        };
    }

    return dynamic_fonts_storage[index];
}











typedef struct {
    _Array_Header_;
    s64 *items;
} Int_Array;




#define SUDOKU_SIZE                         9

#define SUDOKU_MAX_MARKINGS                 (SUDOKU_SIZE + 1) // Account for 0


// in pixels, 60 is highly divisible
#define SUDOKU_CELL_SIZE                    60
#define SUDOKU_CELL_OUTER_LINE_PADDING      (SUDOKU_CELL_SIZE / 24)       // is it cleaner when its in terms of SUDOKU_CELL_SIZE?
#define SUDOKU_CELL_SMALLER_HITBOX_SIZE     (SUDOKU_CELL_SIZE / 4)        // is it cleaner when its in terms of SUDOKU_CELL_SIZE?

#define BACKGROUND_COLOR                    WHITE   // @Color
#define SUDOKU_CELL_LINE_COLOR              GRAY    // @Color
#define SUDOKU_BOX_LINE_COLOR               BLACK   // @Color


#define SELECT_LINE_THICKNESS               (SUDOKU_CELL_SIZE / 12)
#define SELECT_HIGHLIGHT_COLOR              BLUE    // @Color


#define FONT_SIZE                           (SUDOKU_CELL_SIZE / 1)
#define FONT_COLOR                          BLACK   // @Color



// TODO maybe the markings use a different font? maybe the bold version.

#define FONT_COLOR_MARKING                  BLUE    // @Color

#define FONT_SIZE_UNCERTAIN                 (FONT_SIZE / 3)

#define FONT_SIZE_MARKING_CERTAIN_MAX_SIZE          (SUDOKU_CELL_SIZE / 2.5)
#define FONT_SIZE_MARKING_CERTAIN_MIN_SIZE          (SUDOKU_CELL_SIZE / 6)

#define MARKING_ALLOWED_CERTAIN_UPTO_BEFORE_SHRINKING  4




#define MARKING_UNCERTAIN_PADDING_UD                        5
#define MARKING_UNCERTAIN_PADDING_LR                        12
#define MARKING_UNCERTAIN_PADDING_9_AND_10_EXTRA            10

const Vector2 MARKING_LOCATIONS[SUDOKU_MAX_MARKINGS] = {
    /*  1 */ {                   MARKING_UNCERTAIN_PADDING_LR,                                                                 MARKING_UNCERTAIN_PADDING_UD},                           // left      top
    /*  2 */ {SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_LR,                                                                 MARKING_UNCERTAIN_PADDING_UD},                           // right     top
    /*  3 */ {                   MARKING_UNCERTAIN_PADDING_LR,                                              SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_UD - FONT_SIZE_UNCERTAIN},     // left      bottom
    /*  4 */ {SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_LR,                                              SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_UD - FONT_SIZE_UNCERTAIN},     // right     bottom
    /*  5 */ {SUDOKU_CELL_SIZE/2,                                                                                              MARKING_UNCERTAIN_PADDING_UD},                           // middle    top
    /*  6 */ {SUDOKU_CELL_SIZE/2,                                                                           SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_UD - FONT_SIZE_UNCERTAIN},     // middle    bottom
    /*  7 */ {                   MARKING_UNCERTAIN_PADDING_LR,                                              SUDOKU_CELL_SIZE/2                              - FONT_SIZE_UNCERTAIN/2},   // left      middle
    /*  8 */ {SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_LR,                                              SUDOKU_CELL_SIZE/2                              - FONT_SIZE_UNCERTAIN/2},   // right     middle
    /*  9 */ {                   MARKING_UNCERTAIN_PADDING_LR + MARKING_UNCERTAIN_PADDING_9_AND_10_EXTRA,   SUDOKU_CELL_SIZE/2                              - FONT_SIZE_UNCERTAIN/2},   // left-ish  middle
    /* 10 */ {SUDOKU_CELL_SIZE - MARKING_UNCERTAIN_PADDING_LR - MARKING_UNCERTAIN_PADDING_9_AND_10_EXTRA,   SUDOKU_CELL_SIZE/2                              - FONT_SIZE_UNCERTAIN/2},   // right-ish middle
};















global_variable s32     window_width  = 16*80;
global_variable s32     window_height =  9*80;

global_variable bool    debug_draw_smaller_cell_hitbox = false;








internal f32 map(f32 x, f32 min, f32 max, f32 start, f32 end) {
    ASSERT(min <= max);
    f32 a = (x - min) / (max - min); // map to [0..1]
    return a * (end - start) + start; // lerp
}


///////////////////////////////////////////////////////////////////////////
//                          Sudoku Struct & Stuff
///////////////////////////////////////////////////////////////////////////

#define NO_DIGIT_PLACED     -1

// SOA style, probably i bit overkill.
typedef struct {
    // NO_DIGIT_PLACED means no digit
    s8 digits[SUDOKU_SIZE][SUDOKU_SIZE];

    // bitfeilds descripbing witch markings are there.
    struct Marking {
        u16 uncertain;
        u16   certain;
        // TODO color's
        // TODO maybe also lines, but they whouldnt be in this struct.
    } markings[SUDOKU_SIZE][SUDOKU_SIZE];


    struct Sudoku_UI {
        bool is_selected;
        bool is_hovering_over;
    } ui[SUDOKU_SIZE][SUDOKU_SIZE];
} Sudoku_Grid;

typedef struct {
    s8 *digit;
    struct Marking   *marking;
    struct Sudoku_UI *ui;
} Sudoku_Grid_Cell;


internal inline Sudoku_Grid_Cell get_cell(Sudoku_Grid *grid, u8 i, u8 j) {
    ASSERT(Is_Between(i, 0, SUDOKU_SIZE) && Is_Between(j, 0, SUDOKU_SIZE));
    Sudoku_Grid_Cell result = {
        .digit          = &grid->digits  [j][i],
        .marking        = &grid->markings[j][i],
        .ui             = &grid->ui      [j][i],
    };
    return result;
}

internal inline Rectangle get_cell_bounds(Sudoku_Grid *grid, u8 i, u8 j) {
    (void) grid; // maybe this grid will tell us where we are, eventually.

    Rectangle sudoku_cell = {
        (i*SUDOKU_CELL_SIZE) + (window_width /2) - (SUDOKU_SIZE*SUDOKU_CELL_SIZE)/2,
        (j*SUDOKU_CELL_SIZE) + (window_height/2) - (SUDOKU_SIZE*SUDOKU_CELL_SIZE)/2,
        SUDOKU_CELL_SIZE,
        SUDOKU_CELL_SIZE,
    };

    return sudoku_cell;
}



// NOTE lower = higher priority
typedef enum {
    SUL_DIGIT       = 0,
    SUL_CERTAIN     = 1,
    SUL_UNCERTAIN   = 2,
    SUL_COLOR       = 3,
} Sudoku_UI_Layer;




///////////////////////////////////////////////////////////////////////////
//                          Save / Load Sudoku
///////////////////////////////////////////////////////////////////////////

#define MAX_TEMP_FILE_SIZE      (32 * KILOBYTE)
// overwrites temeratry buffer every call.
internal String temp_Read_Entire_File(const char *filename) {
    local_persist u8 temp_file_storeage[MAX_TEMP_FILE_SIZE];

    FILE *file = fopen(filename, "rb");
    String result = ZEROED;
    result.data = (void*) temp_file_storeage;

    if (file) {
        fseek(file, 0, SEEK_END);
        s64 size = Min((s64)MAX_TEMP_FILE_SIZE, ftell(file));
        fseek(file, 0, SEEK_SET);

        if (size >= 0) {
            result.length = size;
            fread(result.data, 1, result.length, file);
        }
        fclose(file);
    }

    return result;
}






typedef u32     Magic_Number;
typedef u32     Version_Number;


u8 SUDOKU_MAGIC_NUMBER_ARRAY[sizeof(Version_Number)] = {'S', 'U', 'D', 'K'};
#define SUDOKU_MAGIC_NUMBER         (*(u32*)SUDOKU_MAGIC_NUMBER_ARRAY)

#define CURRENT_SUDOKU_SAVE_STRUCT_VERSION_NUMBER      1



#define SUDOKU_SIZE_VERSION_1           9

typedef struct {
    Magic_Number    magic_number;
    Version_Number  version_number;

    s8  digits_on_the_grid[SUDOKU_SIZE_VERSION_1][SUDOKU_SIZE_VERSION_1];

    struct Markings_V1 {
        u16 uncertain;
        u16   certain;
    } digit_markings_on_the_grid[SUDOKU_SIZE_VERSION_1][SUDOKU_SIZE_VERSION_1];
} Sudoku_Save_Struct_Version_1;

typedef Sudoku_Save_Struct_Version_1        Sudoku_Save_Struct;
static_assert(CURRENT_SUDOKU_SAVE_STRUCT_VERSION_NUMBER == 1, "to change when new version");





static_assert(Member_Size(Sudoku_Save_Struct, digits_on_the_grid) == Member_Size(Sudoku_Grid, digits), "change when we can change the size of the grid.");


// returns NULL on succsess, else returns error message
internal const char *load_sudoku_version_1(const char *filename, Sudoku_Grid *result) {
    String file = temp_Read_Entire_File(filename);
    if (file.length == 0) return "Could not read file for some reason, or file was empty.";

    if (file.length < sizeof(u64)) return "File was to small to contain header and version number";

    Sudoku_Save_Struct_Version_1 *save_struct = (void*) file.data;


    // Check if the file is valid.

    // TODO extract this into its own function to help when making more load functions
    if (save_struct->magic_number   != SUDOKU_MAGIC_NUMBER) return temp_sprintf("Magic number was incorrect. wanted (0x%X), got (0x%X)", SUDOKU_MAGIC_NUMBER, save_struct->magic_number);
    if (save_struct->version_number != 1) return temp_sprintf("Wanted version number (1), got (%d)", save_struct->version_number);

    if (file.length != sizeof(Sudoku_Save_Struct_Version_1)) return temp_sprintf("incorrect file size for version 1, wanted (%ld), got (%ld)", sizeof(Sudoku_Save_Struct_Version_1), file.length);


    for (u8 j = 0; j < Array_Len(save_struct->digits_on_the_grid); j++) {
        for (u8 i = 0; i < Array_Len(save_struct->digits_on_the_grid[j]); i++) {
            s8 digit = save_struct->digits_on_the_grid[j][i];
            if (!Is_Between(digit, -1, 9)) return temp_sprintf("(%d, %d) was outside the acceptible range [-1, 9], was, %d", i, j, digit);

            struct Markings_V1 markings = save_struct->digit_markings_on_the_grid[j][i];
            (void) markings; // TODO validate this.
        }
    }


    // load into Sudoku_Grid
    if (result) {
        static_assert(CURRENT_SUDOKU_SAVE_STRUCT_VERSION_NUMBER == 1, "to change when new version");
        static_assert(SUDOKU_SIZE_VERSION_1 == SUDOKU_SIZE, "would be pretty dump to miss this...");

        for (u8 j = 0; j < Array_Len(save_struct->digits_on_the_grid); j++) {
            for (u8 i = 0; i < Array_Len(save_struct->digits_on_the_grid[j]); i++) {
                Sudoku_Grid_Cell cell = get_cell(result, i, j);

                *cell.digit = save_struct->digits_on_the_grid[j][i];

                cell.marking->uncertain = save_struct->digit_markings_on_the_grid[j][i].uncertain;
                cell.marking->  certain = save_struct->digit_markings_on_the_grid[j][i].  certain;

                // clear these also.
                cell.ui->is_selected = false;
                cell.ui->is_hovering_over = false;
            }
        }
    }
    return NULL;
}

// returns wheather or not the file was saved succsessfully.
internal bool save_sudoku(const char *filename, Sudoku_Grid *to_save) {
    ASSERT(to_save);

    printf("Saving sudoku version %d\n", CURRENT_SUDOKU_SAVE_STRUCT_VERSION_NUMBER);

    Sudoku_Save_Struct sudoku_save_struct = ZEROED;

    sudoku_save_struct.magic_number = SUDOKU_MAGIC_NUMBER;

    sudoku_save_struct.version_number = CURRENT_SUDOKU_SAVE_STRUCT_VERSION_NUMBER;

    for (u8 j = 0; j < Array_Len(sudoku_save_struct.digits_on_the_grid); j++) {
        for (u8 i = 0; i < Array_Len(sudoku_save_struct.digits_on_the_grid[j]); i++) {
            sudoku_save_struct.digits_on_the_grid[j][i] = to_save->digits[j][i];

            sudoku_save_struct.digit_markings_on_the_grid[j][i].uncertain = to_save->markings[j][i].uncertain;
            sudoku_save_struct.digit_markings_on_the_grid[j][i].  certain = to_save->markings[j][i].  certain;
        }
    }


    // raylib function. dont really need this but whatever.
    return SaveFileData(filename, &sudoku_save_struct, sizeof(sudoku_save_struct));
}













internal void toggle_when_pressed(bool *to_toggle, int key) { *to_toggle ^= IsKeyPressed(key); }



Arena_Pool pool = ZEROED;


const char *save_path = "./build/save.sudoku";

int main(void) {

    Arena *scratch = Pool_Get(&pool);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "Sudoku");

    InitDynamicFonts("./assets/font/iosevka-light.ttf");

    Sudoku_Grid grid = ZEROED;
    {
        const char *error = load_sudoku_version_1(save_path, &grid);
        if (error) {
            fprintf(stderr, "Failed To Load Save '%s': %s\n", save_path, error);
        } else {
            printf("Succsessfully loaded save file '%s'\n", save_path);
        }
    }


    while (!WindowShouldClose()) {
        Arena_Clear(scratch);

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        window_width    = GetScreenWidth();
        window_height   = GetScreenHeight();


        Vector2 mouse_pos                           = GetMousePosition();
        bool    mouse_left_clicked                  = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool    mouse_left_down                     = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool    keyboard_shift_down                 = IsKeyDown(KEY_LEFT_SHIFT)   || IsKeyDown(KEY_RIGHT_SHIFT);
        bool    keyboard_control_down               = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        bool    keyboard_delete_pressed             = IsKeyPressed(KEY_DELETE)    || IsKeyPressed(KEY_BACKSPACE);

        bool    keyboard_shift_or_control_down      = keyboard_shift_down || keyboard_control_down;


        ////////////////////////////////
        //     placeing digits
        ////////////////////////////////
        Sudoku_UI_Layer layer_to_place;
        if      (keyboard_shift_down && keyboard_control_down) layer_to_place = SUL_COLOR;
        else if (keyboard_shift_down)                          layer_to_place = SUL_UNCERTAIN;
        else if (keyboard_control_down)                        layer_to_place = SUL_CERTAIN;
        else                                                   layer_to_place = SUL_DIGIT;

        // determines wheather a number was pressed to put it into the grid.
        s8 number_pressed = NO_DIGIT_PLACED;
        u8 number_keys[] = {KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE};
        for (u32 i = 0; i < Array_Len(number_keys); i++) {
            if (IsKeyPressed(number_keys[i]))   number_pressed = i;
        }


        ////////////////////////////////
        //     removing digits
        ////////////////////////////////
        // make sure that all boxes get the same result after pressing a key (all add or all remove)
        bool remove_number_this_press = true;

        Sudoku_UI_Layer layer_to_delete = SUL_COLOR; // lowest priority



        toggle_when_pressed(&debug_draw_smaller_cell_hitbox, KEY_F1);


        local_persist bool when_dragging_to_set_selected_to = true;
        if (!mouse_left_down) when_dragging_to_set_selected_to = true;


        // update sudoku grid
        // phase 1
        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                Sudoku_Grid_Cell    cell        = get_cell(&grid, i, j);
                Rectangle           cell_bounds = get_cell_bounds(&grid, i, j);


                // selected stuff
                bool mouse_is_over = CheckCollisionPointRec(mouse_pos, cell_bounds);
                cell.ui->is_hovering_over = mouse_is_over;

                if (mouse_left_clicked) {
                    if (mouse_is_over) {
                        if (keyboard_shift_or_control_down) {
                            // start of deselection drag
                            cell.ui->is_selected = !cell.ui->is_selected; // TOGGLE
                        } else {
                            // just this one will be selected
                            cell.ui->is_selected = true;
                        }
                        when_dragging_to_set_selected_to = cell.ui->is_selected;
                    } else {
                        // only stay active if shift or cntl is down
                        cell.ui->is_selected = cell.ui->is_selected && keyboard_shift_or_control_down;
                    }
                }


                // placing digits stuff
                if (cell.ui->is_selected) {
                    bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                    if (number_pressed != NO_DIGIT_PLACED) {
                        switch (layer_to_place) {
                            case SUL_DIGIT: {
                                remove_number_this_press = remove_number_this_press && (*cell.digit == number_pressed);
                            } break;
                            case SUL_CERTAIN: {
                                if (!has_digit) remove_number_this_press = remove_number_this_press && (cell.marking->  certain & (1 << (number_pressed)));
                            } break;
                            case SUL_UNCERTAIN: {
                                if (!has_digit) remove_number_this_press = remove_number_this_press && (cell.marking->uncertain & (1 << (number_pressed)));
                            } break;
                            case SUL_COLOR: { /* TODO Color the cell */ } break;
                        }
                    }

                    if (keyboard_delete_pressed) {
                        // TODO maybe if you have cntl press or something it dose something different?

                        if (has_digit)                  layer_to_delete = Min(layer_to_delete, SUL_DIGIT);
                        if (cell.marking->  certain)    layer_to_delete = Min(layer_to_delete, SUL_CERTAIN);
                        if (cell.marking->uncertain)    layer_to_delete = Min(layer_to_delete, SUL_UNCERTAIN);
                        // Color is the lowest priority
                    }
                }
            }
        }

        // phase 2
        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                Sudoku_Grid_Cell    cell        = get_cell(&grid, i, j);
                Rectangle           cell_bounds = get_cell_bounds(&grid, i, j);


                // placeing digits
                // do this before the dragging stuff, it probably will never come up,
                // but its more correct this way.
                if (cell.ui->is_selected) {
                    if (number_pressed != NO_DIGIT_PLACED) {
                        bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                        switch (layer_to_place) {
                            case SUL_DIGIT: {
                                if (remove_number_this_press)   *cell.digit = NO_DIGIT_PLACED; // @Place_Digit
                                else                            *cell.digit = number_pressed;  // @Place_Digit
                            } break;
                            case SUL_CERTAIN: {
                                if (!has_digit) {
                                    if (remove_number_this_press)   cell.marking->  certain = cell.marking->  certain & ~(1 << number_pressed);
                                    else                            cell.marking->  certain = cell.marking->  certain |  (1 << number_pressed);
                                }
                            } break;
                            case SUL_UNCERTAIN: {
                                if (!has_digit) {
                                    if (remove_number_this_press)   cell.marking->uncertain = cell.marking->uncertain & ~(1 << number_pressed);
                                    else                            cell.marking->uncertain = cell.marking->uncertain |  (1 << number_pressed);
                                }
                            } break;
                            case SUL_COLOR: { /* TODO Color the cell */ } break;
                        }
                    }

                    if (keyboard_delete_pressed) {
                        switch (layer_to_delete) {
                            case SUL_DIGIT:     { *cell.digit = NO_DIGIT_PLACED; /* @Place_Digit */ } break;
                            case SUL_CERTAIN:   { cell.marking->  certain = 0; } break;
                            case SUL_UNCERTAIN: { cell.marking->uncertain = 0; }break;
                            case SUL_COLOR: { /* TODO Remove Color */ } break;
                        }
                    }
                }


                // selected stuff, mouse dragging
                Rectangle smaller_hitbox = ShrinkRectangle(cell_bounds, SUDOKU_CELL_SMALLER_HITBOX_SIZE);
                if (debug_draw_smaller_cell_hitbox) DrawRectangleRec(smaller_hitbox, ColorAlpha(YELLOW, 0.4));

                if (mouse_left_down && CheckCollisionPointRec(mouse_pos, smaller_hitbox)) {
                    cell.ui->is_selected = when_dragging_to_set_selected_to;
                }
            }
        }






        // draw sudoku grid
        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                Sudoku_Grid_Cell    cell        = get_cell(&grid, i, j);
                Rectangle           cell_bounds = get_cell_bounds(&grid, i, j);


                DrawRectangleFrameRec(cell_bounds, SUDOKU_CELL_OUTER_LINE_PADDING, SUDOKU_CELL_LINE_COLOR);


                if (*cell.digit != NO_DIGIT_PLACED) {
                    // draw placed digit

                    // TODO do it smart
                    // char buf[2] = ZEROED;
                    // DrawTextCodepoint();
                    const char *text = TextFormat("%d", *cell.digit);

                    Font_And_Size font_and_size = GetFontWithSize(FONT_SIZE);

                    Vector2 text_size = MeasureTextEx(font_and_size.font, text, font_and_size.size, 0);
                    Vector2 text_position = {
                        cell_bounds.x + SUDOKU_CELL_SIZE/2 - text_size.x/2,
                        cell_bounds.y + SUDOKU_CELL_SIZE/2 - FONT_SIZE/2,
                    };
                    DrawTextEx(font_and_size.font, text, text_position, font_and_size.size, 0, FONT_COLOR);
                } else {
                    // draw uncertain and certain digits
                    Int_Array uncertain_numbers = { .allocator = scratch };
                    Int_Array certain_numbers   = { .allocator = scratch };

                    for (u8 k = 0; k <= 9; k++) {
                        if (cell.marking->uncertain & (1 << k)) Array_Push(&uncertain_numbers, k);
                        if (cell.marking->  certain & (1 << k)) Array_Push(&  certain_numbers, k);
                    }


                    // draw uncertain numbers around the edge of the box
                    if (uncertain_numbers.count) {
                        Font_And_Size font_and_size = GetFontWithSize(FONT_SIZE_UNCERTAIN); // what matters more? speed or binding energy?
                        for (u32 k = 0; k < uncertain_numbers.count; k++) {
                            // TODO do it smarter like above.
                            const char *text = TextFormat("%d", uncertain_numbers.items[k]);

                            Vector2 text_size = MeasureTextEx(font_and_size.font, text, font_and_size.size, 0);
                            Vector2 text_pos = { cell_bounds.x - text_size.x/2, cell_bounds.y };
                            text_pos = Vector2Add(text_pos, MARKING_LOCATIONS[k]);
                            DrawTextEx(font_and_size.font, text, text_pos, font_and_size.size, 0, FONT_COLOR_MARKING);
                        }
                    }


                    // draw small certain numbers in the middle of the box
                    if (certain_numbers.count) {
                        char buf[SUDOKU_MAX_MARKINGS+1] = ZEROED;
                        for (u32 k = 0; k < certain_numbers.count; k++) buf[k] = '0' + certain_numbers.items[k];

                        s32 font_size = FONT_SIZE_MARKING_CERTAIN_MAX_SIZE;
                        if (certain_numbers.count > MARKING_ALLOWED_CERTAIN_UPTO_BEFORE_SHRINKING) {
                            font_size = map(
                                certain_numbers.count,
                                MARKING_ALLOWED_CERTAIN_UPTO_BEFORE_SHRINKING, SUDOKU_MAX_MARKINGS,
                                FONT_SIZE_MARKING_CERTAIN_MAX_SIZE, FONT_SIZE_MARKING_CERTAIN_MIN_SIZE
                            );
                        }

                        Font_And_Size font_and_size = GetFontWithSize(font_size);

                        Vector2 text_size = MeasureTextEx(font_and_size.font, buf, font_and_size.size, 0);
                        Vector2 text_pos = {
                            cell_bounds.x + SUDOKU_CELL_SIZE/2 - text_size.x/2,
                            cell_bounds.y + SUDOKU_CELL_SIZE/2 - font_and_size.size/2,
                        };
                        DrawTextEx(font_and_size.font, buf, text_pos, font_and_size.size, 0, FONT_COLOR_MARKING);
                    }
                }


                // hovering and stuff
                if (cell.ui->is_selected) {
                    // drawing flowing selected
                    bool is_selected_up         =                         j == 0                ? false : get_cell(&grid, i  , j-1).ui->is_selected;
                    bool is_selected_down       =                         j == SUDOKU_SIZE - 1  ? false : get_cell(&grid, i  , j+1).ui->is_selected;
                    bool is_selected_left       = i == 0                                        ? false : get_cell(&grid, i-1, j  ).ui->is_selected;
                    bool is_selected_right      = i == SUDOKU_SIZE - 1                          ? false : get_cell(&grid, i+1, j  ).ui->is_selected;

                    bool is_selected_up_left    = i == 0               || j == 0                ? false : get_cell(&grid, i-1, j-1).ui->is_selected;
                    bool is_selected_up_right   = i == SUDOKU_SIZE - 1 || j == 0                ? false : get_cell(&grid, i+1, j-1).ui->is_selected;
                    bool is_selected_down_left  = i == 0               || j == SUDOKU_SIZE-1    ? false : get_cell(&grid, i-1, j+1).ui->is_selected;
                    bool is_selected_down_right = i == SUDOKU_SIZE - 1 || j == SUDOKU_SIZE-1    ? false : get_cell(&grid, i+1, j+1).ui->is_selected;


                    bool draw_line_up           = !is_selected_up;
                    bool draw_line_down         = !is_selected_down;
                    bool draw_line_left         = !is_selected_left;
                    bool draw_line_right        = !is_selected_right;

                    bool draw_line_up_left      = draw_line_up   || draw_line_left  || (!is_selected_up_left    && is_selected_up   && is_selected_left );
                    bool draw_line_up_right     = draw_line_up   || draw_line_right || (!is_selected_up_right   && is_selected_up   && is_selected_right);
                    bool draw_line_down_left    = draw_line_down || draw_line_left  || (!is_selected_down_left  && is_selected_down && is_selected_left );
                    bool draw_line_down_right   = draw_line_down || draw_line_right || (!is_selected_down_right && is_selected_down && is_selected_right);


                    Rectangle cell_bounds       = get_cell_bounds(&grid, i, j);
                    Rectangle select_bounds     = ShrinkRectangle(cell_bounds, SUDOKU_CELL_OUTER_LINE_PADDING);

                    // orthoganal
                    Rectangle line_up           = { select_bounds.x + SELECT_LINE_THICKNESS,                        select_bounds.y,                                                select_bounds.width - SELECT_LINE_THICKNESS*2,  SELECT_LINE_THICKNESS,                        };
                    Rectangle line_down         = { select_bounds.x + SELECT_LINE_THICKNESS,                        select_bounds.y + select_bounds.height - SELECT_LINE_THICKNESS, select_bounds.width - SELECT_LINE_THICKNESS*2,  SELECT_LINE_THICKNESS,                        };
                    Rectangle line_left         = { select_bounds.x,                                                select_bounds.y + SELECT_LINE_THICKNESS,                        SELECT_LINE_THICKNESS,                          cell_bounds.height - SELECT_LINE_THICKNESS*2, };
                    Rectangle line_right        = { select_bounds.x + select_bounds.width - SELECT_LINE_THICKNESS,  select_bounds.y + SELECT_LINE_THICKNESS,                        SELECT_LINE_THICKNESS,                          cell_bounds.height - SELECT_LINE_THICKNESS*2, };
                    // diagonal
                    Rectangle line_up_left      = { select_bounds.x,                                                select_bounds.y,                                                SELECT_LINE_THICKNESS,                          SELECT_LINE_THICKNESS,                        };
                    Rectangle line_up_right     = { select_bounds.x + select_bounds.width - SELECT_LINE_THICKNESS,  select_bounds.y,                                                SELECT_LINE_THICKNESS,                          SELECT_LINE_THICKNESS,                        };
                    Rectangle line_down_left    = { select_bounds.x,                                                select_bounds.y + select_bounds.height - SELECT_LINE_THICKNESS, SELECT_LINE_THICKNESS,                          SELECT_LINE_THICKNESS,                        };
                    Rectangle line_down_right   = { select_bounds.x + select_bounds.width - SELECT_LINE_THICKNESS,  select_bounds.y + select_bounds.height - SELECT_LINE_THICKNESS, SELECT_LINE_THICKNESS,                          SELECT_LINE_THICKNESS,                        };


                    if (draw_line_up        )   DrawRectangleRec(line_up,         SELECT_HIGHLIGHT_COLOR); //YELLOW);
                    if (draw_line_down      )   DrawRectangleRec(line_down,       SELECT_HIGHLIGHT_COLOR); //RED);
                    if (draw_line_left      )   DrawRectangleRec(line_left,       SELECT_HIGHLIGHT_COLOR); //PURPLE);
                    if (draw_line_right     )   DrawRectangleRec(line_right,      SELECT_HIGHLIGHT_COLOR); //GREEN);

                    if (draw_line_up_left   )   DrawRectangleRec(line_up_left,    SELECT_HIGHLIGHT_COLOR); //MAROON);
                    if (draw_line_up_right  )   DrawRectangleRec(line_up_right,   SELECT_HIGHLIGHT_COLOR); //ORANGE);
                    if (draw_line_down_left )   DrawRectangleRec(line_down_left,  SELECT_HIGHLIGHT_COLOR); //GOLD);
                    if (draw_line_down_right)   DrawRectangleRec(line_down_right, SELECT_HIGHLIGHT_COLOR); //PINK);


                } else if (cell.ui->is_hovering_over) {
                    DrawRectangleRec(ShrinkRectangle(cell_bounds, SUDOKU_CELL_OUTER_LINE_PADDING), ColorAlpha(BLACK, 0.2)); // cool trick
                }
            }
        }


        Rectangle sudoku_top_left_corner_box = get_cell_bounds(&grid, 0, 0);

        // Boarder Lines
        DrawRectangleFrame(
            sudoku_top_left_corner_box.x - SUDOKU_CELL_OUTER_LINE_PADDING,
            sudoku_top_left_corner_box.y - SUDOKU_CELL_OUTER_LINE_PADDING,
            SUDOKU_CELL_SIZE * SUDOKU_SIZE + SUDOKU_CELL_OUTER_LINE_PADDING*2,
            SUDOKU_CELL_SIZE * SUDOKU_SIZE + SUDOKU_CELL_OUTER_LINE_PADDING*2,
            SUDOKU_CELL_OUTER_LINE_PADDING * 2,
            SUDOKU_BOX_LINE_COLOR
        );

        // Lines that seperate the Boxes
        for (u32 j = 0; j < SUDOKU_SIZE/3; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE/3; i++) {
                Rectangle sudoku_box = get_cell_bounds(&grid, i*3, j*3);
                sudoku_box.width  *= 3; // Ha Ha
                sudoku_box.height *= 3; // Ha Ha

                DrawRectangleFrameRec(sudoku_box, SUDOKU_CELL_OUTER_LINE_PADDING, SUDOKU_BOX_LINE_COLOR);
            }
        }


        EndDrawing();
    }


    bool result = save_sudoku(save_path, &grid);

    UnloadDynamicFonts();

    CloseWindow();

    Pool_Free_Arenas(&pool);

    return result ? 0 : 1;
}





#define BESTED_IMPLEMENTATION
#include "Bested.h"
