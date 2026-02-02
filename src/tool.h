#ifndef TOOL_H
#define TOOL_H

#include "raylib.h"
#include "canvas.h"

typedef enum {
    TOOL_PEN = 0,
    TOOL_ERASER,
    TOOL_CLEAR,
    TOOL_FILL_RECTANGLE,
    TOOL_LINE_RECTANGLE,
    TOOL_FILL_CIRCLE,
    TOOL_LINE_CIRCLE,
    // TOOL_FILL_TRIANGLE,
    // TOOL_LINE_TRIANGLE,
    TOOL_LINE,
    TOOL_CURVE,
    TOOL_EXPORT,
    NUMBER_OF_TOOL,
} Tool_Kind;

typedef struct {
    Vector2 start_point;
    Vector2 end_point;
    bool selecting;
    bool confirm;
} Tool_Preview;

typedef struct {
    Tool_Kind current_tool;
    float brush_size;
    unsigned int color_index;
    Tool_Preview preview;
} Tool_Context;

Tool_Context tool_context_create(Tool_Kind current_tool, float brush_size, unsigned int color_index);
void tool_context_destroy(Tool_Context *ctx);
void tool_draw_stroke(Canvas *canvs, Tool_Context *ctx, Vector2 mouse_pos);
void tool_update_preview(Tool_Context *ctx, Vector2 mouse_pos);
void tool_draw_preview(Tool_Context *ctx, Vector2 mouse_pos);

#endif // TOOL_H
