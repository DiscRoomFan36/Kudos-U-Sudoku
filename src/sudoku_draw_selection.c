#pragma once


///////////////////////////////////////////////////////////////////////////
//                         Selected Animation Header
///////////////////////////////////////////////////////////////////////////


typedef struct {
    // how far along in the animation it is.
    // [0..1]
    f64 t_animation;

    Sudoku_UI_Grid prev_ui_state;
    Sudoku_UI_Grid curr_ui_state;

} Selected_Animation;

typedef struct {
    _Array_Header_;
    Selected_Animation *items;
} Selected_Animation_Array;


// the main function for this file.
void draw_sudoku_selection(Sudoku *sudoku, Selected_Animation *animation);







//////////////////////////////////////////
//             Directions
//////////////////////////////////////////

typedef enum {
    DIR_UP    = 0,
    DIR_RIGHT = 1,
    DIR_DOWN  = 2,
    DIR_LEFT  = 3,
} Direction;

internal Direction Opposite_Direction(Direction dir) {
    switch (dir) {
        case DIR_UP:    return DIR_DOWN;
        case DIR_DOWN:  return DIR_UP;
        case DIR_LEFT:  return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
    }
    UNREACHABLE();
}

internal Direction Next_Direction_Clockwise(Direction dir)         { return (dir + 1) % 4; }
internal Direction Next_Direction_Counter_Clockwise(Direction dir) { return (dir + 3) % 4; }



//////////////////////////////////////////
//          Rectangle Helpers
//////////////////////////////////////////

internal Rectangle Grow_Rectangle_In_Direction(Rectangle rec, Direction direction, f32 factor) {
    switch (direction) {
        case DIR_UP:    {
            rec.y += rec.height * (1-factor);
            rec.height *= factor;
        } break;

        case DIR_LEFT:  {
            rec.x += rec.width  * (1-factor);
            rec.width  *= factor;
        } break;

        case DIR_DOWN:  { rec.height *= factor; } break;
        case DIR_RIGHT: { rec.width  *= factor; } break;
    }

    return rec;
}


internal Rectangle Rectangle_Shrink_Both_Sides(Rectangle rec, Direction dir, f32 factor) {
    switch (dir) {
        case DIR_UP:
        case DIR_DOWN: {
            Rectangle final_rec = Grow_Rectangle_In_Direction(rec, DIR_UP, factor);
            final_rec.y -= rec.height/2 - final_rec.height/2;
            return final_rec;
        }

        case DIR_LEFT:
        case DIR_RIGHT: {
            Rectangle final_rec = Grow_Rectangle_In_Direction(rec, DIR_LEFT, factor);
            final_rec.x -= rec.width/2 - final_rec.width/2;
            return final_rec;
        }
    }

    UNREACHABLE();
}




//////////////////////////////////////////
//        Surrounding Helpers
//////////////////////////////////////////

// this struct is 8 bytes, probably dosnt matter for my perposes to pack these.
//
// starts at up and moves clockwise.
typedef union {
    struct {
        bool up;
        bool up_right;

        bool right;

        bool down_right;
        bool down;
        bool down_left;

        bool left;
        bool up_left;
    };

    bool as_array[8];
} Surrounding_Bools;

static_assert(sizeof(Surrounding_Bools) == Member_Size(Surrounding_Bools, as_array), "union must be correctly sized.");


internal Surrounding_Bools surrounding_bools_all_as(bool this) {
    Surrounding_Bools result;
    for (u32 i = 0; i < Array_Len(result.as_array); i++)    result.as_array[i] = this;
    return result;
}





internal Surrounding_Bools get_surrounding_is_selected(Sudoku_UI_Grid *ui, u8 i, u8 j) {
    ASSERT(ui);
    ASSERT_VALID_SUDOKU_ADDRESS(i, j);
    Surrounding_Bools result = {
        .up         =                         j == 0                ? false : ui->grid[j-1][i  ].is_selected,
        .down       =                         j == SUDOKU_SIZE - 1  ? false : ui->grid[j+1][i  ].is_selected,
        .left       = i == 0                                        ? false : ui->grid[j  ][i-1].is_selected,
        .right      = i == SUDOKU_SIZE - 1                          ? false : ui->grid[j  ][i+1].is_selected,

        .up_left    = i == 0               || j == 0                ? false : ui->grid[j-1][i-1].is_selected,
        .up_right   = i == SUDOKU_SIZE - 1 || j == 0                ? false : ui->grid[j-1][i+1].is_selected,
        .down_left  = i == 0               || j == SUDOKU_SIZE-1    ? false : ui->grid[j+1][i-1].is_selected,
        .down_right = i == SUDOKU_SIZE - 1 || j == SUDOKU_SIZE-1    ? false : ui->grid[j+1][i+1].is_selected,
    };
    return result;
}

internal Surrounding_Bools surrounding_is_selected_to_draw_lines(Surrounding_Bools is_selected) {
    Surrounding_Bools result = {
        .up           = !is_selected.up,
        .down         = !is_selected.down,
        .left         = !is_selected.left,
        .right        = !is_selected.right,

        .up_left      = !is_selected.up   || !is_selected.left  || (!is_selected.up_left    && is_selected.up   && is_selected.left ),
        .up_right     = !is_selected.up   || !is_selected.right || (!is_selected.up_right   && is_selected.up   && is_selected.right),
        .down_left    = !is_selected.down || !is_selected.left  || (!is_selected.down_left  && is_selected.down && is_selected.left ),
        .down_right   = !is_selected.down || !is_selected.right || (!is_selected.down_right && is_selected.down && is_selected.right),
    };
    return result;
}



//////////////////////////////////////////
//     Surrounding + Direction
//////////////////////////////////////////


internal bool *get_surrounding_bool_direction(Surrounding_Bools *bools, Direction dir) {
    switch (dir) {
        case DIR_UP:    return &bools->up;
        case DIR_DOWN:  return &bools->down;
        case DIR_LEFT:  return &bools->left;
        case DIR_RIGHT: return &bools->right;
    }
    UNREACHABLE();
}

// from [0..7]
// internal s32 get_direction_to_surrounding_bool_index(Direction dir) {
//     Surrounding_Bools bools;

//     bool *ptr = get_surrounding_bool_direction(&bools, dir);
//     s32 index = (s32)(ptr - &bools.up);
//     ASSERT(Is_Between(index, 0, (s32)Array_Len(bools.as_array)-1));
//     return index;
// }

internal bool *get_corner_between(Surrounding_Bools *bools, Direction dir_1, Direction dir_2) {
    ASSERT(bools);
    ASSERT(dir_1 != dir_2);
    ASSERT(Opposite_Direction(dir_1) != dir_2);

    if (dir_1 > dir_2) Swap(dir_1, dir_2);

    if (dir_1 == DIR_UP) {
        if      (dir_2 == DIR_RIGHT)    return &bools->up_right;
        else if (dir_2 == DIR_LEFT)     return &bools->up_left;
    } else if (dir_1 == DIR_RIGHT) {
        if      (dir_2 == DIR_DOWN)     return &bools->down_right;
    } else if (dir_1 == DIR_DOWN) {
        if      (dir_2 == DIR_LEFT)     return &bools->down_left;
    }
    UNREACHABLE();
}

internal s32 get_corner_between_index(Direction dir_1, Direction dir_2) {
    Surrounding_Bools bools;

    bool *corner = get_corner_between(&bools, dir_1, dir_2);
    s32 index = (s32)(corner - &bools.up);
    ASSERT(Is_Between(index, 0, (s32)Array_Len(bools.as_array)-1));
    return index;
}





//////////////////////////////////////////
//           Drawing Helpers
//////////////////////////////////////////


internal void draw_selected_lines(Rectangle bounds, f32 thickness, Surrounding_Bools draw_lines, Color color) {
    // orthoganal
    Rectangle line_up           = { bounds.x + thickness,                   bounds.y,                               bounds.width - thickness*2, thickness,                   };
    Rectangle line_down         = { bounds.x + thickness,                   bounds.y + bounds.height - thickness,   bounds.width - thickness*2, thickness,                   };
    Rectangle line_left         = { bounds.x,                               bounds.y + thickness,                   thickness,                  bounds.height - thickness*2, };
    Rectangle line_right        = { bounds.x + bounds.width - thickness,    bounds.y + thickness,                   thickness,                  bounds.height - thickness*2, };
    // diagonal
    Rectangle line_up_left      = { bounds.x,                               bounds.y,                               thickness,                  thickness,                   };
    Rectangle line_up_right     = { bounds.x + bounds.width - thickness,    bounds.y,                               thickness,                  thickness,                   };
    Rectangle line_down_left    = { bounds.x,                               bounds.y + bounds.height - thickness,   thickness,                  thickness,                   };
    Rectangle line_down_right   = { bounds.x + bounds.width - thickness,    bounds.y + bounds.height - thickness,   thickness,                  thickness,                   };


    RectangleRemoveNegatives(&line_up);
    RectangleRemoveNegatives(&line_down);
    RectangleRemoveNegatives(&line_left);
    RectangleRemoveNegatives(&line_right);

    RectangleRemoveNegatives(&line_up_left);
    RectangleRemoveNegatives(&line_up_right);
    RectangleRemoveNegatives(&line_down_left);
    RectangleRemoveNegatives(&line_down_right);


    ClipRectangleAIntoRectangleB(bounds, &line_up);
    ClipRectangleAIntoRectangleB(bounds, &line_down);
    ClipRectangleAIntoRectangleB(bounds, &line_left);
    ClipRectangleAIntoRectangleB(bounds, &line_right);

    ClipRectangleAIntoRectangleB(bounds, &line_up_left);
    ClipRectangleAIntoRectangleB(bounds, &line_up_right);
    ClipRectangleAIntoRectangleB(bounds, &line_down_left);
    ClipRectangleAIntoRectangleB(bounds, &line_down_right);


    if (draw_lines.up        )   DrawRectangleRec(line_up,         color); //YELLOW);
    if (draw_lines.down      )   DrawRectangleRec(line_down,       color); //RED);
    if (draw_lines.left      )   DrawRectangleRec(line_left,       color); //PURPLE);
    if (draw_lines.right     )   DrawRectangleRec(line_right,      color); //GREEN);

    if (draw_lines.up_left   )   DrawRectangleRec(line_up_left,    color); //MAROON);
    if (draw_lines.up_right  )   DrawRectangleRec(line_up_right,   color); //ORANGE);
    if (draw_lines.down_left )   DrawRectangleRec(line_down_left,  color); //GOLD);
    if (draw_lines.down_right)   DrawRectangleRec(line_down_right, color); //PINK);
}

internal void draw_selected_lines_based_on_surrounding_is_selected(Rectangle bounds, f32 thickness, Surrounding_Bools is_selected, Color color) {
    Surrounding_Bools draw_lines = surrounding_is_selected_to_draw_lines(is_selected);
    draw_selected_lines(bounds, thickness, draw_lines, color);
}





//////////////////////////////////////////////////////////////////
//        The Big One  /  Main  /  Draw Sudoku Selection
//////////////////////////////////////////////////////////////////

void draw_sudoku_selection(Sudoku *sudoku, Selected_Animation *animation) {
    ASSERT(sudoku);

    if (!animation) {
        Sudoku_UI_Grid *ui = &sudoku->ui;
        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {
                if (!cell_is_selected(ui, i, j)) continue;

                Surrounding_Bools surrounding_is_selected = get_surrounding_is_selected(ui, i, j);
                Surrounding_Bools draw_lines = surrounding_is_selected_to_draw_lines(surrounding_is_selected);

                Rectangle cell_bounds       = get_cell_bounds(sudoku, i, j);
                Rectangle select_bounds     = ShrinkRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2);

                Color color = SELECT_HIGHLIGHT_COLOR;

                draw_selected_lines(select_bounds, SELECT_LINE_THICKNESS, draw_lines, color);
            }
        }

        return;
    }

    ASSERT(animation);

    Sudoku_UI_Grid *curr_ui = &animation->curr_ui_state;
    Sudoku_UI_Grid *prev_ui = &animation->prev_ui_state;

    Surrounding_Bools curr_surrounding_is_selected_grid[SUDOKU_SIZE][SUDOKU_SIZE];
    Surrounding_Bools prev_surrounding_is_selected_grid[SUDOKU_SIZE][SUDOKU_SIZE];
    for (u32 j = 0; j < SUDOKU_SIZE; j++) {
        for (u32 i = 0; i < SUDOKU_SIZE; i++) {
            curr_surrounding_is_selected_grid[j][i] = get_surrounding_is_selected(curr_ui, i, j);
            prev_surrounding_is_selected_grid[j][i] = get_surrounding_is_selected(prev_ui, i, j);
        }
    }


    // not selected loop, for animation
    {
        f64 t = animation->t_animation;
        f64 factor = sqrt(t);

        for (u32 j = 0; j < SUDOKU_SIZE; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE; i++) {

                // if it is not selected this ui, and was selected in the previous ui,
                // fade it out.
                if (cell_is_selected(curr_ui, i, j) == true)  continue;
                if (cell_is_selected(prev_ui, i, j) == false) continue;

                // current_surrounding_is_selected
                Surrounding_Bools csis = curr_surrounding_is_selected_grid[j][i];
                // previous_surrounding_is_selected
                Surrounding_Bools psis = prev_surrounding_is_selected_grid[j][i];

                Rectangle cell_bounds       = get_cell_bounds(sudoku, i, j);
                Rectangle select_bounds     = ShrinkRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2);

                Color color = SELECT_HIGHLIGHT_COLOR;

                bool prev_no_surrunding_selected = !psis.up && !psis.right && !psis.down && !psis.left;
                bool curr_no_surrunding_selected = !csis.up && !csis.right && !csis.down && !csis.left;

                bool curr_only_up                =  csis.up && !csis.right && !csis.down && !csis.left;
                bool curr_only_right             = !csis.up &&  csis.right && !csis.down && !csis.left;
                bool curr_only_down              = !csis.up && !csis.right &&  csis.down && !csis.left;
                bool curr_only_left              = !csis.up && !csis.right && !csis.down &&  csis.left;

                if (prev_no_surrunding_selected && curr_no_surrunding_selected) {
                    // idealy we would want all massive selections to shrink into themseleves,
                    // but i have no idea how to do that...
                    //
                    // grab all lines, make bounding box, shrink bounding box.
                    // make all lines be within the box...
                    //
                    // TODO this could work
                    select_bounds = ShrinkRectanglePercent(select_bounds, 1-factor);

                } else if (prev_no_surrunding_selected && curr_only_up)    {
                    select_bounds.height  *= 1-factor;
                    psis.up = true; // remove top edge of the select
                    // remove the down edge of the thing up.
                    curr_surrounding_is_selected_grid[j-1][i  ].down = true;

                } else if (prev_no_surrunding_selected && curr_only_right) {
                    select_bounds.x += select_bounds.width  * factor;
                    select_bounds.width  *= 1-factor;
                    psis.right = true; // remove right edge of the select
                    // remove the left edge of the thing right.
                    curr_surrounding_is_selected_grid[j  ][i+1].left = true;

                } else if (prev_no_surrunding_selected && curr_only_down)  {
                    select_bounds.y += select_bounds.height * factor;
                    select_bounds.height *= 1-factor;
                    psis.down = true; // remove down edge of the select
                    // remove the up edge of the thing down.
                    curr_surrounding_is_selected_grid[j+1][i  ].up = true;

                } else if (prev_no_surrunding_selected && curr_only_left)  {
                    select_bounds.width  *= 1-factor;
                    psis.left = true; // remove left edge of the select
                    // remove the right edge of the thing left.
                    curr_surrounding_is_selected_grid[j  ][i-1].right = true;


                } else {
                    // else just fade away.
                    // the big shrink is comeing. run
                    color = Fade(color, 1-factor);
                }


                draw_selected_lines_based_on_surrounding_is_selected(select_bounds, SELECT_LINE_THICKNESS, psis, color);
            }
        }
    }



    // selected loop
    for (u32 j = 0; j < SUDOKU_SIZE; j++) {
        for (u32 i = 0; i < SUDOKU_SIZE; i++) {
            if (!cell_is_selected(curr_ui, i, j)) continue;

            Surrounding_Bools csis = curr_surrounding_is_selected_grid[j][i];

            Rectangle cell_bounds       = get_cell_bounds(sudoku, i, j);
            Rectangle select_bounds     = ShrinkRectangle(cell_bounds, SUDOKU_CELL_INNER_LINE_THICKNESS/2);

            Color color = SELECT_HIGHLIGHT_COLOR;

            // if it was the same as last time, just render them normally.
            if (cell_is_selected(prev_ui, i, j)) {
                draw_selected_lines_based_on_surrounding_is_selected(select_bounds, SELECT_LINE_THICKNESS, csis, color);
                continue;
            }

            // if it was not selected on the previous ui, do some animation
            ASSERT(!cell_is_selected(prev_ui, i, j));

            // previous_surrounding_is_selected
            Surrounding_Bools psis = prev_surrounding_is_selected_grid[j][i];

            f64 t = animation->t_animation;
            f64 factor = sqrt(t);

            bool draw_selected_bounds_at_the_end = true;

            enum {
                UP    = (1<<0), // 1
                RIGHT = (1<<1), // 2
                DOWN  = (1<<2), // 4
                LEFT  = (1<<3), // 8
            };

            u32 prev_selected = 0;
            if (psis.up)    prev_selected |= UP;
            if (psis.right) prev_selected |= RIGHT;
            if (psis.down)  prev_selected |= DOWN;
            if (psis.left)  prev_selected |= LEFT;

            switch (prev_selected) {
                case 0:     { //  0
                    // grow from nothing
                    select_bounds = ShrinkRectanglePercent(select_bounds, factor);
                    color = Fade(color, factor);
                } break;

                case UP:    /*  1 */ { select_bounds = Grow_Rectangle_In_Direction(select_bounds, DIR_DOWN,  factor); } break;
                case LEFT:  /*  8 */ { select_bounds = Grow_Rectangle_In_Direction(select_bounds, DIR_RIGHT, factor); } break;
                case DOWN:  /*  2 */ { select_bounds = Grow_Rectangle_In_Direction(select_bounds, DIR_UP,    factor); } break;
                case RIGHT: /*  4 */ { select_bounds = Grow_Rectangle_In_Direction(select_bounds, DIR_LEFT,  factor); } break;


                case UP   | DOWN:    // 5
                case LEFT | RIGHT: { // 10
                    // draw closeing from both sides
                    draw_selected_bounds_at_the_end = false;

                    Direction dir     = prev_selected & UP ? DIR_UP : DIR_LEFT;
                    Direction opp_dir = Opposite_Direction(dir);

                    {
                        // grows up / left
                        Surrounding_Bools draw_lines = surrounding_bools_all_as(true);
                        *get_surrounding_bool_direction(&draw_lines, opp_dir) = false;

                        Rectangle rec = Grow_Rectangle_In_Direction(select_bounds, dir, factor/2);

                        draw_selected_lines(rec, SELECT_LINE_THICKNESS, draw_lines, color);
                    }


                    {
                        // grows down / right
                        Surrounding_Bools draw_lines = surrounding_bools_all_as(true);
                        *get_surrounding_bool_direction(&draw_lines, dir) = false;

                        Rectangle rec = Grow_Rectangle_In_Direction(select_bounds, opp_dir, factor/2);

                        draw_selected_lines(rec, SELECT_LINE_THICKNESS, draw_lines, color);
                    }
                } break;


                case UP   | RIGHT:   //  3
                case DOWN | RIGHT:   //  6
                case UP   | LEFT:    //  9
                case DOWN | LEFT:  { // 12

                    // draw two sides comming together
                    draw_selected_bounds_at_the_end = false;

                    Direction dir;
                    switch (prev_selected) {
                        case UP   | RIGHT: { dir = DIR_UP;    } break;
                        case UP   | LEFT:  { dir = DIR_LEFT;  } break;
                        case DOWN | RIGHT: { dir = DIR_RIGHT; } break;
                        case DOWN | LEFT:  { dir = DIR_DOWN;  } break;
                        default: UNREACHABLE();
                    }


                    Direction next_dir = Next_Direction_Clockwise(dir);

                    Direction opp_dir      = Opposite_Direction(dir);
                    Direction opp_next_dir = Opposite_Direction(next_dir);


                    Surrounding_Bools draw_lines = surrounding_is_selected_to_draw_lines(csis);
                    {
                        Surrounding_Bools draw_lines_extra = surrounding_bools_all_as(false);

                        s32 index_corner = get_corner_between_index(dir, next_dir);

                        draw_lines_extra.as_array[index_corner] = draw_lines.as_array[index_corner];

                        draw_selected_lines(select_bounds, SELECT_LINE_THICKNESS, draw_lines_extra, color);

                        draw_lines.as_array[index_corner] = false;
                    }


                    // so the lines cross each other, and look nice.
                    // also dont want the lines to shrink to zero with 1-factor,
                    f64 line_thickness_over_width = SELECT_LINE_THICKNESS / (f64)select_bounds.width;
                    f64 inner_side_factor = Remap(factor, 0, 1, 1 + line_thickness_over_width, line_thickness_over_width);

                    {
                        Rectangle outer_rec = Grow_Rectangle_In_Direction(select_bounds, opp_dir, factor);
                        Rectangle final_rec = Grow_Rectangle_In_Direction(outer_rec, next_dir, inner_side_factor);

                        draw_selected_lines(final_rec, SELECT_LINE_THICKNESS, draw_lines, color);
                    }

                    {
                        Rectangle outer_rec = Grow_Rectangle_In_Direction(select_bounds, opp_next_dir, factor);
                        Rectangle final_rec = Grow_Rectangle_In_Direction(outer_rec, dir, inner_side_factor);

                        draw_selected_lines(final_rec, SELECT_LINE_THICKNESS, draw_lines, color);
                    }

                } break;


                case UP   | DOWN | RIGHT:   //  7
                case UP   | DOWN | LEFT:    // 13
                case UP   | LEFT | RIGHT:   // 11
                case DOWN | LEFT | RIGHT: { // 14
                    // close in from 3 sides
                    draw_selected_bounds_at_the_end = false;

                    Direction dir_middle;
                    switch (prev_selected) {
                        case UP   | DOWN | RIGHT: { dir_middle = DIR_RIGHT; } break;
                        case UP   | DOWN | LEFT:  { dir_middle = DIR_LEFT;  } break;
                        case UP   | LEFT | RIGHT: { dir_middle = DIR_UP;    } break;
                        case DOWN | LEFT | RIGHT: { dir_middle = DIR_DOWN;  } break;
                        default: UNREACHABLE();
                    }


                    Direction dir_left   = Next_Direction_Counter_Clockwise(dir_middle);
                    Direction dir_right  = Next_Direction_Clockwise        (dir_middle);
                    Direction dir_opp    = Opposite_Direction              (dir_middle);


                    Surrounding_Bools draw_lines = surrounding_is_selected_to_draw_lines(csis);
                    {
                        Surrounding_Bools draw_lines_the_small_corners = surrounding_bools_all_as(false);

                        s32 index_corner_middle_right = get_corner_between_index(dir_middle, dir_right);
                        s32 index_corner_middle_left  = get_corner_between_index(dir_middle, dir_left);

                        draw_lines_the_small_corners.as_array[index_corner_middle_right] = draw_lines.as_array[index_corner_middle_right];
                        draw_lines_the_small_corners.as_array[index_corner_middle_left]  = draw_lines.as_array[index_corner_middle_left];

                        draw_selected_lines(select_bounds, SELECT_LINE_THICKNESS, draw_lines_the_small_corners, color);

                        draw_lines.as_array[index_corner_middle_right] = false;
                        draw_lines.as_array[index_corner_middle_left]  = false;
                    }


                    // so the corners touch.
                    f64 line_thickness_over_width = SELECT_LINE_THICKNESS / (f64)select_bounds.width;

                    f64 half_factor = Remap(factor, 0, 1, 0, 0.5 + line_thickness_over_width/2);

                    f64 inner_side_factor = Remap(factor, 0, 1, 1 + line_thickness_over_width, 0.5 + line_thickness_over_width);

                    { // left
                        Surrounding_Bools this_draw_lines = draw_lines;
                        *get_surrounding_bool_direction(&this_draw_lines, dir_left) = true;
                        *get_corner_between(&this_draw_lines, dir_middle, dir_left) = true;

                        Rectangle outer_rec = Grow_Rectangle_In_Direction(select_bounds, dir_left,   half_factor);
                        Rectangle final_rec = Grow_Rectangle_In_Direction(outer_rec,     dir_middle, inner_side_factor);

                        draw_selected_lines(final_rec, SELECT_LINE_THICKNESS, this_draw_lines, color);
                    }

                    { // right
                        Surrounding_Bools this_draw_lines = draw_lines;
                        *get_surrounding_bool_direction(&this_draw_lines, dir_right) = true;
                        *get_corner_between(&this_draw_lines, dir_middle, dir_right) = true;

                        Rectangle outer_rec = Grow_Rectangle_In_Direction(select_bounds, dir_right,  half_factor);
                        Rectangle final_rec = Grow_Rectangle_In_Direction(outer_rec,     dir_middle, inner_side_factor);

                        draw_selected_lines(final_rec, SELECT_LINE_THICKNESS, this_draw_lines, color);
                    }


                    { // middle
                        // this is to meet in the middle with the others
                        Rectangle outer_rec = Grow_Rectangle_In_Direction(select_bounds, dir_opp, factor/2);
                        // // this is to shrink down to nothing.
                        Rectangle final_rec = Rectangle_Shrink_Both_Sides(outer_rec, dir_left, 1-factor);

                        draw_selected_lines(final_rec, SELECT_LINE_THICKNESS, draw_lines, color);
                    }


                } break;


                case UP | DOWN | RIGHT | LEFT: { // 15
                    // draw a shinking Rect that makes it look like the outside is sinking in.
                    Surrounding_Bools draw_lines = surrounding_bools_all_as(true);
                    Rectangle shrinking_rectangle = ShrinkRectanglePercent(select_bounds, 1-factor);
                    draw_selected_lines(shrinking_rectangle, SELECT_LINE_THICKNESS, draw_lines, color);
                } break;


                default: { UNREACHABLE(); } break;
            }

            if (draw_selected_bounds_at_the_end) {
                draw_selected_lines_based_on_surrounding_is_selected(select_bounds, SELECT_LINE_THICKNESS, csis, color);
            }
        }
    }


}

