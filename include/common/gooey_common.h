/**
 * @file gooey_widgets_internal.h
 * @brief Internal definitions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains the internal structures, enums, and macros used by the Gooey GUI library.
 * It defines the core widget types, their properties, and utility structures for managing GUI elements.
 */

#ifndef GOOEY_WIDGETS_INTERNAL_H
#define GOOEY_WIDGETS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../user_config.h"
#include "theme/gooey_theme.h"

#pragma pack(push, 1)

typedef enum
{
    WIDGET_LABEL,
    WIDGET_SLIDER,
    WIDGET_RADIOBUTTON,
    WIDGET_CHECKBOX,
    WIDGET_BUTTON,
    WIDGET_TEXTBOX,
    WIDGET_DROPDOWN,
    WIDGET_CANVAS,
    WIDGET_LAYOUT,
    WIDGET_PLOT,
    WIDGET_DROP_SURFACE,
    WIDGET_IMAGE,
    WIDGET_LIST,
    WIDGET_PROGRESSBAR,
    WIDGET_METER,
    WIDGET_CONTAINER,
    WIDGET_SWITCH,
    WIDGET_WEBVIEW,
    WIDGET_CTXMENU,
    WIDGET_NODE_EDITOR,
    WIDGET_TABS
} WIDGET_TYPE;

typedef struct
{
    void *sprite_ptr;
    int x;
    int y;
    int width;
    int height;
    bool needs_redraw;
    char __padding[3];
} GooeyTFT_Sprite;

typedef struct
{
    void *timer_ptr;
} GooeyTimer;

typedef struct
{
    GooeyTFT_Sprite *sprite;
    WIDGET_TYPE type;
    bool is_visible;
    bool disable_input;
    char __padding[2];
    int x, y;
    int width, height;
} GooeyWidget;

typedef enum
{
    GOOEY_CURSOR_ARROW,
    GOOEY_CURSOR_TEXT,
    GOOEY_CURSOR_CROSSHAIR,
    GOOEY_CURSOR_HAND,
    GOOEY_CURSOR_RESIZE_H,
    GOOEY_CURSOR_RESIZE_V,
    GOOEY_CURSOR_RESIZE_TL_BR,
    GOOEY_CURSOR_RESIZE_TR_BL,
    GOOEY_CURSOR_RESIZE_ALL,
    GOOEY_CURSOR_NOT_ALLOWED
} GOOEY_CURSOR;

typedef enum
{
    SLIDER_HORIZONTAL,
    SLIDER_VERTICAL
} SLIDER_ORIENTATION;

typedef enum
{
    MSGBOX_SUCCES,
    MSGBOX_INFO,
    MSGBOX_FAIL
} MSGBOX_TYPE;

typedef struct
{
    GooeyWidget core;
    char label[256];
    void (*callback)(void *user_data);
    void *user_data;
    bool clicked;
    bool hover;
    bool is_highlighted;
    bool is_disabled;
    int click_timer;
} GooeyButton;

typedef struct
{
    GooeyWidget core;
    char *current_path;
    char **file_list;
    int file_count;
    int selected_index;
    void (*on_file_selected)(const char *file_path, void *user_data);
    void *user_data;
} GooeyFDialog;

typedef struct
{
    char label[256];
    void (*callback)(void *user_data);
    void *user_data;
} GooeyCtxMenuItem;

typedef struct
{
    GooeyWidget core;
    size_t menu_item_count;
    bool is_open;
    bool is_hovered;
    char __padding[6];
    GooeyCtxMenuItem menu_items[GOOEY_CTXMENU_MAX_ITEMS];
} GooeyCtxMenu;

typedef struct
{
    size_t id;
    void **widgets;
    size_t widget_count;
} GooeyContainer;

typedef struct
{
    GooeyWidget core;
    GooeyContainer *container;
    size_t container_count;
    size_t active_container_id;
} GooeyContainers;

typedef enum
{
    CANVA_DRAW_RECT,
    CANVA_DRAW_LINE,
    CANVA_DRAW_ARC,
    CANVA_DRAW_SET_FG
} CANVA_DRAW_OP;

typedef struct
{
    GooeyWidget core;
    char text[256];
    char placeholder[256];
    void (*callback)(char *text, void *user_data);
    void *user_data;
    bool focused;
    bool is_password;
    int cursor_pos;
    int scroll_offset;
} GooeyTextbox;

typedef struct
{
    GooeyWidget core;
    char text[256];
    float font_size;
    unsigned long color;
    bool is_using_custom_color;
    char __padding[3];
} GooeyLabel;

typedef struct
{
    GooeyWidget core;
    bool checked;
    char label[256];
    void (*callback)(bool checked, void *user_data);
    void *user_data;
} GooeyCheckbox;

typedef struct
{
    char title[256];
    char description[256];
} GooeyListItem;

typedef struct
{
    GooeyWidget core;
    GooeyListItem *items;
    int scroll_offset;
    int thumb_y;
    int thumb_height;
    int thumb_width;
    int item_spacing;
    size_t item_count;
    bool show_separator;
    char __padding[7];
    void (*callback)(int index, void *user_data);
    void *user_data;
    int element_hovered_over;
} GooeyList;

typedef struct
{
    GooeyWidget core;
    bool selected;
    char label[256];
    int radius;
    void (*callback)(bool selected, void *user_data);
    void *user_data;
} GooeyRadioButton;

typedef struct
{
    GooeyRadioButton buttons[MAX_RADIO_BUTTONS];
    int button_count;
} GooeyRadioButtonGroup;

typedef struct
{
    GooeyWidget core;
    long value;
    long min_value;
    long max_value;
    bool show_hints;
    char __padding[7];
    void (*callback)(long value, void *user_data);
    void *user_data;
    SLIDER_ORIENTATION orientation;
} GooeySlider;

typedef struct
{
    GooeyWidget core;
    long value;
} GooeyProgressBar;

typedef struct
{
    GooeyWidget core;
    int selected_index;
    const char **options;
    int num_options;
    bool is_open;
    bool is_animating;
    char __padding[2];
    void (*callback)(int selected_index, void *user_data);
    void *user_data;
    int element_hovered_over;
    GooeyTimer *animation_timer;
    int animation_height;
    int target_height;
    int animation_step;
    int start_height;
} GooeyDropdown;

typedef struct
{
    char title[128];
    char *menu_elements[MAX_MENU_CHILDREN];
    void (*callbacks[MAX_MENU_CHILDREN])();
    void *user_data[MAX_MENU_CHILDREN];
    int menu_elements_count;
    bool is_open;
    char __padding[3];
    int element_hovered_over;

    // Animation fields
    bool is_animating;
    int animation_step;
    int animation_height;
    int start_height;
    int target_height;
    GooeyTimer *animation_timer;
} GooeyMenuChild;

typedef struct
{
    GooeyMenuChild children[MAX_MENU_CHILDREN];
    int children_count;
    bool is_busy;
    char __padding[7];
} GooeyMenu;

typedef enum
{
    LAYOUT_HORIZONTAL,
    LAYOUT_VERTICAL,
    LAYOUT_GRID
} GooeyLayoutType;

typedef struct
{
    GooeyWidget core;
    GooeyLayoutType layout_type;
    int padding;
    int margin;
    int rows;
    int cols;
    void *widgets[MAX_WIDGETS];
    int widget_count;
} GooeyLayout;

typedef struct
{
    CANVA_DRAW_OP operation;
    void *args;
} CanvaElement;

typedef struct
{
    GooeyWidget core;
    CanvaElement *elements;
    int element_count;
    void (*callback)(int x, int y, void *user_data);
    void *user_data;
} GooeyCanvas;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    unsigned long color;
    bool is_filled;
    bool is_rounded;
    char __padding[2];
    float thickness;
    float corner_radius;
} CanvasDrawRectangleArgs;

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
    unsigned long color;
} CanvasDrawLineArgs;

typedef struct
{
    int x_center;
    int y_center;
    int width;
    int height;
    int angle1;
    int angle2;
} CanvasDrawArcArgs;

typedef struct
{
    unsigned long color;
} CanvasSetFGArgs;

typedef enum
{
    GOOEY_PLOT_LINE,
    GOOEY_PLOT_SCATTER,
    GOOEY_PLOT_BAR,
    GOOEY_PLOT_HISTOGRAM,
    GOOEY_PLOT_PIE,
    GOOEY_PLOT_BOX,
    GOOEY_PLOT_VIOLIN,
    GOOEY_PLOT_DENSITY,
    GOOEY_PLOT_ECDF,
    GOOEY_PLOT_STACKED_BAR,
    GOOEY_PLOT_GROUPED_BAR,
    GOOEY_PLOT_AREA,
    GOOEY_PLOT_STACKED_AREA,
    GOOEY_PLOT_BUBBLE,
    GOOEY_PLOT_HEATMAP,
    GOOEY_PLOT_CONTOUR,
    GOOEY_PLOT_3D_SCATTER,
    GOOEY_PLOT_3D_SURFACE,
    GOOEY_PLOT_3D_WIREFRAME,
    GOOEY_PLOT_NETWORK,
    GOOEY_PLOT_TREE,
    GOOEY_PLOT_SANKEY,
    GOOEY_PLOT_TIME_SERIES,
    GOOEY_PLOT_CANDLESTICK,
    GOOEY_PLOT_OHLC,
    GOOEY_PLOT_CORRELOGRAM,
    GOOEY_PLOT_PAIRPLOT,
    GOOEY_PLOT_POLAR,
    GOOEY_PLOT_RADAR,
    GOOEY_PLOT_WATERFALL,
    GOOEY_PLOT_FUNNEL,
    GOOEY_PLOT_GANTT,
    GOOEY_PLOT_COUNT
} GOOEY_PLOT_TYPE;

typedef struct
{
    float *x_data;
    float *y_data;
    size_t data_count;
    char *x_label;
    float x_step;
    char *y_label;
    float y_step;
    char *title;
    float max_x_value;
    float min_x_value;
    float max_y_value;
    float min_y_value;
    const char **bar_labels;
    GOOEY_PLOT_TYPE plot_type;
    float custom_x_step;
    float custom_y_step;
} GooeyPlotData;

typedef struct
{
    GooeyWidget core;
    GooeyPlotData *data;
} GooeyPlot;

typedef struct
{
    GooeyWidget core;
    unsigned int texture_id;
    bool is_loaded;
    bool needs_refresh;
    char __padding[2];
    void (*callback)(void *user_data);
    void *user_data;
    const char *image_path;
} GooeyImage;

typedef struct
{
    GooeyWidget core;
    void (*callback)(char *mime, char *file_path, void *user_data);
    void *user_data;
    char default_message[64];
    bool is_file_dropped;
    char __padding[3];
    unsigned long file_icon_texture_id;
} GooeyDropSurface;

typedef struct
{
    char tab_name[64];
    size_t tab_id;
    void **widgets;
    size_t widget_count;
} GooeyTab;

typedef struct
{
    GooeyWidget core;
    GooeyTab *tabs;
    size_t tab_count;
    size_t active_tab_id;
    bool is_sidebar;
    bool is_open;
    bool is_animating;
    char __padding[5];
    int sidebar_offset;
    int target_offset;
    int current_step;
    GooeyTimer *animation_timer;
} GooeyTabs;

typedef struct
{
    GooeyWidget core;
    bool is_toggled;
    bool show_hints;
    char __padding[6];
    void (*callback)(bool state, void *user_data);
    void *user_data;

       bool is_animating;
    int animation_step;
    int thumb_position;
    int start_position;
    int target_position;
    GooeyTimer *animation_timer;
} GooeySwitch;

typedef struct
{
    const char *appname;
} GooeyAppbar;

typedef struct
{
    GooeyWidget core;
    long value;
    const char *label;
    unsigned long texture_id;

} GooeyMeter;

typedef struct
{
    GooeyWidget core;
    bool is_shown;
    char __padding[7];
    size_t text_widget_id;
} GooeyVK;

typedef struct
{
    GooeyWidget core;
    char url[256];
    bool needs_refresh;
    char __padding[7];
} GooeyWebview;

// Node Editor Structures
typedef struct GooeyNodeEditor GooeyNodeEditor;
typedef struct GooeyNode GooeyNode;
typedef struct GooeyNodeSocket GooeyNodeSocket;
typedef struct GooeyNodeConnection GooeyNodeConnection;

typedef enum {
    GOOEY_SOCKET_TYPE_INPUT,
    GOOEY_SOCKET_TYPE_OUTPUT
} GooeySocketType;

typedef enum {
    GOOEY_DATA_TYPE_FLOAT,
    GOOEY_DATA_TYPE_INT,
    GOOEY_DATA_TYPE_BOOL,
    GOOEY_DATA_TYPE_STRING,
    GOOEY_DATA_TYPE_CUSTOM
} GooeyDataType;

struct GooeyNodeSocket {
    char id[32];
    char name[32];
    GooeySocketType type;
    GooeyDataType data_type;
    int x, y;
    bool is_connected;
    GooeyNode* parent_node;
};

struct GooeyNode {
    char node_id[32];
    char title[32];
    int x, y;
    int width, height;
    GooeyNodeSocket* sockets;
    int socket_count;
    bool is_selected;
    bool is_dragging;
    int drag_offset_x, drag_offset_y;
    void* user_data;
};

struct GooeyNodeConnection {
    GooeyNodeSocket* from_socket;
    GooeyNodeSocket* to_socket;
    bool is_selected;
    void* user_data;
};

struct GooeyNodeEditor {
    GooeyWidget core;
    GooeyNode** nodes;
    GooeyNodeConnection** connections;
    int node_count;
    int connection_count;
    int grid_size;
    bool show_grid;
    float zoom_level;
    int pan_x, pan_y;
    bool is_panning;
    int pan_start_x, pan_start_y;
    GooeyNodeSocket* dragging_socket;
    GooeyNodeConnection* temp_connection;
    void (*callback)(void *user_data);
    void *user_data;
};

typedef enum
{
    WINDOW_REGULAR,
    WINDOW_MSGBOX
} WINDOW_TYPE;

typedef struct
{
    WINDOW_TYPE type;
    int creation_id;
    int width;
    int height;
    bool visibility;
    bool enable_debug_overlay;
    bool continuous_redraw;
    char __padding[5];
    GooeyCtxMenu *ctx_menu;
    GooeyAppbar *appbar;
    GooeyVK *vk;
    GooeyButton **buttons;
    GooeyLabel **labels;
    GooeyCheckbox **checkboxes;
    GooeyRadioButton **radio_buttons;
    GooeySlider **sliders;
    GooeyDropdown **dropdowns;
    GooeyRadioButtonGroup **radio_button_groups;
    GooeyTextbox **textboxes;
    GooeyLayout **layouts;
    GooeyMenu *menu;
    GooeyList **lists;
    GooeyCanvas **canvas;
    GooeyPlot **plots;
    GooeyProgressBar **progressbars;
    GooeyWidget **widgets;
    void *current_event;
    GooeyTheme *active_theme;
    GooeyTheme *default_theme;
    GooeyImage **images;
    GooeyDropSurface **drop_surface;
    GooeyTabs **tabs;
    GooeyMeter **meters;
    GooeyContainers **containers;
    GooeySwitch **switches;
    GooeyWebview **webviews;
    GooeyNodeEditor **node_editors;
    size_t node_editor_count;
    size_t webview_count;
    size_t container_count;
    size_t switch_count;
    size_t meter_count;
    size_t tab_count;
    size_t drop_surface_count;
    size_t list_count;
    size_t image_count;
    size_t scrollable_count;
    size_t button_count;
    size_t label_count;
    size_t checkbox_count;
    size_t radio_button_count;
    size_t slider_count;
    size_t dropdown_count;
    size_t textboxes_count;
    size_t layout_count;
    size_t radio_button_group_count;
    size_t canvas_count;
    size_t plot_count;
    size_t progressbar_count;
    size_t widget_count;
    void* memory_pool;
} GooeyWindow;

#pragma pack(pop)

#endif