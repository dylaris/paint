#include "tool.h"
#include "ui.h"
#include "utils.h"

#include <math.h>

Tool_Context tool_context_create(Tool_Kind current_tool, float brush_size, unsigned int color_index) {
    return (Tool_Context) {
        .current_tool = current_tool,
        .brush_size = brush_size,
        .color_index = color_index,
        .preview = (Tool_Preview) {
            .selecting = false,
            .confirm = false,
        }
    };
}

void tool_context_destroy(Tool_Context *ctx) { (void) ctx; }

void tool_draw_preview(Tool_Context *ctx, Vector2 mouse_pos) {
    Color brush_color = ctx->current_tool == TOOL_ERASER ? ERASER_COLOR : palette[ctx->color_index];
    Color preview_brush_color = { brush_color.r, brush_color.g, brush_color.b, 128 };

    switch (ctx->current_tool) {
    case TOOL_PEN:
    case TOOL_ERASER:
        DrawCircleV(mouse_pos, ctx->brush_size / 2, brush_color);
        break;

    case TOOL_FILL_RECTANGLE: {
        if (!ctx->preview.selecting) break;
        Rectangle rect = {
            .x = MIN(ctx->preview.start_point.x, ctx->preview.end_point.x),
            .y = MIN(ctx->preview.start_point.y, ctx->preview.end_point.y),
            .width = ABS(ctx->preview.end_point.x - ctx->preview.start_point.x),
            .height = ABS(ctx->preview.end_point.y - ctx->preview.start_point.y),
        };
        DrawRectangleRec(rect, preview_brush_color);
        DrawRectangleLinesEx(rect, 2, brush_color);
    } break;

    case TOOL_LINE_RECTANGLE: {
        if (!ctx->preview.selecting) break;
        Rectangle rect = {
            .x = MIN(ctx->preview.start_point.x, ctx->preview.end_point.x),
            .y = MIN(ctx->preview.start_point.y, ctx->preview.end_point.y),
            .width = ABS(ctx->preview.end_point.x - ctx->preview.start_point.x),
            .height = ABS(ctx->preview.end_point.y - ctx->preview.start_point.y),
        };
        Rectangle inner_rect = {
            .x = rect.x + ctx->brush_size,
            .y = rect.y + ctx->brush_size,
            .width = rect.width - 2 * ctx->brush_size,
            .height = rect.height - 2 * ctx->brush_size,
        };
        DrawRectangleLinesEx(rect, ctx->brush_size, preview_brush_color);
        DrawRectangleLinesEx(rect, 2, brush_color);
        DrawRectangleLinesEx(inner_rect, 2, brush_color);
    } break;

    case TOOL_FILL_CIRCLE: {
        if (!ctx->preview.selecting) break;
        Vector2 center = {
            .x = (ctx->preview.start_point.x + ctx->preview.end_point.x) / 2,
            .y = (ctx->preview.start_point.y + ctx->preview.end_point.y) / 2,
        };
        float radius = DIST(center, ctx->preview.start_point);
        DrawCircleV(center, radius, preview_brush_color);
        DrawCircleLinesV(center, radius, brush_color);
    } break;

    case TOOL_LINE_CIRCLE: {
        if (!ctx->preview.selecting) break;
        Vector2 center = {
            .x = (ctx->preview.start_point.x + ctx->preview.end_point.x) / 2,
            .y = (ctx->preview.start_point.y + ctx->preview.end_point.y) / 2,
        };
        float radius = DIST(center, ctx->preview.start_point);
        float inner_radius = radius < ctx->brush_size ? 0 : radius - ctx->brush_size;
        DrawRing(center, inner_radius, radius, 0.0f, 360.0f, 0, preview_brush_color);
        DrawRingLines(center, inner_radius, radius, 0.0f, 360.0f, 0, brush_color);
    } break;

    case TOOL_LINE:
        if (!ctx->preview.selecting) break;
        DrawLineEx(ctx->preview.start_point, ctx->preview.end_point, ctx->brush_size, preview_brush_color);
        break;

    case TOOL_CURVE:
        if (!ctx->preview.selecting) break;
        DrawLineBezier(ctx->preview.start_point, ctx->preview.end_point, ctx->brush_size, preview_brush_color);
        break;

    default:
        break;
    }

    Rectangle canvas_with_border = {
        .x = 0.0f,
        .y = 0.0f,
        .width = SCREEN_WIDTH,
        .height = CANVAS_HEIGHT + 2 * SCREEN_PADDING,
    };
    DrawRectangleLinesEx(canvas_with_border, SCREEN_PADDING, CANVAS_BORDER_COLOR);
}

void tool_update_preview(Tool_Context *ctx, Vector2 mouse_pos) {
    if (!ctx->preview.selecting) return;
    ctx->preview.end_point = mouse_pos;
}

static Vector2 screen_to_canvas(Vector2 point) {
    return (Vector2) {
        .x = point.x - SCREEN_PADDING,
        .y = CANVAS_HEIGHT - (point.y - SCREEN_PADDING)
    };
}

static void tool_reset_preview(Tool_Context *ctx) {
    ctx->preview.selecting = false;
    ctx->preview.confirm = false;
}

void tool_draw_stroke(Canvas *canvas, Tool_Context *ctx, Vector2 mouse_pos) {
    Color brush_color = ctx->current_tool == TOOL_ERASER ? BACKGROUND_COLOR : palette[ctx->color_index];
    Vector2 start_point = screen_to_canvas(ctx->preview.start_point);
    Vector2 end_point = screen_to_canvas(ctx->preview.end_point);
    Vector2 mouse_delta = GetMouseDelta();
    Vector2 last_mouse_pos = {
        .x = mouse_pos.x - mouse_delta.x,
        .y = mouse_pos.y - mouse_delta.y,
    };

    switch (ctx->current_tool) {
    case TOOL_PEN:
    case TOOL_ERASER:
        if (!ui_is_point_in_canvas(mouse_pos)) break;
        BeginTextureMode(canvas->render_texture);
            DrawLineEx(screen_to_canvas(last_mouse_pos), screen_to_canvas(mouse_pos), ctx->brush_size, brush_color);
            DrawCircleV(screen_to_canvas(mouse_pos), ctx->brush_size / 2, brush_color);
        EndTextureMode();
        break;

    case TOOL_FILL_RECTANGLE: {
        if (!ctx->preview.confirm) break;
        Rectangle rect = {
            .x = MIN(start_point.x, end_point.x),
            .y = MIN(start_point.y, end_point.y),
            .width = ABS(end_point.x - start_point.x),
            .height = ABS(end_point.y - start_point.y),
        };
        BeginTextureMode(canvas->render_texture);
            DrawRectangleRec(rect, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
    } break;

    case TOOL_FILL_CIRCLE: {
        if (!ctx->preview.confirm) break;
        Vector2 center = {
            .x = (start_point.x + end_point.x) / 2,
            .y = (start_point.y + end_point.y) / 2,
        };
        float radius = DIST(center, start_point);
        BeginTextureMode(canvas->render_texture);
            DrawCircleV(center, radius, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
    } break;

    case TOOL_LINE_RECTANGLE: {
        if (!ctx->preview.confirm) break;
        Rectangle rect = {
            .x = MIN(start_point.x, end_point.x),
            .y = MIN(start_point.y, end_point.y),
            .width = ABS(end_point.x - start_point.x),
            .height = ABS(end_point.y - start_point.y),
        };
        BeginTextureMode(canvas->render_texture);
            DrawRectangleLinesEx(rect, ctx->brush_size, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
    } break;

    case TOOL_LINE_CIRCLE: {
        if (!ctx->preview.confirm) break;
        Vector2 center = {
            .x = (start_point.x + end_point.x) / 2,
            .y = (start_point.y + end_point.y) / 2,
        };
        float radius = DIST(center, start_point);
        float inner_radius = radius < ctx->brush_size ? 0 : radius - ctx->brush_size;
        BeginTextureMode(canvas->render_texture);
            DrawRing(center, inner_radius, radius, 0.0f, 360.0f, 0, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
    } break;

    case TOOL_LINE:
        if (!ctx->preview.confirm) break;
        BeginTextureMode(canvas->render_texture);
            DrawLineEx(start_point, end_point, ctx->brush_size, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
        break;

    case TOOL_CURVE:
        if (!ctx->preview.confirm) break;
        BeginTextureMode(canvas->render_texture);
            DrawLineBezier(start_point, end_point, ctx->brush_size, brush_color);
        EndTextureMode();
        tool_reset_preview(ctx);
        break;

    case TOOL_CLEAR:
        canvas_clear(canvas);
        ctx->current_tool = TOOL_PEN;
        break;

    case TOOL_EXPORT:
        canvas_export(canvas);
        ctx->current_tool = TOOL_PEN;
        break;

    default:
        break;
    }
}

