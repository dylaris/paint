#ifndef CANVAS_H
#define CANVAS_H

#include "raylib.h"

#define OUTPUT_DIR "output/"

typedef struct {
    RenderTexture2D render_texture;
    float width;
    float height;
} Canvas;

Canvas canvas_create(float width, float height);
void canvas_destroy(Canvas *canvas);
Vector2 canvas_to_screen(Vector2 mouse_pos);
void canvas_draw(Canvas *canvas);
void canvas_export(Canvas *canvas);
void canvas_clear(Canvas *canvas);

#endif // CANVAS_H
