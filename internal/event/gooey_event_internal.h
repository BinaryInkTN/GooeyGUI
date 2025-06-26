#ifndef GOOEY_EVENT_INTERNAL_H
#define GOOEY_EVENT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of event types.
 */
typedef enum
{
    GOOEY_EVENT_CLICK_PRESS,
    GOOEY_EVENT_CLICK_RELEASE,
    GOOEY_EVENT_MOUSE_MOVE,
    GOOEY_EVENT_MOUSE_SCROLL,
    GOOEY_EVENT_KEY_PRESS,
    GOOEY_EVENT_KEY_RELEASE,
    GOOEY_EVENT_WINDOW_CLOSE,
    GOOEY_EVENT_EXPOSE,
    GOOEY_EVENT_RESIZE,
    GOOEY_EVENT_REDRAWREQ,
    GOOEY_EVENT_DROP,
    GOOEY_EVENT_RESET
} GooeyEventType;

/**
 * @brief Mouse event data.
 */
typedef struct
{
    int x;
    int y;
} GooeyMouseData;

/**
 * @brief Keyboard key press event data.
 */
typedef struct
{
    unsigned int state;
    char value[20];
    unsigned long keycode;
} GooeyKeyPressData;

/**
 * @brief File drop event data.
 */
typedef struct
{
    unsigned int state;
    char mime[20];
    char file_path[1024];
    int drop_x;
    int drop_y;
} GooeyDropData;

/**
 * @brief General event structure.
 */
typedef struct
{
    GooeyEventType type;

    union {
        GooeyMouseData click;
        GooeyMouseData mouse_move;
        GooeyMouseData mouse_scroll;
        GooeyKeyPressData key_press;
        GooeyDropData drop_data;
    };
} GooeyEvent;

#ifdef __cplusplus
}
#endif

#endif /* GOOEY_EVENT_INTERNAL_H */
