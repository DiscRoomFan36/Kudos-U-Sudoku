#pragma once


// #include "Bested.h"

#include <raylib.h>
#include <raymath.h>


typedef union Line {
    struct { Vector2 start, end; };
    struct { float x1, y1, x2, y2; };
} Line;

#define CheckCollisionLinesL(l1, l2, hit_location)      CheckCollisionLines((l1).start, (l1).end, (l2).start, (l2).end, (hit_location))


////////////////////////////////////////
//         Rectangle Helpers
////////////////////////////////////////

internal inline Rectangle ShrinkRectangle(Rectangle rec, float value) {
    Rectangle result = {
        rec.x + value, rec.y + value,
        rec.width   - value*2,
        rec.height  - value*2,
    };
    return result;
}

#define GrowRectangle(rec, value)       ShrinkRectangle((rec), -(value))


internal inline Vector2 RectangleTopLeft(Rectangle rec) {
    Vector2 result = {rec.x, rec.y};
    return result;
}

Vector2 RectangleCenter(Rectangle rec) {
    Vector2 result = {rec.x + rec.width, rec.y + rec.height};
    return result;
}


// returns a static list of 4 lines, clears on every call,
// could fix that though
internal Line *RectangleToLines(Rectangle rec) {
    local_persist Line result[4];
    result[0] = (Line){.start = {rec.x,             rec.y             }, .end = {rec.x + rec.width, rec.y             }};
    result[1] = (Line){.start = {rec.x + rec.width, rec.y             }, .end = {rec.x + rec.width, rec.y + rec.height}};
    result[2] = (Line){.start = {rec.x + rec.width, rec.y + rec.height}, .end = {rec.x,             rec.y + rec.height}};
    result[3] = (Line){.start = {rec.x,             rec.y + rec.height}, .end = {rec.x,             rec.y             }};
    return result;
}




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




////////////////////////////////////////
//             Textures
////////////////////////////////////////


#define DrawTextureRightsideUp(texture, x, y)      DrawTextureRightsideUpV((texture), (Vector2){(x), (y)})

internal void DrawTextureRightsideUpV(Texture texture, Vector2 position) {
    Rectangle sourceRec = { 0, 0, texture.width, -texture.height };
    Rectangle destRec   = { position.x, position.y, texture.width, texture.height };
    DrawTexturePro(texture, sourceRec, destRec, Vector2Zero(), 0, WHITE);
}



////////////////////////////////////////
//             Triangle
////////////////////////////////////////

internal bool PointsAreCounterClockwise(Vector2 p1, Vector2 p2, Vector2 p3) {
    f32 A = p2.x*p1.y + p3.x*p2.y + p1.x*p3.y;
    f32 B = p1.x*p2.y + p2.x*p3.y + p3.x*p1.y;
    return A < B;
}

// puts the points in counter clockwise order
internal void NormalizeTriangle(Vector2 *ptr_p1, Vector2 *ptr_p2, Vector2 *ptr_p3) {
    ASSERT(ptr_p1 && ptr_p2 && ptr_p3);

    Vector2 p1 = *ptr_p1;
    Vector2 p2 = *ptr_p2;
    Vector2 p3 = *ptr_p3;

// make sure to use the real variables. (or could make this function accept pointers.)
#define Swap(x, y) do { Typeof(x) temp = x; x = y; y = temp; } while (0)

    if (PointsAreCounterClockwise(p1, p2, p3)) Swap(p1, p3);

    ASSERT(!PointsAreCounterClockwise(p1, p2, p3));

    // sort y lowest to highest.
    // if (p1.y > p2.y) Swap(p1, p2);
    // if (p2.y > p3.y) Swap(p2, p3);
    // if (p1.y > p2.y) Swap(p1, p2);

    // if (p2.x > p3.x) Swap(p2, p3);


    *ptr_p1 = p1;
    *ptr_p2 = p2;
    *ptr_p3 = p3;
}

internal void DrawTrianglesFromStartPoint(const Vector2 *points, int points_count, Color color) {
    if (points_count == 0) return;

    for (s32 i = 2; i < points_count; i++) {
        Vector2 p1 = points[0];
        Vector2 p2 = points[i-1];
        Vector2 p3 = points[i];
        NormalizeTriangle(&p1, &p2, &p3);
        DrawTriangle(p1, p2, p3, color);
    }
}


