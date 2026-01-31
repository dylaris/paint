#include "raylib.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

#define TOOL_BAR_HEIGHT 30
#define COLOR_BOX_LENGTH 20
#define BRUSH_BOX_LENGTH 6

#define MAX_BRUSH_SIZE 60
#define MIN_BRUSH_SIZE 2

#define PALETTE_X_OFFSET 5
#define PALETTE_Y_OFFSET (SCREEN_HEIGHT - TOOL_BAR_HEIGHT + 5)
#define PALETTE_X_GAP 2
#define PALETTE_SELECTED_OFFSET 2

#define CANVAS_OFFSET 5
#define CANVAS_WIDTH (SCREEN_WIDTH - 2*CANVAS_OFFSET)
#define CANVAS_HEIGHT (SCREEN_HEIGHT - TOOL_BAR_HEIGHT - 2*CANVAS_OFFSET)

static Color palette[] = {
    { 255, 255, 255, 255 }, // white
    { 255, 0, 0, 255 },     // red
    { 255, 165, 0, 255 },   // orange
    { 255, 255, 0, 255 },   // yellow
    { 0, 255, 0, 255 },     // green
    { 0, 255, 255, 255 },   // cyan
    { 0, 0, 255, 255 },     // blue
    { 128, 0, 128, 255 },   // purple
};
static RenderTexture2D canvas;
static Vector2 last_canvas_mouse_position, current_canvas_mouse_position;
static bool is_first_frame = true;
static float brush_size = 10.0f;
static int brush_color_index = 0;

static void draw_tool_bar_to_screen(void) {
    int y_offset = PALETTE_Y_OFFSET;
    int x_offset = PALETTE_X_OFFSET;

    // draw palette
    for (unsigned i = 0; i < (unsigned) sizeof(palette)/sizeof(palette[0]); i++) {
        // emphasize selected color
        if (i == brush_color_index) {
            DrawRectangle(
                x_offset + PALETTE_SELECTED_OFFSET,
                y_offset + PALETTE_SELECTED_OFFSET,
                COLOR_BOX_LENGTH - 2*PALETTE_SELECTED_OFFSET,
                COLOR_BOX_LENGTH - 2*PALETTE_SELECTED_OFFSET,
                palette[i]
            );
        } else {
            DrawRectangle(x_offset, y_offset, COLOR_BOX_LENGTH, COLOR_BOX_LENGTH, palette[i]);
        }
        x_offset += COLOR_BOX_LENGTH + PALETTE_X_GAP;
    }

    // draw brush size
    x_offset += 10;
    y_offset += (COLOR_BOX_LENGTH - BRUSH_BOX_LENGTH) / 2;
    DrawRectangle(x_offset, y_offset, brush_size, BRUSH_BOX_LENGTH, RAYWHITE);
}

static void draw_canvas_to_screen(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TOOL_BAR_HEIGHT, GRAY);
    DrawTexture(canvas.texture, CANVAS_OFFSET, CANVAS_OFFSET, WHITE);
}

static void draw_stroke_to_canvas(void) {
    Color brush_color = palette[brush_color_index];
    BeginTextureMode(canvas);
    if (!is_first_frame) {
        DrawLineEx(last_canvas_mouse_position, current_canvas_mouse_position, brush_size, brush_color);
    }
    DrawCircle(current_canvas_mouse_position.x, current_canvas_mouse_position.y, brush_size/2, brush_color);
    EndTextureMode();
}

static void update_brush_size(void) {
    float wheel_move = GetMouseWheelMove();
    brush_size += 2.0f * wheel_move;
    if (brush_size < MIN_BRUSH_SIZE) brush_size = MIN_BRUSH_SIZE;
    else if (brush_size > MAX_BRUSH_SIZE) brush_size = MAX_BRUSH_SIZE;
}

static void switch_brush_color(void) {
    Vector2 current_screen_mouse_position = GetMousePosition();
    if (current_screen_mouse_position.y < PALETTE_Y_OFFSET) return;

    Rectangle color_box = {
        .x = PALETTE_X_OFFSET,
        .y = PALETTE_Y_OFFSET,
        .width = COLOR_BOX_LENGTH,
        .height = COLOR_BOX_LENGTH
    };
    for (unsigned i = 0; i < (unsigned) sizeof(palette)/sizeof(palette[0]); i++) {
        if (CheckCollisionPointRec(current_screen_mouse_position, color_box)) {
            brush_color_index = i;
            return;
        }
        color_box.x += COLOR_BOX_LENGTH + PALETTE_X_GAP;
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Paint");
    SetTargetFPS(60);

    canvas = LoadRenderTexture(CANVAS_WIDTH, CANVAS_HEIGHT);
    BeginTextureMode(canvas);
    ClearBackground(BLACK);
    EndTextureMode();

    while (!WindowShouldClose()) {
        current_canvas_mouse_position = (Vector2) {
            .x = (float) GetMouseX(),
            .y = CANVAS_HEIGHT - (float) GetMouseY(),
        };
        update_brush_size();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) switch_brush_color();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draw_stroke_to_canvas();

        BeginDrawing();
        ClearBackground(BLACK);
        draw_tool_bar_to_screen();
        draw_canvas_to_screen();
        EndDrawing();

        last_canvas_mouse_position = current_canvas_mouse_position;
        is_first_frame = false;
    }

    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
