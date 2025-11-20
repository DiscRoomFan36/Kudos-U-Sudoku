
#include <time.h>

#include "Bested.h"
#undef Clamp // TODO we gotta do something about this...

// TODO @Bested.h change to append
#define Array_Append Array_Push



#include <raylib.h>
#include <raymath.h>

#include "raylib_helpers.c"


// all the memory in this program comes from here.
global_variable Arena_Pool pool = ZEROED;

internal Arena *Scratch_Get(void)            { return Pool_Get(&pool); }
internal void   Scratch_Release(Arena *scratch) { Pool_Release(&pool, scratch); }




typedef struct {
    _Array_Header_;
    s64 *items;
} Int_Array;




#define SUDOKU_SIZE                         9

#define SUDOKU_MAX_MARKINGS                 (SUDOKU_SIZE + 1) // Account for 0


// in pixels, 60 is highly divisible
#define SUDOKU_CELL_SIZE                    60

#define SUDOKU_CELL_INNER_LINE_THICKNESS    1.0           // (SUDOKU_CELL_SIZE / 24)       // is it cleaner when its in terms of SUDOKU_CELL_SIZE?
#define SUDOKU_CELL_BOARDER_LINE_THICKNESS  3.0

static_assert((s32)SUDOKU_CELL_INNER_LINE_THICKNESS <= (s32)SUDOKU_CELL_BOARDER_LINE_THICKNESS, "must be true");


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














// sqaure for now, change when adding a control pannel.
global_variable s32     window_width  =  9*80;
global_variable s32     window_height =  9*80;

global_variable bool    debug_draw_smaller_cell_hitbox  = false;
global_variable bool    debug_draw_cursor_position      = false;






// TODO @Bested.h
#define Proper_Mod(x, y) ({ Typeof(y) _y = (y); (((x) % _y) + _y) % _y; })


internal f32 map(f32 x, f32 min, f32 max, f32 start, f32 end) {
    ASSERT(min <= max);
    f32 a = (x - min) / (max - min); // map to [0..1]
    return a * (end - start) + start; // lerp
}


///////////////////////////////////////////////////////////////////////////
//                          Sudoku Struct & Stuff
///////////////////////////////////////////////////////////////////////////


// NOTE lower = higher priority
typedef enum {
    SUL_DIGIT       = 0,
    SUL_CERTAIN     = 1,
    SUL_UNCERTAIN   = 2,
    SUL_COLOR       = 3,
} Sudoku_UI_Layer;


#define NO_DIGIT_PLACED     -1

typedef struct {
    // NO_DIGIT_PLACED means no digit
    s8 digits[SUDOKU_SIZE][SUDOKU_SIZE];

    // bitfeilds descripbing witch markings are there.
    struct Marking {
        // real digits are Black, players digits are marking color
        bool digit_placed_in_solve_mode;

        u16 uncertain; // bitfield's
        u16   certain; // bitfield's

        u32 color_bitfield; // color can be any pattern of bits, TODO use voronoi to generate color patterns.
        // TODO maybe also lines, but they whouldnt be in this struct.
    } markings[SUDOKU_SIZE][SUDOKU_SIZE];

} Sudoku_Grid;

typedef struct {
    _Array_Header_;
    Sudoku_Grid *items;
} Sudoku_Grid_Array;




// SOA style, probably i bit overkill.
typedef struct {
    Sudoku_Grid grid;

    struct Sudoku_UI {
        bool is_selected;
        bool is_hovering_over;
    } ui[SUDOKU_SIZE][SUDOKU_SIZE];


    String name;
    #define SUDOKU_MAX_NAME_LENGTH 128
    char name_buf[SUDOKU_MAX_NAME_LENGTH+1];
    u32 name_buf_count;



    // Undo
    //
    // the last item in this buffer is always the current grid.
    Sudoku_Grid_Array undo_buffer;
    u32 redo_count; // how many times you can redo.
} Sudoku;



typedef struct {
    s8 *digit;
    struct Marking *marking;
} Sudoku_Grid_Cell;


typedef struct {
    s8 *digit;
    struct Marking   *marking;
    struct Sudoku_UI *ui;
} Sudoku_Cell;


#define ASSERT_VALID_SUDOKU_ADDRESS(i, j)       ASSERT(Is_Between((i), 0, SUDOKU_SIZE) && Is_Between((j), 0, SUDOKU_SIZE))



internal inline Sudoku_Grid_Cell get_grid_cell(Sudoku_Grid *grid, u8 i, u8 j) {
    ASSERT_VALID_SUDOKU_ADDRESS(i, j);
    Sudoku_Grid_Cell result = {
        .digit      = &grid->digits  [j][i],
        .marking    = &grid->markings[j][i],
    };
    return result;
}



internal inline Sudoku_Cell get_cell(Sudoku *sudoku, u8 i, u8 j) {
    ASSERT_VALID_SUDOKU_ADDRESS(i, j);
    Sudoku_Cell result = {
        .digit          = &sudoku->grid.digits  [j][i],
        .marking        = &sudoku->grid.markings[j][i],
        .ui             = &sudoku->ui           [j][i],
    };
    return result;
}


internal inline Rectangle get_cell_bounds(Sudoku *sudoku, u8 i, u8 j) {
    // while this dosnt nessesarily cause problems, i want this to be
    // used wherever get_cell is, and them having the same properties is nice
    ASSERT_VALID_SUDOKU_ADDRESS(i, j);

    (void) sudoku; // maybe this will tell us where we are, someday.

    Vector2 top_left_corner = {
        (window_width /2) - ((SUDOKU_SIZE*SUDOKU_CELL_SIZE) + (2*(SUDOKU_CELL_BOARDER_LINE_THICKNESS - SUDOKU_CELL_INNER_LINE_THICKNESS/2)))/2,
        (window_height/2) - ((SUDOKU_SIZE*SUDOKU_CELL_SIZE) + (2*(SUDOKU_CELL_BOARDER_LINE_THICKNESS - SUDOKU_CELL_INNER_LINE_THICKNESS/2)))/2,
    };

    Rectangle sudoku_cell = {
        top_left_corner.x + (i*SUDOKU_CELL_SIZE) + ((i/3) * (SUDOKU_CELL_BOARDER_LINE_THICKNESS - SUDOKU_CELL_INNER_LINE_THICKNESS/2)),
        top_left_corner.y + (j*SUDOKU_CELL_SIZE) + ((j/3) * (SUDOKU_CELL_BOARDER_LINE_THICKNESS - SUDOKU_CELL_INNER_LINE_THICKNESS/2)),
        SUDOKU_CELL_SIZE,
        SUDOKU_CELL_SIZE,
    };

    return sudoku_cell;
}


/*
internal void put_digit_on_layer(Sudoku_Grid *grid, u8 i, u8 j, s8 digit, Sudoku_UI_Layer layer) {
    ASSERT_VALID_SUDOKU_ADDRESS(i, j);
    Sudoku_Grid_Cell cell = get_cell(grid, i, j);

    switch (layer) {
        case SUL_DIGIT: {
            if (*cell.digit == digit)
        } break;
        case SUL_CERTAIN:   { TODO("SUL_CERTAIN");  } break;
        case SUL_UNCERTAIN: { TODO("SUL_UNCERTAIN");  } break;
        case SUL_COLOR:     { TODO("SUL_COLOR");  } break;
    }
}
*/



#include "sudoku_save_and_load.c"













internal void toggle_when_pressed(bool *to_toggle, int key) { *to_toggle ^= IsKeyPressed(key); }





const char *autosave_path = "./build/autosave.sudoku";

int main(void) {

    Arena *scratch = Pool_Get(&pool);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(window_width, window_height, "Sudoku");

    InitDynamicFonts("./assets/font/iosevka-light.ttf");

    Sudoku sudoku = ZEROED;
    // make sure that load_sudoku_version_1(), respects this alloctor
    sudoku.undo_buffer.allocator = Pool_Get(&pool);
    Array_Reserve(&sudoku.undo_buffer, 512); // just give it some room.


    {
        const char *error = load_sudoku(autosave_path, &sudoku);
        if (error) {
            fprintf(stderr, "Failed To Load Save '%s': %s\n", autosave_path, error);
            // TODO make a "clear grid" function.
            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    *get_cell(&sudoku, i, j).digit = NO_DIGIT_PLACED;
                }
            }
        } else {
            printf("Succsessfully loaded save file '%s'\n", autosave_path);
        }
    }


#define Sudoku_Grid_Is_The_Same_As_The_Last_Element_In_The_Undo_Buffer(sudoku) Mem_Eq(&(sudoku)->grid, &(sudoku)->undo_buffer.items[(sudoku)->undo_buffer.count-1], sizeof((sudoku)->grid))

    if (sudoku.undo_buffer.count == 0) {
        Array_Append(&sudoku.undo_buffer, sudoku.grid);
        sudoku.redo_count = 0;
    }
    ASSERT(Sudoku_Grid_Is_The_Same_As_The_Last_Element_In_The_Undo_Buffer(&sudoku));


    while (!WindowShouldClose()) {
        ASSERT(Sudoku_Grid_Is_The_Same_As_The_Last_Element_In_The_Undo_Buffer(&sudoku));


        Arena_Clear(scratch);

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        window_width    = GetScreenWidth();
        window_height   = GetScreenHeight();

        ////////////////////////////////
        //        User Input
        ////////////////////////////////
        struct {
            struct {
                Vector2 pos;
                struct {
                    bool clicked;
                    bool down;
                } left;
            } mouse;


            struct {
                bool shift_down;
                bool control_down;
                bool delete_pressed;

                struct {
                    bool up_pressed;
                    bool down_pressed;
                    bool left_pressed;
                    bool right_pressed;
                } direction;

                struct {
                    bool z_pressed;
                    bool x_pressed;
                } key;

                bool shift_or_control_down;
                bool any_direction_pressed;

            } keyboard;

        } input = ZEROED;

        input.mouse.pos                           = GetMousePosition();
        input.mouse.left.clicked                  = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        input.mouse.left.down                     = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        input.keyboard.shift_down                 = IsKeyDown(KEY_LEFT_SHIFT)   || IsKeyDown(KEY_RIGHT_SHIFT);
        input.keyboard.control_down               = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        input.keyboard.delete_pressed             = IsKeyPressed(KEY_DELETE)    || IsKeyPressed(KEY_BACKSPACE);

        input.keyboard.direction.up_pressed       = IsKeyPressed(KEY_UP)        || IsKeyPressed(KEY_W);
        input.keyboard.direction.down_pressed     = IsKeyPressed(KEY_DOWN)      || IsKeyPressed(KEY_S);
        input.keyboard.direction.left_pressed     = IsKeyPressed(KEY_LEFT)      || IsKeyPressed(KEY_A);
        input.keyboard.direction.right_pressed    = IsKeyPressed(KEY_RIGHT)     || IsKeyPressed(KEY_D);

        input.keyboard.key.z_pressed              = IsKeyPressed(KEY_Z);
        input.keyboard.key.x_pressed              = IsKeyPressed(KEY_X);


        input.keyboard.shift_or_control_down      = input.keyboard.shift_down || input.keyboard.control_down;
        input.keyboard.any_direction_pressed      = input.keyboard.direction.up_pressed || input.keyboard.direction.down_pressed || input.keyboard.direction.left_pressed || input.keyboard.direction.right_pressed;



        toggle_when_pressed(&debug_draw_smaller_cell_hitbox,    KEY_F1);
        toggle_when_pressed(&debug_draw_cursor_position,        KEY_F2);


        local_persist bool in_solve_mode = true;
        {
            toggle_when_pressed(&in_solve_mode, KEY_B);

            const char *text = in_solve_mode ? "SOLVE"              : "BUILD";
            Color text_color = in_solve_mode ? FONT_COLOR_MARKING   : FONT_COLOR;
            Vector2 text_pos = { window_width/2, 10 + FONT_SIZE/2 };
            DrawTextCentered(GetFontWithSize(FONT_SIZE), text, text_pos, text_color);
        }


        ////////////////////////////////
        //        Selection
        ////////////////////////////////
        local_persist bool when_dragging_to_set_selected_to = true;
        if (!input.mouse.left.down) when_dragging_to_set_selected_to = true;


        local_persist s8 cursor_x = SUDOKU_SIZE / 2; // should be 4 (the middle)
        local_persist s8 cursor_y = SUDOKU_SIZE / 2; // should be 4 (the middle)
        {
            s8 prev_x = cursor_x;
            s8 prev_y = cursor_y;

            if (input.keyboard.direction.up_pressed   ) cursor_y -= 1;
            if (input.keyboard.direction.down_pressed ) cursor_y += 1;
            if (input.keyboard.direction.left_pressed ) cursor_x -= 1;
            if (input.keyboard.direction.right_pressed) cursor_x += 1;

            cursor_x = Proper_Mod(cursor_x, SUDOKU_SIZE);
            cursor_y = Proper_Mod(cursor_y, SUDOKU_SIZE);

            // cursor cannot go back over a selected cell.
            //
            // if shift is not pressed, and the cursor couldnt move to the cell,
            // but will be able to, it dosn't move.
            if (get_cell(&sudoku, cursor_x, cursor_y).ui->is_selected) {
                cursor_x = prev_x;
                cursor_y = prev_y;
            }

            if (debug_draw_cursor_position) {
                Rectangle rec = get_cell_bounds(&sudoku, cursor_x, cursor_y);
                DrawCircle(rec.x + rec.width/2, rec.y + rec.height/2, rec.height/3, ColorAlpha(RED, 0.5));
            }
        }


        ////////////////////////////////
        //       placeing digits
        ////////////////////////////////
        Sudoku_UI_Layer layer_to_place;
        if      (input.keyboard.shift_down && input.keyboard.control_down)  layer_to_place = SUL_COLOR;
        else if (input.keyboard.shift_down)                                 layer_to_place = SUL_UNCERTAIN;
        else if (input.keyboard.control_down)                               layer_to_place = SUL_CERTAIN;
        else                                                                layer_to_place = SUL_DIGIT;

        // determines wheather a number was pressed to put it into the grid.
        s8 number_pressed = NO_DIGIT_PLACED;
        u8 number_keys[] = {KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE};
        for (u32 i = 0; i < Array_Len(number_keys); i++) {
            if (IsKeyPressed(number_keys[i]))   number_pressed = i;
        }


        ////////////////////////////////
        //       removing digits
        ////////////////////////////////
        // make sure that all boxes get the same result after pressing a key (all add or all remove)
        bool remove_number_this_press = true;

        Sudoku_UI_Layer layer_to_delete = SUL_COLOR; // lowest priority



        ////////////////////////////////
        //         Undo / Redo
        ////////////////////////////////

        if (input.keyboard.control_down && input.keyboard.key.z_pressed) {
            if (sudoku.undo_buffer.count > 1) {
                sudoku.undo_buffer.count    -= 1;
                sudoku.redo_count           += 1;
                sudoku.grid = sudoku.undo_buffer.items[sudoku.undo_buffer.count - 1];
            }
        }

        if (input.keyboard.control_down && input.keyboard.key.x_pressed) { // TODO cntl-x should be cut, but we dont have that yet.
            if (sudoku.redo_count > 0) {
                sudoku.undo_buffer.count    += 1;
                sudoku.redo_count           -= 1;
                sudoku.grid = sudoku.undo_buffer.items[sudoku.undo_buffer.count - 1];
            }
        }



        ////////////////////////////////////////////////
        //            update sudoku grid
        ////////////////////////////////////////////////

        {
            ////////////////////////////////////////////////
            //              Selection stuff
            ////////////////////////////////////////////////

            // phase 1
            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell        = get_cell(&sudoku, i, j);
                    Rectangle   cell_bounds = get_cell_bounds(&sudoku, i, j);


                    // selected stuff
                    bool mouse_is_over = CheckCollisionPointRec(input.mouse.pos, cell_bounds);
                    cell.ui->is_hovering_over = mouse_is_over;

                    if (input.mouse.left.clicked) {
                        if (mouse_is_over) {
                            cursor_x = i; cursor_y = j; // put cursor wherever mouse is.

                            if (input.keyboard.shift_or_control_down) {
                                // start of deselection drag
                                cell.ui->is_selected = !cell.ui->is_selected; // TOGGLE
                            } else {
                                // just this one will be selected
                                cell.ui->is_selected = true;
                            }
                            when_dragging_to_set_selected_to = cell.ui->is_selected;
                        } else {
                            // only stay active if shift or cntl is down
                            cell.ui->is_selected = cell.ui->is_selected && input.keyboard.shift_or_control_down;
                        }
                    }

                    if (input.keyboard.any_direction_pressed) {
                        bool cursor_is_here = ((s8)i == cursor_x) && ((s8)j == cursor_y);
                        cell.ui->is_selected = cursor_is_here || (input.keyboard.shift_or_control_down && cell.ui->is_selected);
                    }

                }
            }


            // phase 2
            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell        = get_cell(&sudoku, i, j);
                    Rectangle   cell_bounds = get_cell_bounds(&sudoku, i, j);

                    // selected stuff, mouse dragging
                    Rectangle smaller_hitbox = ShrinkRectangle(cell_bounds, SUDOKU_CELL_SMALLER_HITBOX_SIZE);
                    if (debug_draw_smaller_cell_hitbox) DrawRectangleRec(smaller_hitbox, ColorAlpha(YELLOW, 0.4));

                    if (input.mouse.left.down && CheckCollisionPointRec(input.mouse.pos, smaller_hitbox)) {
                        cell.ui->is_selected = when_dragging_to_set_selected_to;
                        cursor_x = i; cursor_y = j; // move the cursor here as well.
                    }
                }
            }
        }



        ////////////////////////////////////////////////
        //              Placeing Digits
        ////////////////////////////////////////////////
        if (number_pressed != NO_DIGIT_PLACED) {
            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell = get_cell(&sudoku, i, j);
                    if (!cell.ui->is_selected) continue;


                    bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                    bool has_builder_digit = has_digit && !cell.marking->digit_placed_in_solve_mode;
                    bool slot_is_modifiable = (in_solve_mode && !has_builder_digit) || !in_solve_mode;

                    switch (layer_to_place) {
                        case SUL_DIGIT: {
                            if (slot_is_modifiable) remove_number_this_press = remove_number_this_press && (*cell.digit == number_pressed);
                        } break;
                        case SUL_CERTAIN: {
                            if (!has_digit) remove_number_this_press = remove_number_this_press && (cell.marking->  certain & (1 << (number_pressed)));
                        } break;
                        case SUL_UNCERTAIN: {
                            if (!has_digit) remove_number_this_press = remove_number_this_press && (cell.marking->uncertain & (1 << (number_pressed)));
                        } break;
                        case SUL_COLOR: { /* TODO("figure out what to Color the cell"); */ } break;
                    }
                }
            }

            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell = get_cell(&sudoku, i, j);
                    if (!cell.ui->is_selected) continue;


                    bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                    bool has_builder_digit = has_digit && !cell.marking->digit_placed_in_solve_mode;
                    bool slot_is_modifiable = (in_solve_mode && !has_builder_digit) || !in_solve_mode;

                    switch (layer_to_place) {
                    case SUL_DIGIT: {
                        if (slot_is_modifiable) {
                            if (remove_number_this_press) {
                                *cell.digit = NO_DIGIT_PLACED; // @Place_Digit
                                cell.marking->digit_placed_in_solve_mode = false;
                            } else {
                                *cell.digit = number_pressed;  // @Place_Digit
                                cell.marking->digit_placed_in_solve_mode = in_solve_mode;
                            }
                        }
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

                    case SUL_COLOR: { /* TODO("Color the cell"); */ } break;
                    }
                }
            }
        }


        ////////////////////////////////////////////////
        //             Removeing Digits
        ////////////////////////////////////////////////
        if (input.keyboard.delete_pressed) {
            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell = get_cell(&sudoku, i, j);
                    if (!cell.ui->is_selected) continue;

                    bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                    bool has_builder_digit = has_digit && !cell.marking->digit_placed_in_solve_mode;
                    bool slot_is_modifiable = (in_solve_mode && !has_builder_digit) || !in_solve_mode;

                    // TODO maybe if you have cntl press or something it dose something different?

                    if (has_digit && slot_is_modifiable)    layer_to_delete = Min(layer_to_delete, SUL_DIGIT);
                    if (cell.marking->  certain)            layer_to_delete = Min(layer_to_delete, SUL_CERTAIN);
                    if (cell.marking->uncertain)            layer_to_delete = Min(layer_to_delete, SUL_UNCERTAIN);
                    // Color is the lowest priority
                }
            }


            for (u32 j = 0; j < SUDOKU_SIZE; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                    Sudoku_Cell cell = get_cell(&sudoku, i, j);
                    if (!cell.ui->is_selected) continue;

                    bool has_digit = (*cell.digit != NO_DIGIT_PLACED);
                    bool has_builder_digit = has_digit && !cell.marking->digit_placed_in_solve_mode;
                    bool slot_is_modifiable = (in_solve_mode && !has_builder_digit) || !in_solve_mode;

                    switch (layer_to_delete) {
                    case SUL_DIGIT: {
                        if (slot_is_modifiable) {
                            *cell.digit = NO_DIGIT_PLACED; /* @Place_Digit */
                            cell.marking->digit_placed_in_solve_mode = false;
                        }
                    } break;
                    case SUL_CERTAIN:   { cell.marking->  certain = 0; } break;
                    case SUL_UNCERTAIN: { cell.marking->uncertain = 0; } break;
                    case SUL_COLOR: { /* printf("TODO: Color the cell\n"); */ } break;
                    }
                }
            }
        }


        ////////////////////////////////////////////////
        //             Undo / Redo
        ////////////////////////////////////////////////
        if (!Sudoku_Grid_Is_The_Same_As_The_Last_Element_In_The_Undo_Buffer(&sudoku)) {
            Array_Append(&sudoku.undo_buffer, sudoku.grid);
            sudoku.redo_count = 0;
        }



        ////////////////////////////////////////////////
        //             draw sudoku grid
        ////////////////////////////////////////////////
        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                Sudoku_Cell cell        = get_cell(&sudoku, i, j);
                Rectangle   cell_bounds = get_cell_bounds(&sudoku, i, j);

                {
                    Rectangle bit_bigger = GrowRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2);
                    DrawRectangleFrameRec(bit_bigger, SUDOKU_CELL_INNER_LINE_THICKNESS, SUDOKU_CELL_LINE_COLOR);
                }


                if (*cell.digit != NO_DIGIT_PLACED) {
                    // draw placed digit

                    // TODO do it smart
                    // char buf[2] = ZEROED;
                    // DrawTextCodepoint();
                    const char *text = TextFormat("%d", *cell.digit);

                    Vector2 text_position = { cell_bounds.x + SUDOKU_CELL_SIZE/2, cell_bounds.y + SUDOKU_CELL_SIZE/2 };
                    Color text_color = cell.marking->digit_placed_in_solve_mode ? FONT_COLOR_MARKING : FONT_COLOR;

                    DrawTextCentered(GetFontWithSize(FONT_SIZE), text, text_position, text_color);
                } else {
                    // draw uncertain and certain digits
                    Int_Array uncertain_numbers = { .allocator = scratch };
                    Int_Array certain_numbers   = { .allocator = scratch };

                    for (u8 k = 0; k <= 9; k++) {
                        if (cell.marking->uncertain & (1 << k)) Array_Append(&uncertain_numbers, k);
                        if (cell.marking->  certain & (1 << k)) Array_Append(&  certain_numbers, k);
                    }


                    // draw uncertain numbers around the edge of the box
                    if (uncertain_numbers.count) {
                        Font_And_Size font_and_size = GetFontWithSize(FONT_SIZE_UNCERTAIN); // what matters more? speed or binding energy?
                        for (u32 k = 0; k < uncertain_numbers.count; k++) {
                            // TODO do it smarter like above.
                            const char *text = TextFormat("%d", uncertain_numbers.items[k]);

                            Vector2 text_pos = { cell_bounds.x, cell_bounds.y + font_and_size.size/2 };
                            text_pos = Vector2Add(text_pos, MARKING_LOCATIONS[k]);
                            DrawTextCentered(font_and_size, text, text_pos, FONT_COLOR_MARKING);
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

                        Vector2 text_pos = { cell_bounds.x + SUDOKU_CELL_SIZE/2, cell_bounds.y + SUDOKU_CELL_SIZE/2 };
                        DrawTextCentered(GetFontWithSize(font_size), buf, text_pos, FONT_COLOR_MARKING);
                    }
                }


                // hovering and stuff
                if (cell.ui->is_selected) {
                    // drawing flowing selected
                    bool is_selected_up         =                         j == 0                ? false : get_cell(&sudoku, i  , j-1).ui->is_selected;
                    bool is_selected_down       =                         j == SUDOKU_SIZE - 1  ? false : get_cell(&sudoku, i  , j+1).ui->is_selected;
                    bool is_selected_left       = i == 0                                        ? false : get_cell(&sudoku, i-1, j  ).ui->is_selected;
                    bool is_selected_right      = i == SUDOKU_SIZE - 1                          ? false : get_cell(&sudoku, i+1, j  ).ui->is_selected;

                    bool is_selected_up_left    = i == 0               || j == 0                ? false : get_cell(&sudoku, i-1, j-1).ui->is_selected;
                    bool is_selected_up_right   = i == SUDOKU_SIZE - 1 || j == 0                ? false : get_cell(&sudoku, i+1, j-1).ui->is_selected;
                    bool is_selected_down_left  = i == 0               || j == SUDOKU_SIZE-1    ? false : get_cell(&sudoku, i-1, j+1).ui->is_selected;
                    bool is_selected_down_right = i == SUDOKU_SIZE - 1 || j == SUDOKU_SIZE-1    ? false : get_cell(&sudoku, i+1, j+1).ui->is_selected;


                    bool draw_line_up           = !is_selected_up;
                    bool draw_line_down         = !is_selected_down;
                    bool draw_line_left         = !is_selected_left;
                    bool draw_line_right        = !is_selected_right;

                    bool draw_line_up_left      = draw_line_up   || draw_line_left  || (!is_selected_up_left    && is_selected_up   && is_selected_left );
                    bool draw_line_up_right     = draw_line_up   || draw_line_right || (!is_selected_up_right   && is_selected_up   && is_selected_right);
                    bool draw_line_down_left    = draw_line_down || draw_line_left  || (!is_selected_down_left  && is_selected_down && is_selected_left );
                    bool draw_line_down_right   = draw_line_down || draw_line_right || (!is_selected_down_right && is_selected_down && is_selected_right);


                    Rectangle cell_bounds       = get_cell_bounds(&sudoku, i, j);
                    Rectangle select_bounds     = ShrinkRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2);

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
                    DrawRectangleRec(ShrinkRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2), ColorAlpha(BLACK, 0.2)); // cool trick // @Color
                }
            }
        }



        {
            // Lines that seperate the Boxes
            for (u32 j = 0; j < SUDOKU_SIZE/3; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE/3; i++) {
                    Rectangle ul_box = get_cell_bounds(&sudoku, i*3,     j*3    );
                    Rectangle dr_box = get_cell_bounds(&sudoku, i*3 + 2, j*3 + 2);

                    Rectangle region_box = {
                        ul_box.x,
                        ul_box.y,
                        (dr_box.x + dr_box.width ) - ul_box.x,
                        (dr_box.y + dr_box.height) - ul_box.y,
                    };

                    DrawRectangleFrameRec(
                        GrowRectangle(region_box, SUDOKU_CELL_BOARDER_LINE_THICKNESS - SUDOKU_CELL_INNER_LINE_THICKNESS/2),
                        SUDOKU_CELL_BOARDER_LINE_THICKNESS,
                        SUDOKU_BOX_LINE_COLOR
                    );
                }
            }

            { // this outer box hits 1 extra pixel.
                Rectangle ul_box = get_cell_bounds(&sudoku, 0, 0);
                Rectangle dr_box = get_cell_bounds(&sudoku, SUDOKU_SIZE-1, SUDOKU_SIZE-1);

                Rectangle region_box = {
                    ul_box.x,
                    ul_box.y,
                    (dr_box.x + dr_box.width ) - ul_box.x,
                    (dr_box.y + dr_box.height) - ul_box.y,
                };

                DrawRectangleFrameRec(
                    GrowRectangle(region_box, SUDOKU_CELL_BOARDER_LINE_THICKNESS),
                    SUDOKU_CELL_BOARDER_LINE_THICKNESS,
                    SUDOKU_BOX_LINE_COLOR
                );
            }
        }


        EndDrawing();
    }


    bool result = save_sudoku(autosave_path, &sudoku);

    UnloadDynamicFonts();

    CloseWindow();

    Pool_Free_Arenas(&pool);

    return result ? 0 : 1;
}





#define BESTED_IMPLEMENTATION
#include "Bested.h"
