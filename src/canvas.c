#include "canvas.h"
#include "ui.h"

#include <stdio.h>

Canvas canvas_create(float width, float height) {
    return (Canvas) {
        .render_texture = LoadRenderTexture(width, height),
        .width = width,
        .height = height,
    };
}

void canvas_destroy(Canvas *canvas) {
    UnloadRenderTexture(canvas->render_texture);
}

Vector2 canvas_to_screen(Vector2 canvas_point) {
    return (Vector2) {
        .x = canvas_point.x + SCREEN_PADDING,
        .y = CANVAS_HEIGHT - canvas_point.y + SCREEN_PADDING,
    };
}

void canvas_draw(Canvas *canvas) {
    DrawRectangleLinesEx(
        (Rectangle) {
            .x = 0,
            .y = 0,
            .width = SCREEN_WIDTH,
            .height = SCREEN_HEIGHT,
        },
        SCREEN_PADDING,
        CANVAS_BORDER_COLOR);
    DrawTexture(canvas->render_texture.texture, SCREEN_PADDING, SCREEN_PADDING, WHITE);
}

void canvas_export(Canvas *canvas) {
    static int output_times = 0;
    static char filename[128] = {0};
    snprintf(filename, sizeof(filename), "output-%02d.png", output_times++);
    Image image = LoadImageFromTexture(canvas->render_texture.texture);
    ExportImage(image, filename);
    UnloadImage(image);
}

void canvas_clear(Canvas *canvas) {
    BeginTextureMode(canvas->render_texture);
    ClearBackground(BACKGROUND_COLOR);
    EndTextureMode();
}
