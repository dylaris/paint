#include "tool.h"
#include "utils.h"
#include "ui.h"

Color palette[] = {
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

Texture2D tool_icons[NUMBER_OF_TOOL];

void ui_context_init(void) {
    tool_icons[TOOL_PEN]            = LoadTexture("assets/pen.png");
    tool_icons[TOOL_ERASER]         = LoadTexture("assets/eraser.png");
    tool_icons[TOOL_CLEAR]          = LoadTexture("assets/clear.png");
    tool_icons[TOOL_FILL_RECTANGLE] = LoadTexture("assets/fill_rectangle.png");
    tool_icons[TOOL_FILL_CIRCLE]    = LoadTexture("assets/fill_circle.png");
    // tool_icons[TOOL_FILL_TRIANGLE]  = LoadTexture("assets/fill_triangle.png");
    tool_icons[TOOL_LINE_RECTANGLE] = LoadTexture("assets/line_rectangle.png");
    tool_icons[TOOL_LINE_CIRCLE]    = LoadTexture("assets/line_circle.png");
    // tool_icons[TOOL_LINE_TRIANGLE]  = LoadTexture("assets/line_triangle.png");
    tool_icons[TOOL_LINE]           = LoadTexture("assets/line.png");
    tool_icons[TOOL_CURVE]          = LoadTexture("assets/curve.png");
    tool_icons[TOOL_EXPORT]         = LoadTexture("assets/export.png");
}

void ui_context_fini(void) {
    for (unsigned int i = 0; i < NUMBER_OF_TOOL; i++) {
        UnloadTexture(tool_icons[i]);
    }
}

void ui_set_icon(void) {
    Image icon = LoadImage("assets/app.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
}

void ui_switch_tool(Tool_Context *ctx, Vector2 mouse_pos) {
    Rectangle tool_box = {
        .x = TOOL_ICON_X_OFFSET,
        .y = TOOL_ICON_Y_OFFSET,
        .width = TOOL_BOX_LENGTH,
        .height = TOOL_BOX_LENGTH
    };

    for (unsigned int i = 0; i < TOOL_ICON_COUNT; i++) {
        if (CheckCollisionPointRec(mouse_pos, tool_box)) {
            ctx->current_tool = i;
            return;
        }
        tool_box.x += TOOL_BOX_LENGTH + TOOL_ICON_GAP;
    }
}

void ui_switch_color(Tool_Context *ctx, Vector2 mouse_pos) {
    Rectangle color_box = {
        .x = PALETTE_X_OFFSET,
        .y = PALETTE_Y_OFFSET,
        .width = TOOL_BOX_LENGTH,
        .height = TOOL_BOX_LENGTH
    };

    for (unsigned int i = 0; i < PALETTE_COUNT; i++) {
        if (CheckCollisionPointRec(mouse_pos, color_box)) {
            ctx->color_index = i;
            return;
        }
        color_box.x += TOOL_BOX_LENGTH + PALETTE_GAP;
    }
}

bool ui_is_point_in_canvas(Vector2 point) {
    return (point.x >= SCREEN_PADDING &&
            point.x <= SCREEN_WIDTH - SCREEN_PADDING &&
            point.y >= SCREEN_PADDING &&
            point.y <= SCREEN_HEIGHT - TOOL_BAR_HEIGHT - SCREEN_PADDING);
}

static void draw_palette(Tool_Context *ctx) {
    int x_offset = PALETTE_X_OFFSET;
    int y_offset = PALETTE_Y_OFFSET;

    for (unsigned int i = 0; i < PALETTE_COUNT; i++) {
        float draw_width = (float) TOOL_BOX_LENGTH;
        float draw_height = (float) TOOL_BOX_LENGTH;

        // emphasize selected color
        if (i == ctx->color_index) {
            draw_width -= 2 * TOOL_SELECTED_PADDING;
            draw_height -= 2 * TOOL_SELECTED_PADDING;
            x_offset += TOOL_SELECTED_PADDING;
            y_offset += TOOL_SELECTED_PADDING;
        }

        DrawRectangle(x_offset, y_offset, draw_width, draw_height, palette[i]);

        if (i == ctx->color_index) {
            x_offset -= TOOL_SELECTED_PADDING;
            y_offset -= TOOL_SELECTED_PADDING;
        }

        x_offset += TOOL_BOX_LENGTH + PALETTE_GAP;
    }
}

static void draw_tool_icons(Tool_Context *ctx) {
    int x_offset = TOOL_ICON_X_OFFSET;
    int y_offset = TOOL_ICON_Y_OFFSET;

    for (unsigned int i = 0; i < TOOL_ICON_COUNT; i++) {
        float texture_width = (float) tool_icons[i].width;
        float texture_height = (float) tool_icons[i].height;

        float draw_width = (float) TOOL_BOX_LENGTH;
        float draw_height = (float) TOOL_BOX_LENGTH;

        // emphasize selected tool
        if (i == ctx->current_tool) {
            draw_width -= 2 * TOOL_SELECTED_PADDING;
            draw_height -= 2 * TOOL_SELECTED_PADDING;
            x_offset += TOOL_SELECTED_PADDING;
            y_offset += TOOL_SELECTED_PADDING;
        }

        DrawTexturePro(
            tool_icons[i],
            (Rectangle) { 0, 0, texture_width, texture_height },
            (Rectangle) { (float) x_offset, (float) y_offset, draw_width, draw_height },
            (Vector2) { 0, 0 },
            0.0f,
            WHITE
        );

        DrawRectangleLines(x_offset, y_offset, draw_width, draw_height, WHITE);

        if (i == ctx->current_tool) {
            x_offset -= TOOL_SELECTED_PADDING;
            y_offset -= TOOL_SELECTED_PADDING;
        }

        x_offset += TOOL_BOX_LENGTH + TOOL_ICON_GAP;
    }
}

void ui_draw(Tool_Context *ctx) {
    DrawRectangle(TOOL_BAR_X_OFFSET, TOOL_BAR_Y_OFFSET, TOOL_BAR_WIDTH, TOOL_BAR_HEIGHT, BACKGROUND_COLOR);
    draw_palette(ctx);
    draw_tool_icons(ctx);
}

