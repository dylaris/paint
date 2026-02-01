#include "raylib.h"

#define FACTOR 2

#define SCREEN_WIDTH  (FACTOR*800)
#define SCREEN_HEIGHT (FACTOR*600)
#define SCREEN_PADDING (FACTOR*5)

#define TOOL_BAR_PADDING (FACTOR*5)
#define TOOL_BAR_X_OFFSET 0
#define TOOL_BAR_Y_OFFSET (SCREEN_HEIGHT - TOOL_BAR_HEIGHT)
#define TOOL_BAR_WIDTH SCREEN_WIDTH
#define TOOL_BAR_HEIGHT (FACTOR*30)
#define TOOL_BOX_LENGTH (FACTOR*20)
#define BRUSH_BOX_WIDTH (FACTOR*3)
#define TOOL_SELECTED_PADDING (FACTOR*3)

#define PALETTE_GAP (FACTOR*2)
#define PALETTE_X_OFFSET (TOOL_BAR_X_OFFSET + TOOL_BAR_PADDING)
#define PALETTE_Y_OFFSET (TOOL_BAR_Y_OFFSET + TOOL_BAR_PADDING)

#define TOOL_ICON_GAP (2*PALETTE_GAP)
#define TOOL_ICON_X_OFFSET (PALETTE_X_OFFSET + (GET_ARRAY_SIZE(palette)*(TOOL_BOX_LENGTH+PALETTE_GAP)) + TOOL_BOX_LENGTH)
#define TOOL_ICON_Y_OFFSET PALETTE_Y_OFFSET

#define BRUSH_X_OFFSET (TOOL_ICON_X_OFFSET + (GET_ARRAY_SIZE(tools)*(TOOL_BOX_LENGTH+TOOL_ICON_GAP)) + TOOL_BOX_LENGTH)
#define BRUSH_Y_OFFSET PALETTE_Y_OFFSET

#define MAX_BRUSH_SIZE 80
#define MIN_BRUSH_SIZE 2

#define CANVAS_WIDTH (SCREEN_WIDTH - 2*SCREEN_PADDING)
#define CANVAS_HEIGHT (SCREEN_HEIGHT - TOOL_BAR_HEIGHT - 2*SCREEN_PADDING)

#define BACKGROUND ((Color) { 0x1b, 0x1f, 0x29, 0xff })
#define CANVAS_BORDER BLACK
#define PEN palette[brush_color_index]
#define ERASER ((Color) { 0xf8, 0xf9, 0xfa, 0x80 })

#define GET_ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

static Color palette[] = {
    { 0xf0, 0x71, 0x78, 0xff },
    { 0xf2, 0x96, 0x68, 0xff },
    { 0xff, 0x8f, 0x40, 0xff },
    { 0xff, 0xb4, 0x54, 0xff },
    { 0xe6, 0xc0, 0x8a, 0xff },
    { 0xd2, 0xa6, 0xff, 0xff },
    { 0xaa, 0xd9, 0x4c, 0xff },
    { 0x95, 0xe6, 0xcb, 0xff },
    { 0x59, 0xc2, 0xff, 0xff },
    { 0x39, 0xba, 0xe6, 0xff },
    { 0x10, 0x14, 0x1c, 0xff },
    { 0xf8, 0xf9, 0xfa, 0xff },
};

enum {
    TOOL_PEN = 0,
    TOOL_ERASER,
    TOOL_CLEAR,
    TOOL_RECTANGLE,
    TOOL_CIRCLE,
    TOOL_LINE,
    TOOL_CURVE,
    TOOL_NUMBER,
};
static Texture2D tools[TOOL_NUMBER];

static RenderTexture2D canvas;
static Vector2 last_canvas_mouse_position, current_canvas_mouse_position, current_screen_mouse_position;
static bool is_first_frame = true;
static float brush_size = 10.0f;
static unsigned brush_color_index = 0;
static unsigned tool_index = 0;

static void draw_palette(void) {
    int x_offset = PALETTE_X_OFFSET;
    int y_offset = PALETTE_Y_OFFSET;

    for (unsigned i = 0; i < (unsigned) GET_ARRAY_SIZE(palette); i++) {
        float draw_width = (float) TOOL_BOX_LENGTH;
        float draw_height = (float) TOOL_BOX_LENGTH;

        // emphasize selected color
        if (i == brush_color_index) {
            draw_width -= 2 * TOOL_SELECTED_PADDING;
            draw_height -= 2 * TOOL_SELECTED_PADDING;
            x_offset += TOOL_SELECTED_PADDING;
            y_offset += TOOL_SELECTED_PADDING;
        }

        DrawRectangle(x_offset, y_offset, draw_width, draw_height, palette[i]);

        if (i == brush_color_index) {
            x_offset -= TOOL_SELECTED_PADDING;
            y_offset -= TOOL_SELECTED_PADDING;
        }

        x_offset += TOOL_BOX_LENGTH + PALETTE_GAP;
    }
}

static void draw_tool_icon(void) {
    int x_offset = TOOL_ICON_X_OFFSET;
    int y_offset = TOOL_ICON_Y_OFFSET;

    for (unsigned i = 0; i < (unsigned) GET_ARRAY_SIZE(tools); i++) {
        float texture_width = (float) tools[i].width;
        float texture_height = (float) tools[i].height;

        float draw_width = (float) TOOL_BOX_LENGTH;
        float draw_height = (float) TOOL_BOX_LENGTH;

        // emphasize selected tool
        if (i == tool_index) {
            draw_width -= 2 * TOOL_SELECTED_PADDING;
            draw_height -= 2 * TOOL_SELECTED_PADDING;
            x_offset += TOOL_SELECTED_PADDING;
            y_offset += TOOL_SELECTED_PADDING;
        }

        DrawTexturePro(
            tools[i],
            (Rectangle) { 0, 0, texture_width, texture_height },
            (Rectangle) { (float) x_offset, (float) y_offset, draw_width, draw_height },
            (Vector2) { 0, 0 },
            0.0f,
            WHITE
        );

        DrawRectangleLines(x_offset, y_offset, draw_width, draw_height, WHITE);

        if (i == tool_index) {
            x_offset -= TOOL_SELECTED_PADDING;
            y_offset -= TOOL_SELECTED_PADDING;
        }

        x_offset += TOOL_BOX_LENGTH + TOOL_ICON_GAP;
    }
}

static void draw_brush(void) {
    int x_offset = BRUSH_X_OFFSET;
    int y_offset = BRUSH_Y_OFFSET + (TOOL_BOX_LENGTH - BRUSH_BOX_WIDTH) / 2;
    DrawRectangle(x_offset, y_offset, brush_size, BRUSH_BOX_WIDTH, RAYWHITE);
}

static void draw_tool_bar(void) {
    DrawRectangle(TOOL_BAR_X_OFFSET, TOOL_BAR_Y_OFFSET, TOOL_BAR_WIDTH, TOOL_BAR_HEIGHT, BACKGROUND);
    draw_palette();
    draw_tool_icon();
    draw_brush();
}

static void draw_canvas(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TOOL_BAR_HEIGHT, CANVAS_BORDER);
    DrawTexture(canvas.texture, SCREEN_PADDING, SCREEN_PADDING, WHITE);
}

static void draw_stroke(void) {
    Color brush_color = tool_index == TOOL_ERASER ? BACKGROUND : palette[brush_color_index];

    switch (tool_index) {
    case TOOL_PEN:
    case TOOL_ERASER:
        BeginTextureMode(canvas);
        if (!is_first_frame) {
            DrawLineEx(last_canvas_mouse_position, current_canvas_mouse_position, brush_size, brush_color);
        }
        DrawCircle(current_canvas_mouse_position.x, current_canvas_mouse_position.y, brush_size/2, brush_color);
        EndTextureMode();
        break;

    case TOOL_CLEAR:
        BeginTextureMode(canvas);
        ClearBackground(BACKGROUND);
        EndTextureMode();
        break;

    case TOOL_RECTANGLE:
    case TOOL_CIRCLE:
    case TOOL_LINE:
    case TOOL_CURVE:
        break;
    }
}

static void draw_eraser(void) {
    DrawCircleV(current_screen_mouse_position, brush_size/2, ERASER);
    Rectangle canvas_with_border = {
        .x = 0.0f,
        .y = 0.0f,
        .width = SCREEN_WIDTH,
        .height = CANVAS_HEIGHT + 2*SCREEN_PADDING,
    };
    DrawRectangleLinesEx(canvas_with_border, SCREEN_PADDING, CANVAS_BORDER);
}

static void update_brush_size(void) {
    float wheel_move = GetMouseWheelMove();
    brush_size += 2.0f * wheel_move;
    if (brush_size < MIN_BRUSH_SIZE) brush_size = MIN_BRUSH_SIZE;
    else if (brush_size > MAX_BRUSH_SIZE) brush_size = MAX_BRUSH_SIZE;
}

static void switch_tool(void) {
    if (current_screen_mouse_position.y < TOOL_ICON_Y_OFFSET) return;

    Rectangle tool_box = {
        .x = TOOL_ICON_X_OFFSET,
        .y = TOOL_ICON_Y_OFFSET,
        .width = TOOL_BOX_LENGTH,
        .height = TOOL_BOX_LENGTH
    };

    for (unsigned i = 0; i < (unsigned) GET_ARRAY_SIZE(tools); i++) {
        if (CheckCollisionPointRec(current_screen_mouse_position, tool_box)) {
            tool_index = i;
            return;
        }
        tool_box.x += TOOL_BOX_LENGTH + TOOL_ICON_GAP;
    }
}

static void switch_brush_color(void) {
    if (current_screen_mouse_position.y < PALETTE_Y_OFFSET) return;

    Rectangle color_box = {
        .x = PALETTE_X_OFFSET,
        .y = PALETTE_Y_OFFSET,
        .width = TOOL_BOX_LENGTH,
        .height = TOOL_BOX_LENGTH
    };
    for (unsigned i = 0; i < (unsigned) GET_ARRAY_SIZE(palette); i++) {
        if (CheckCollisionPointRec(current_screen_mouse_position, color_box)) {
            brush_color_index = i;
            return;
        }
        color_box.x += TOOL_BOX_LENGTH + PALETTE_GAP;
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Paint");
    SetTargetFPS(60);

    canvas = LoadRenderTexture(CANVAS_WIDTH, CANVAS_HEIGHT);
    BeginTextureMode(canvas);
    ClearBackground(BACKGROUND);
    EndTextureMode();

    tools[TOOL_PEN]       = LoadTexture("assets/pen.png");
    tools[TOOL_ERASER]    = LoadTexture("assets/eraser.png");
    tools[TOOL_CLEAR]     = LoadTexture("assets/clear.png");
    tools[TOOL_RECTANGLE] = LoadTexture("assets/rectangle.png");
    tools[TOOL_CIRCLE]    = LoadTexture("assets/circle.png");
    tools[TOOL_LINE]      = LoadTexture("assets/line.png");
    tools[TOOL_CURVE]     = LoadTexture("assets/curve.png");

    while (!WindowShouldClose()) {
        current_screen_mouse_position = GetMousePosition();
        current_canvas_mouse_position = (Vector2) {
            .x = current_screen_mouse_position.x,
            .y = CANVAS_HEIGHT - current_screen_mouse_position.y
        };
        update_brush_size();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            switch_brush_color();
            switch_tool();
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draw_stroke();

        BeginDrawing();
        ClearBackground(BACKGROUND);
        draw_canvas();
        if (tool_index == TOOL_ERASER) draw_eraser();
        draw_tool_bar();
        EndDrawing();

        last_canvas_mouse_position = current_canvas_mouse_position;
        is_first_frame = false;
    }

    for (unsigned i = 0; i < GET_ARRAY_SIZE(tools); i++) UnloadTexture(tools[i]);
    UnloadRenderTexture(canvas);
    CloseWindow();
    return 0;
}
