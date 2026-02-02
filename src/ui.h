#ifndef UI_H
#define UI_H

#define FACTOR 2.5

#define SCREEN_WIDTH   (FACTOR * 800)
#define SCREEN_HEIGHT  (FACTOR * 600)
#define SCREEN_PADDING (FACTOR * 5)

#define TOOL_BAR_X_OFFSET (0)
#define TOOL_BAR_Y_OFFSET (SCREEN_HEIGHT - TOOL_BAR_HEIGHT)
#define TOOL_BAR_WIDTH    (SCREEN_WIDTH)
#define TOOL_BAR_HEIGHT   (FACTOR * 30)
#define TOOL_BAR_PADDING  (FACTOR * 5)

#define TOOL_BOX_LENGTH       (FACTOR * 20)
#define TOOL_SELECTED_PADDING (FACTOR * 3)

#define PALETTE_GAP      (FACTOR * 2)
#define PALETTE_X_OFFSET (TOOL_BAR_X_OFFSET + TOOL_BAR_PADDING)
#define PALETTE_Y_OFFSET (TOOL_BAR_Y_OFFSET + TOOL_BAR_PADDING)
#define PALETTE_COUNT    (ARRAY_LEN(palette))

#define TOOL_ICON_GAP      (2 * PALETTE_GAP)
#define TOOL_ICON_X_OFFSET (PALETTE_X_OFFSET + (PALETTE_COUNT * (TOOL_BOX_LENGTH + PALETTE_GAP)) + TOOL_BOX_LENGTH)
#define TOOL_ICON_Y_OFFSET (PALETTE_Y_OFFSET)
#define TOOL_ICON_COUNT    (ARRAY_LEN(tool_icons))

#define CANVAS_WIDTH  (SCREEN_WIDTH - 2 * SCREEN_PADDING)
#define CANVAS_HEIGHT (SCREEN_HEIGHT - TOOL_BAR_HEIGHT - 2 * SCREEN_PADDING)

#define MAX_BRUSH_SIZE 80
#define MIN_BRUSH_SIZE 2

#define BACKGROUND_COLOR ((Color) { 0x1b, 0x1f, 0x29, 0xff })
#define CANVAS_BORDER_COLOR BLACK
#define ERASER_COLOR ((Color) { 0xf8, 0xf9, 0xfa, 0x80 })

#include "raylib.h"
#include "tool.h"

extern Color palette[];
extern Texture2D tool_icons[];

void ui_context_init(void);
void ui_context_fini(void);
void ui_set_icon(void);
void ui_switch_tool(Tool_Context *ctx, Vector2 mouse_pos);
void ui_switch_color(Tool_Context *ctx, Vector2 mouse_pos);
bool ui_is_point_in_canvas(Vector2 point);
void ui_draw(Tool_Context *ctx);

#endif // UI_H
