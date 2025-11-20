#pragma once


// #include "Bested.h"

#include <raylib.h>
#include <raymath.h>




////////////////////////////////////////
//         Rectangle Helpers
////////////////////////////////////////


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

#define GrowRectangle(rec, value)       ShrinkRectangle((rec), -(value))



////////////////////////////////////////
//             Text Stuff
////////////////////////////////////////


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



// towards nearest number.
internal int Round(float x) {
    if (x >= 0) return (int)(x + 0.5);
    else        return (int)(x - 0.5);
}

internal void DrawTextCentered(Font_And_Size font_and_size, const char *text, Vector2 position, Color color) {
    Vector2 text_size = MeasureTextEx(font_and_size.font, text, font_and_size.size, 0);
    position.x -= text_size.x/2; // center text
    position.y -= text_size.y/2; // center text

    // if a font is drawn inbetween pixels, it looks dumb
    position.x = Round(position.x);
    position.y = Round(position.y);

    DrawTextEx(font_and_size.font, text, position, font_and_size.size, 0, color);
}

