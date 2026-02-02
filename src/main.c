#include "canvas.h"
#include "ui.h"
#include "utils.h"

static void handle_input(Tool_Context *tool_context, Vector2 mouse_pos);

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Paint");
    Canvas canvas = canvas_create(CANVAS_WIDTH, CANVAS_HEIGHT);
    Tool_Context tool_context = tool_context_create(TOOL_PEN, 10.0f, 0);
    ui_context_init();

    SetTargetFPS(60);
    ui_set_icon();

    while (!WindowShouldClose()) {
        Vector2 mouse_pos = GetMousePosition();

        handle_input(&tool_context, mouse_pos);
        tool_update_preview(&tool_context, mouse_pos);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            tool_draw_stroke(&canvas, &tool_context, mouse_pos);
        }

        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            canvas_draw(&canvas);
            tool_draw_preview(&tool_context, mouse_pos);
            ui_draw(&tool_context);
        EndDrawing();
    }

    ui_context_fini();
    tool_context_destroy(&tool_context);
    canvas_destroy(&canvas);
    CloseWindow();
    return 0;
}

static void update_brush_size(Tool_Context *tool_context) {
    float wheel_move = GetMouseWheelMove();
    tool_context->brush_size += 2.0f * wheel_move;
    CLAMP(tool_context->brush_size, MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
}

static void handle_input(Tool_Context *tool_context, Vector2 mouse_pos) {
    // global
    update_brush_size(tool_context);

    // inside canvas
    if (ui_is_point_in_canvas(mouse_pos)) {
        if (tool_context->current_tool != TOOL_PEN &&
            tool_context->current_tool != TOOL_ERASER &&
            tool_context->current_tool != TOOL_CLEAR &&
            tool_context->current_tool != TOOL_EXPORT
        ) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (!tool_context->preview.selecting) {
                    tool_context->preview.start_point = mouse_pos;
                    tool_context->preview.end_point = mouse_pos;
                    tool_context->preview.selecting = true;
                    tool_context->preview.confirm = false;
                } else {
                    tool_context->preview.confirm = true;
                }
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                tool_context->preview.selecting = false;
                tool_context->preview.confirm = false;
            }
        }
    }
    // outside canvas (toolbar / palette)
    else {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ui_switch_color(tool_context, mouse_pos);
            ui_switch_tool(tool_context, mouse_pos);
        }
    }
}

// TODO: create the output directory in code
// TODO: support triangle
// TODO: support draw histroy (use Ctrl-Z to undo last operation)
// TODO: change the canvas border color to declare if the clear/export operation is success
