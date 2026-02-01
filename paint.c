#include "raylib.h"
#include <stdio.h>

#define FACTOR 2.5

#define SCREEN_WIDTH  (FACTOR*800)
#define SCREEN_HEIGHT (FACTOR*600)
#define SCREEN_PADDING (FACTOR*5)

#define TOOL_BAR_PADDING (FACTOR*5)
#define TOOL_BAR_X_OFFSET 0
#define TOOL_BAR_Y_OFFSET (SCREEN_HEIGHT - TOOL_BAR_HEIGHT)
#define TOOL_BAR_WIDTH SCREEN_WIDTH
#define TOOL_BAR_HEIGHT (FACTOR*30)
#define TOOL_BOX_LENGTH (FACTOR*20)
#define TOOL_SELECTED_PADDING (FACTOR*3)

#define PALETTE_GAP (FACTOR*2)
#define PALETTE_X_OFFSET (TOOL_BAR_X_OFFSET + TOOL_BAR_PADDING)
#define PALETTE_Y_OFFSET (TOOL_BAR_Y_OFFSET + TOOL_BAR_PADDING)

#define TOOL_ICON_GAP (2*PALETTE_GAP)
#define TOOL_ICON_X_OFFSET (PALETTE_X_OFFSET + (GET_ARRAY_SIZE(palette)*(TOOL_BOX_LENGTH+PALETTE_GAP)) + TOOL_BOX_LENGTH)
#define TOOL_ICON_Y_OFFSET PALETTE_Y_OFFSET

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
    TOOL_EXPORT,
    TOOL_NUMBER,
};
static Texture2D tools[TOOL_NUMBER];

static RenderTexture2D canvas;
static Vector2 last_canvas_mouse_position, current_canvas_mouse_position, current_screen_mouse_position;
static bool is_first_frame = true;
static float brush_size = 10.0f;
static unsigned brush_color_index = 0;
static unsigned tool_index = 0;
static int output_times = 0;

typedef struct {
    Vector2 first_point;
    Vector2 last_point;
    bool selecting;
    bool confirm;
} Preview_Section;

static Preview_Section current_preview_section = {
    .selecting = false,
    .confirm = false,
};

#define da_append(da, item) \
    do { \
        if ((da)->count + 1 > (da)->capacity) { \
            (da)->items = realloc((da)->items, sizeof((da)->items[0])*(da)->capacity); \
            assert((da)->items && "run out of memory"); \
        } \
        (da)->items[(da)->count++] = (item); \
    } while (0)

#define da_free(da) \
    do { \
        if ((da)->items) free((da)->items); \
        (da)->items = NULL; \
        (da)->count = 0; \
    } while (0)

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

static bool is_mouse_in_canvas(void) {
    return (current_screen_mouse_position.x >= SCREEN_PADDING &&
            current_screen_mouse_position.x <= SCREEN_WIDTH - SCREEN_PADDING &&
            current_screen_mouse_position.y >= SCREEN_PADDING &&
            current_screen_mouse_position.y <= SCREEN_HEIGHT - TOOL_BAR_HEIGHT - SCREEN_PADDING);
}

static void update_preview_section(void) {
    if (current_preview_section.selecting && is_mouse_in_canvas()) {
        current_preview_section.last_point = current_canvas_mouse_position;
    }
}

static Vector2 canvas_to_screen(Vector2 canvas_point) {
    return (Vector2) {
        .x = canvas_point.x + SCREEN_PADDING,
        .y = CANVAS_HEIGHT - canvas_point.y + SCREEN_PADDING,
    };
}

static void draw_preview_rectangle(void) {
    if (!current_preview_section.selecting) return;

    Color brush_color = palette[brush_color_index];
    Vector2 screen_first_point = canvas_to_screen(current_preview_section.first_point);
    Vector2 screen_last_point = canvas_to_screen(current_preview_section.last_point);
    float rect_x = MIN(screen_first_point.x, screen_last_point.x);
    float rect_y = MIN(screen_first_point.y, screen_last_point.y);
    float rect_width = ABS(screen_last_point.x - screen_first_point.x);
    float rect_height = ABS(screen_last_point.y - screen_first_point.y);
    Rectangle rect = { rect_x, rect_y, rect_width, rect_height };

    DrawRectangleRec(rect, (Color){ brush_color.r, brush_color.g, brush_color.b, 128 });
    DrawRectangleLinesEx(rect, 2, brush_color);
}

static void draw_confirm_rectangle(void) {
    if (!current_preview_section.confirm) return;

    Color brush_color = palette[brush_color_index];
    float rect_x = MIN(current_preview_section.first_point.x, current_preview_section.last_point.x);
    float rect_y = MIN(current_preview_section.first_point.y, current_preview_section.last_point.y);
    float rect_width = ABS(current_preview_section.last_point.x - current_preview_section.first_point.x);
    float rect_height = ABS(current_preview_section.last_point.y - current_preview_section.first_point.y);

    BeginTextureMode(canvas);
    DrawRectangleRec((Rectangle) { rect_x, rect_y, rect_width, rect_height }, brush_color);
    EndTextureMode();

    current_preview_section.selecting = false;
    current_preview_section.confirm = false;
}

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

static void draw_tool_bar(void) {
    DrawRectangle(TOOL_BAR_X_OFFSET, TOOL_BAR_Y_OFFSET, TOOL_BAR_WIDTH, TOOL_BAR_HEIGHT, BACKGROUND);
    draw_palette();
    draw_tool_icon();
}

static void draw_canvas(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TOOL_BAR_HEIGHT, CANVAS_BORDER);
    DrawTexture(canvas.texture, SCREEN_PADDING, SCREEN_PADDING, WHITE);
}

static void export_canvas(void) {
    static char filename[128] = {0};
    snprintf(filename, sizeof(filename), "output-%02d.png", output_times++);
    Image image = LoadImageFromTexture(canvas.texture);
    ExportImage(image, filename);
    UnloadImage(image);
}

static void clear_canvas(void) {
    BeginTextureMode(canvas);
    ClearBackground(BACKGROUND);
    EndTextureMode();
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

    case TOOL_RECTANGLE:
    case TOOL_CIRCLE:
    case TOOL_LINE:
    case TOOL_CURVE:
        break;

    default:
        break;
    }
}

static void draw_mouse_in_canvas(Color color) {
    DrawCircleV(current_screen_mouse_position, brush_size/2, color);
    Rectangle canvas_with_border = {
        .x = 0.0f,
        .y = 0.0f,
        .width = SCREEN_WIDTH,
        .height = CANVAS_HEIGHT + 2*SCREEN_PADDING,
    };
    DrawRectangleLinesEx(canvas_with_border, SCREEN_PADDING, CANVAS_BORDER);
}

static void draw_eraser(void) {
    draw_mouse_in_canvas(ERASER);
}

static void draw_pen(void) {
    draw_mouse_in_canvas(PEN);
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

    Image icon = LoadImage("assets/app.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

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
    tools[TOOL_EXPORT]    = LoadTexture("assets/export.png");

    while (!WindowShouldClose()) {
        current_screen_mouse_position = GetMousePosition();
        current_canvas_mouse_position = (Vector2) {
            .x = current_screen_mouse_position.x - SCREEN_PADDING,
            .y = CANVAS_HEIGHT - (current_screen_mouse_position.y - SCREEN_PADDING)
        };

        update_brush_size();
        update_preview_section();

        if (is_mouse_in_canvas()) {
            if (tool_index == TOOL_RECTANGLE) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (!current_preview_section.selecting) {
                        current_preview_section.first_point = current_canvas_mouse_position;
                        current_preview_section.last_point = current_canvas_mouse_position;
                        current_preview_section.selecting = true;
                        current_preview_section.confirm = false;
                    } else {
                        current_preview_section.confirm = true;
                        draw_confirm_rectangle();
                    }
                }
                if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    current_preview_section.selecting = false;
                    current_preview_section.confirm = false;
                }
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                switch_brush_color();
                switch_tool();
            }
        }

        if (tool_index == TOOL_CLEAR) {
            clear_canvas();
            tool_index = TOOL_PEN;
        } else if (tool_index == TOOL_EXPORT) {
            export_canvas();
            tool_index = TOOL_PEN;
        } else {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) draw_stroke();
        }

        BeginDrawing();
        ClearBackground(BACKGROUND);
        draw_canvas();
        if (tool_index == TOOL_ERASER) draw_eraser();
        else if (tool_index == TOOL_PEN) draw_pen();
        else if (tool_index == TOOL_RECTANGLE) draw_preview_rectangle();
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

// TODO: the code is sucks, very chaos, rewrite it
// TODO: save the paint to 'output/' folder
// TODO: filled-circle/line/outlin-rectangle/outline-circle
// TODO: click 'clear' and 'export', change the border color to say ok or error
// TODO: Ctrl-Z to cancel the last operation
