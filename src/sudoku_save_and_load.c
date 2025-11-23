#pragma once






// returns NULL on succsess, otherwise returns an error string.
internal const char *load_sudoku(const char *filename, Sudoku *result);
// returns wheather or not the file was saved succsessfully.
internal bool save_sudoku(const char *filename, Sudoku *to_save);







///////////////////////////////////////////////////////////////////////////
//                          Save / Load Sudoku
///////////////////////////////////////////////////////////////////////////

#define MAX_TEMP_FILE_SIZE      (1 * MEGABYTE)
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

#define CURRENT_SUDOKU_SAVE_VERSION_NUMBER      2



struct Save_Header {
    Magic_Number    magic_number;
    Version_Number  version_number;
};











/////////////////////////////////////////////////////////////////
//                          Load Version 1
/////////////////////////////////////////////////////////////////


typedef struct {
    struct Save_Header header;

    #define SUDOKU_SIZE_VERSION_1           9
    s8  digits_on_the_grid[SUDOKU_SIZE_VERSION_1][SUDOKU_SIZE_VERSION_1];

    struct Markings_V1 {
        u16 uncertain;
        u16   certain;
    } digit_markings_on_the_grid[SUDOKU_SIZE_VERSION_1][SUDOKU_SIZE_VERSION_1];
} Sudoku_Save_Struct_Version_1;



// returns NULL on succsess, else returns error message
internal const char *load_sudoku_version_1(String file, Sudoku *result) {
    // String file = temp_Read_Entire_File(filename);

    Sudoku_Save_Struct_Version_1 *save_struct = (void*) file.data;
    ASSERT(save_struct->header.version_number == 1);

    // Check if the file is valid.

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
        // TODO this assert is still relevent down here,
        // because the new sudoku grid might do something weird.
        //
        // one way to remove it would be have this function produce a
        // different struct that can always be turned into a sudoku grid somewhere else...
        static_assert(CURRENT_SUDOKU_SAVE_VERSION_NUMBER == 2, "to change when new version");
        static_assert(SUDOKU_SIZE_VERSION_1 <= SUDOKU_SIZE, "would be pretty dumb to miss this...");

        for (u8 j = 0; j < Array_Len(save_struct->digits_on_the_grid); j++) {
            for (u8 i = 0; i < Array_Len(save_struct->digits_on_the_grid[j]); i++) {
                Sudoku_Cell cell = get_cell(result, i, j);

                *cell.digit = save_struct->digits_on_the_grid[j][i];

                cell.marking->digit_placed_in_solve_mode = false;
                cell.marking->uncertain = save_struct->digit_markings_on_the_grid[j][i].uncertain;
                cell.marking->  certain = save_struct->digit_markings_on_the_grid[j][i].  certain;
                cell.marking->color_bitfield = 0;

                // clear these also.
                cell.ui->is_selected = false;
                cell.ui->is_hovering_over = false;
            }
        }
    }

    return NULL;
}












/////////////////////////////////////////////////////////////////
//                          Load Version 2
/////////////////////////////////////////////////////////////////



#define SUDOKU_SIZE_VERSION_2           9

typedef struct {
    union Cell_Version_2 {
        struct {
            s8 digit;
            b8 placed_in_solve_mode;

            u16 uncertain;
            u16 certain;

            u32 color_bitfield;
        };

        u64 padding[2]; // this struct must be 2 pointers long.

    } cells[SUDOKU_SIZE_VERSION_2][SUDOKU_SIZE_VERSION_2];
    static_assert(sizeof(union Cell_Version_2) == sizeof(u64)*2, "cannot be bigger than 2 pointers");
} Sudoku_Grid_Version_2;

// packed so it never changes
typedef struct __attribute__((packed)) {
    struct Save_Header header;


    Sudoku_Grid_Version_2 grid;

    u32 reserved_for_color; // may want a background color;

    u32 reserved_for_array_1; // resurved for use as pointers to extra rules
    u32 reserved_for_array_2; // resurved for use as pointers to extra rules


    u32 name_offset_in_file;
    u32 name_length;

    u32 reserved_for_discription_1;
    u32 reserved_for_discription_2;


    u32 undo_buffer_offset_in_file;
    u32 undo_buffer_length;

    u32 redo_count;

} Sudoku_Save_Struct_Version_2;
// TODO
static_assert(sizeof(Sudoku_Save_Struct_Version_2) == 1344, "this size never changes");

static_assert(CURRENT_SUDOKU_SAVE_VERSION_NUMBER == 2, "to change when new version");




internal const char *load_sudoku_version_2(String file, Sudoku *result) {
    Sudoku_Save_Struct_Version_2 *save_struct = (void*)file.data;
    ASSERT(save_struct->header.version_number == 2);

    // Check if the file is valid.

    if (file.length < sizeof(Sudoku_Save_Struct_Version_2)) return temp_sprintf("file is smaller that save struct, can only be bigger, wanted (%ld), got (%ld)", sizeof(Sudoku_Save_Struct_Version_2), file.length);

    // TODO more checks


    if (result) {
        for (u32 j = 0; j < SUDOKU_SIZE_VERSION_2; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE_VERSION_2; i++) {
                Sudoku_Cell cell = get_cell(result, i, j);

                *cell.digit                                 = save_struct->grid.cells[j][i].digit;
                cell.marking->digit_placed_in_solve_mode    = save_struct->grid.cells[j][i].placed_in_solve_mode;
                cell.marking->uncertain                     = save_struct->grid.cells[j][i].uncertain;
                cell.marking->  certain                     = save_struct->grid.cells[j][i].  certain;
                cell.marking->color_bitfield                = save_struct->grid.cells[j][i].color_bitfield;
            }
        }


        ASSERT(save_struct->name_length <= SUDOKU_MAX_NAME_LENGTH);                         // TODO move this to validation
        ASSERT(file.length >= save_struct->name_length + save_struct->name_offset_in_file); // TODO move this to validation
        char *name_start = file.data + save_struct->name_offset_in_file;
        Mem_Copy(result->name_buf, name_start, save_struct->name_length);


        // TODO move this to validation
        ASSERT(file.length >= save_struct->name_offset_in_file + (save_struct->undo_buffer_length + save_struct->redo_count) * sizeof(Sudoku_Grid_Version_2));
        Sudoku_Grid_Version_2 *undo_buffer_array = (void*) (file.data + save_struct->name_offset_in_file);

        for (u32 k = 0; k < save_struct->undo_buffer_length + save_struct->redo_count; k++) {
            Sudoku_Grid_Version_2 *undo_grid = &undo_buffer_array[k];
            Sudoku_Grid grid = ZEROED;

            for (u32 j = 0; j < SUDOKU_SIZE_VERSION_2; j++) {
                for (u32 i = 0; i < SUDOKU_SIZE_VERSION_2; i++) {
                    Sudoku_Grid_Cell cell = get_grid_cell(&grid, i, j);

                    *cell.digit                                 = undo_grid->cells[j][i].digit;
                    cell.marking->digit_placed_in_solve_mode    = undo_grid->cells[j][i].placed_in_solve_mode;
                    cell.marking->uncertain                     = undo_grid->cells[j][i].uncertain;
                    cell.marking->  certain                     = undo_grid->cells[j][i].  certain;
                    cell.marking->color_bitfield                = undo_grid->cells[j][i].color_bitfield;
                }
            }

            Array_Append(&result->undo_buffer, grid);
        }

        result->undo_buffer.count  -= save_struct->redo_count;
        result->redo_count          = save_struct->redo_count;
    }
    return NULL;
}







/////////////////////////////////////////////////////////////////
//                             Load
/////////////////////////////////////////////////////////////////




internal const char *load_sudoku(const char *filename, Sudoku *result) {
    String file = temp_Read_Entire_File(filename);
    if (file.length == 0) return "Could not read file for some reason, or file was empty.";

    if (file.length < sizeof(struct Save_Header)) return "File was to small to contain header and version number";

    struct Save_Header *save_header = (void*) file.data;
    if (save_header->magic_number != SUDOKU_MAGIC_NUMBER) return temp_sprintf("Magic number was incorrect. wanted (0x%X), got (0x%X)", SUDOKU_MAGIC_NUMBER, save_header->magic_number);

    switch (save_header->version_number) {
        case 1: return load_sudoku_version_1(file, result);
        case 2: return load_sudoku_version_2(file, result);
        default: return temp_sprintf("Unknown version number (%d)", save_header->version_number);
    }
}




/////////////////////////////////////////////////////////////////
//                             Save
/////////////////////////////////////////////////////////////////

internal bool save_sudoku(const char *filename, Sudoku *to_save) {
    ASSERT(to_save);



    static_assert(CURRENT_SUDOKU_SAVE_VERSION_NUMBER == 2, "to change when new version");
    printf("Saving sudoku version %d\n", CURRENT_SUDOKU_SAVE_VERSION_NUMBER);

    Sudoku_Save_Struct_Version_2 save_struct = ZEROED;
    save_struct.header.magic_number = SUDOKU_MAGIC_NUMBER;
    save_struct.header.version_number = CURRENT_SUDOKU_SAVE_VERSION_NUMBER;


    static_assert(SUDOKU_SIZE_VERSION_2 <= SUDOKU_SIZE, "Would hit an assert here");
    for (u8 j = 0; j < Array_Len(save_struct.grid.cells); j++) {
        for (u8 i = 0; i < Array_Len(save_struct.grid.cells[j]); i++) {
            Sudoku_Cell cell = get_cell(to_save, i, j);

            save_struct.grid.cells[j][i].digit = *cell.digit;
            save_struct.grid.cells[j][i].placed_in_solve_mode    = cell.marking->digit_placed_in_solve_mode;
            save_struct.grid.cells[j][i].uncertain               = cell.marking->uncertain;
            save_struct.grid.cells[j][i].  certain               = cell.marking->  certain;
            save_struct.grid.cells[j][i].color_bitfield          = cell.marking->color_bitfield;
        }
    }



    save_struct.name_offset_in_file         = sizeof(save_struct);
    save_struct.name_length                 = to_save->name_buf_count;
    save_struct.undo_buffer_offset_in_file  = sizeof(save_struct) + to_save->name_buf_count;
    save_struct.undo_buffer_length          = to_save->undo_buffer.count;
    save_struct.redo_count                  = to_save->redo_count;


    // reserved stuff
    save_struct.reserved_for_color         = 0;
    save_struct.reserved_for_array_1       = 0;
    save_struct.reserved_for_array_2       = 0;
    save_struct.reserved_for_discription_1 = 0;
    save_struct.reserved_for_discription_2 = 0;


    String_Builder sb = ZEROED;
    sb.allocator = Scratch_Get();
    String_Builder_Ptr_And_Size(&sb, (void*)&save_struct, sizeof(save_struct));

    String_Builder_Ptr_And_Size(&sb, to_save->name_buf, to_save->name_buf_count);


    for (u32 k = 0; k < to_save->undo_buffer.count + to_save->redo_count; k++) {
        Sudoku_Grid *grid = &to_save->undo_buffer.items[k];

        Sudoku_Grid_Version_2 undo_grid = ZEROED;

        for (u32 j = 0; j < SUDOKU_SIZE_VERSION_2; j++) {
            for (u32 i = 0; i < SUDOKU_SIZE_VERSION_2; i++) {
                Sudoku_Grid_Cell cell = get_grid_cell(grid, i, j);
                // TODO use get_cell();
                undo_grid.cells[j][i].digit                = *cell.digit;
                undo_grid.cells[j][i].placed_in_solve_mode = cell.marking->digit_placed_in_solve_mode;
                undo_grid.cells[j][i].uncertain            = cell.marking->uncertain;
                undo_grid.cells[j][i].certain              = cell.marking->certain;
                undo_grid.cells[j][i].color_bitfield       = cell.marking->color_bitfield;
            }
        }

        String_Builder_Ptr_And_Size(&sb, (void*)&undo_grid, sizeof(undo_grid));
    }

    // Would be kinda awkward, we cant load this...
    u64 size_of_file = String_Builder_Count(&sb);
    if (size_of_file > MAX_TEMP_FILE_SIZE) {
        fprintf(stderr, "ERROR: file is to big to fit into temperary buffer, is %.2fMB (%lu)\n", (f64)size_of_file / (f64)MEGABYTE, size_of_file);
    }


    bool result = false;
    FILE *f = fopen(filename, "wb");
    if (f) {
        String_Builder_To_File(&sb, f);
        fclose(f);
        result = true;
    }

    Scratch_Release(sb.allocator);
    return result && size_of_file < MAX_TEMP_FILE_SIZE;
}

