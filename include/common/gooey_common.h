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

/**
 * @brief Enumeration for widget types in the Gooey framework.
 * @note add new widget between widet label and tabs
 */
typedef enum
{
    WIDGET_LABEL,       /**< Label widget */
    WIDGET_SLIDER,      /**< Slider widget */
    WIDGET_RADIOBUTTON, /**< Radio button widget */
    WIDGET_CHECKBOX,    /**< Checkbox widget */
    WIDGET_BUTTON,      /**< Button widget */
    WIDGET_TEXTBOX,     /**< Textbox widget */
    WIDGET_DROPDOWN,    /**< Dropdown widget */
    WIDGET_CANVAS,      /**< Canvas widget */
    WIDGET_LAYOUT,      /**< Layout widget */
    WIDGET_PLOT,        /**< Plot widget */
    WIDGET_DROP_SURFACE,
    WIDGET_IMAGE,
    WIDGET_LIST,
    WIDGET_PROGRESSBAR,
    WIDGET_METER,
    WIDGET_CONTAINER,
    WIDGET_SWITCH,
    WIDGET_WEBVIEW,
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

} GooeyTFT_Sprite;

typedef struct
{
    void *timer_ptr;
} GooeyTimer;

/**
 * @brief A base structure for all Gooey widgets containing their position and size.
 */
typedef struct
{
    GooeyTFT_Sprite *sprite;

    WIDGET_TYPE type;  /**< Type of the widget */
    bool is_visible;   /**< Visibility of the widget */
    int x, y;          /**< Position of the widget (top-left corner) */
    int width, height; /**< Dimensions of the widget */
} GooeyWidget;

/**
 * @brief Enumeration for cursor types in the Gooey framework.
 */
typedef enum
{
    GOOEY_CURSOR_ARROW,        /**< The regular arrow cursor shape. */
    GOOEY_CURSOR_TEXT,         /**< The text input I-beam cursor shape. */
    GOOEY_CURSOR_CROSSHAIR,    /**< The crosshair cursor shape. */
    GOOEY_CURSOR_HAND,         /**< The pointing hand cursor shape. */
    GOOEY_CURSOR_RESIZE_H,     /**< The horizontal resize/move arrow shape. */
    GOOEY_CURSOR_RESIZE_V,     /**< The vertical resize/move arrow shape. */
    GOOEY_CURSOR_RESIZE_TL_BR, /**< The top-left to bottom-right diagonal resize/move arrow shape. */
    GOOEY_CURSOR_RESIZE_TR_BL, /**< The top-right to bottom-left diagonal resize/move arrow shape. */
    GOOEY_CURSOR_RESIZE_ALL,   /**< The omni-directional resize/move cursor shape. */
    GOOEY_CURSOR_NOT_ALLOWED   /**< The operation-not-allowed shape. */
} GOOEY_CURSOR;

typedef enum
{
    SLIDER_HORIZONTAL,
    SLIDER_VERTICAL
} SLIDER_ORIENTATION;

/**
 * @brief Enumeration for message box types in the Gooey framework.
 */
typedef enum
{
    MSGBOX_SUCCES, /**< Success message box */
    MSGBOX_INFO,   /**< Information message box */
    MSGBOX_FAIL    /**< Failure message box */
} MSGBOX_TYPE;

/**
 * @brief A structure representing a button widget.
 */
typedef struct
{
    GooeyWidget core;    /**< Core widget properties */
    char label[256];     /**< Text label of the button */
    void (*callback)();  /**< Callback function for button click */
    bool clicked;        /**< State of the button (clicked or not) */
    bool hover;          /**< State of the button (hovered or not) */
    bool is_highlighted; /**< Whether the button is highlighted */
    int click_timer;     /**< Timer for click effect */
    bool is_disabled;    /**< Whether the button is disabled */
} GooeyButton;
/**
 * @brief Container widget.
 */
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

/**
 * @brief Enumeration for canvas drawing operations.
 */
typedef enum
{
    CANVA_DRAW_RECT,  /**< Draw a rectangle */
    CANVA_DRAW_LINE,  /**< Draw a line */
    CANVA_DRAW_ARC,   /**< Draw an arc */
    CANVA_DRAW_SET_FG /**< Set the foreground color */
} CANVA_DRAW_OP;

/**
 * @brief A structure representing a textbox widget.
 */
typedef struct
{
    GooeyWidget core;             /**< Core widget properties */
    char text[256];               /**< Text in the textbox */
    char placeholder[256];        /**< Placeholder text for the textbox */
    bool focused;                 /**< Whether the textbox is focused */
    int cursor_pos;               /**< Position of the cursor in the text */
    int scroll_offset;            /**< Scroll offset of the textbox */
    void (*callback)(char *text); /**< Callback when text changes */
    bool is_password;             /**< Whether the textbox is a password field */
} GooeyTextbox;

/**
 * @brief A structure representing a label widget.
 */
typedef struct
{
    GooeyWidget core;    /**< Core widget properties */
    char text[256];      /**< Text displayed on the label */
    float font_size;     /**< Font size of the label */
    unsigned long color; /**< Color of the label */
    bool is_using_custom_color;
} GooeyLabel;

/**
 * @brief A structure representing a checkbox widget.
 */
typedef struct
{
    GooeyWidget core;               /**< Core widget properties */
    bool checked;                   /**< Whether the checkbox is checked or not */
    char label[256];                /**< Label for the checkbox */
    void (*callback)(bool checked); /**< Callback function when the checkbox is clicked */
} GooeyCheckbox;

/**
 * @brief A structure representing a list widget item.
 */
typedef struct
{
    char title[256];       /**< Title of the list item */
    char description[256]; /**< Description of the list item */
} GooeyListItem;

/**
 * @brief A structure representing a list widget.
 */
typedef struct
{
    GooeyWidget core;            /**< Core widget properties */
    GooeyListItem *items;        /**< Items in a list widget */
    int scroll_offset;           /**< Mouse scroll offset */
    int thumb_y;                 /**< Thumb's y-coordinate */
    int thumb_height;            /**< Thumb's height */
    int thumb_width;             /**< Thumb's width */
    int item_spacing;            /**< Item spacing */
    size_t item_count;           /**< List widget item count */
    bool show_separator;         /**< Show or hide separator */
    void (*callback)(int index); /**< Callback function for item selection */
    int element_hovered_over;
} GooeyList;

/**
 * @brief A structure representing a radio button widget.
 */
typedef struct
{
    GooeyWidget core;                /**< Core widget properties */
    bool selected;                   /**< Whether the radio button is selected or not */
    char label[256];                 /**< Label for the radio button */
    int radius;                      /**< Radius of the radio button */
    void (*callback)(bool selected); /**< Callback function when the radio button is selected */
} GooeyRadioButton;

/**
 * @brief A structure representing a group of radio buttons.
 */
typedef struct
{
    GooeyRadioButton buttons[MAX_RADIO_BUTTONS]; /**< List of radio buttons in the group */
    int button_count;                            /**< Number of radio buttons in the group */
} GooeyRadioButtonGroup;

/**
 * @brief A structure representing a slider widget.
 */
typedef struct
{
    GooeyWidget core;             /**< Core widget properties */
    long value;                   /**< Current value of the slider */
    long min_value;               /**< Minimum value of the slider */
    long max_value;               /**< Maximum value of the slider */
    bool show_hints;              /**< Whether to show hints for the slider */
    void (*callback)(long value); /**< Callback function when the slider value changes */
    SLIDER_ORIENTATION orientation;
} GooeySlider;

typedef struct
{
    GooeyWidget core;
    long value;
} GooeyProgressBar;

/**
 * @brief A structure representing a dropdown menu widget.
 */
typedef struct
{
    GooeyWidget core;                     /**< Core widget properties */
    int selected_index;                   /**< Index of the selected item */
    const char **options;                 /**< List of options for the dropdown */
    int num_options;                      /**< Number of options in the dropdown */
    bool is_open;                         /**< Whether the dropdown menu is open */
    void (*callback)(int selected_index); /**< Callback function when an option is selected */
    int element_hovered_over;
} GooeyDropdown;

/**
 * @brief A structure representing a menu item in the window.
 */
typedef struct
{
    char title[128];                        /**< Title of the menu item */
    char *menu_elements[MAX_MENU_CHILDREN]; /**< List of child menu elements */
    void (*callbacks[MAX_MENU_CHILDREN])(); /**< List of callback functions for child menu elements */
    int menu_elements_count;                /**< Number of child menu elements */
    bool is_open;                           /**< Whether the menu is open or not */
    int element_hovered_over;
} GooeyMenuChild;

/**
 * @brief A structure representing a menu widget.
 */
typedef struct
{
    GooeyMenuChild children[MAX_MENU_CHILDREN]; /**< List of menu items */
    int children_count;                         /**< Number of menu items */
    bool is_busy;                               /**< Whether the menu is busy */
} GooeyMenu;

/**
 * @brief Enumeration for layout types of widgets in the Gooey framework.
 */
typedef enum
{
    LAYOUT_HORIZONTAL, /**< Horizontal layout */
    LAYOUT_VERTICAL,   /**< Vertical layout */
    LAYOUT_GRID        /**< Grid layout */
} GooeyLayoutType;

/**
 * @brief A structure representing the layout of widgets in a window.
 */
typedef struct
{
    GooeyWidget core;            /**< Core widget properties */
    GooeyLayoutType layout_type; /**< Type of the layout (horizontal, vertical, or grid) */
    int padding;                 /**< Padding around the layout */
    int margin;                  /**< Margin around the layout */
    int rows;                    /**< Number of rows in the layout (for grid layouts) */
    int cols;                    /**< Number of columns in the layout (for grid layouts) */
    void *widgets[MAX_WIDGETS];  /**< List of widgets in the layout */
    int widget_count;            /**< Number of widgets in the layout */
} GooeyLayout;

/**
 * @brief A structure representing a canvas drawing operation.
 */
typedef struct
{
    CANVA_DRAW_OP operation; /**< Type of drawing operation */
    void *args;              /**< Arguments for the drawing operation */
} CanvaElement;

/**
 * @brief A structure representing a canvas widget.
 */
typedef struct
{
    GooeyWidget core;       /**< Core widget properties */
    CanvaElement *elements; /**< List of drawing operations */
    int element_count;      /**< Number of drawing operations */
    void (*callback)(int x, int y);
} GooeyCanvas;

/**
 * @brief Arguments for drawing a rectangle on a canvas.
 */
typedef struct
{
    int x;               /**< X-coordinate of the rectangle */
    int y;               /**< Y-coordinate of the rectangle */
    int width;           /**< Width of the rectangle */
    int height;          /**< Height of the rectangle */
    unsigned long color; /**< Color of the rectangle */
    bool is_filled;      /**< Whether the rectangle is filled */
    bool is_rounded;
    float thickness;
    float corner_radius;
} CanvasDrawRectangleArgs;

/**
 * @brief Arguments for drawing a line on a canvas.
 */
typedef struct
{
    int x1;              /**< X-coordinate of the starting point */
    int y1;              /**< Y-coordinate of the starting point */
    int x2;              /**< X-coordinate of the ending point */
    int y2;              /**< Y-coordinate of the ending point */
    unsigned long color; /**< Color of the line */
} CanvasDrawLineArgs;

/**
 * @brief Arguments for drawing an arc on a canvas.
 */
typedef struct
{
    int x_center; /**< X-coordinate of the center */
    int y_center; /**< Y-coordinate of the center */
    int width;    /**< Width of the arc */
    int height;   /**< Height of the arc */
    int angle1;   /**< Starting angle of the arc */
    int angle2;   /**< Ending angle of the arc */
} CanvasDrawArcArgs;

/**
 * @brief Arguments for setting the foreground color on a canvas.
 */
typedef struct
{
    unsigned long color; /**< Foreground color */
} CanvasSetFGArgs;

/**
 * @brief Enumeration for plot types in the Gooey framework.
 */
typedef enum
{
    GOOEY_PLOT_LINE,         /**< Line Plot */
    GOOEY_PLOT_SCATTER,      /**< Scatter Plot */
    GOOEY_PLOT_BAR,          /**< Bar Chart */
    GOOEY_PLOT_HISTOGRAM,    /**< Histogram */
    GOOEY_PLOT_PIE,          /**< Pie Chart */
    GOOEY_PLOT_BOX,          /**< Box Plot */
    GOOEY_PLOT_VIOLIN,       /**< Violin Plot */
    GOOEY_PLOT_DENSITY,      /**< Density Plot (KDE) */
    GOOEY_PLOT_ECDF,         /**< Empirical CDF */
    GOOEY_PLOT_STACKED_BAR,  /**< Stacked Bar Chart */
    GOOEY_PLOT_GROUPED_BAR,  /**< Grouped Bar Chart */
    GOOEY_PLOT_AREA,         /**< Area Chart */
    GOOEY_PLOT_STACKED_AREA, /**< Stacked Area Chart */
    GOOEY_PLOT_BUBBLE,       /**< Bubble Chart */
    GOOEY_PLOT_HEATMAP,      /**< Heatmap */
    GOOEY_PLOT_CONTOUR,      /**< Contour Plot */
    GOOEY_PLOT_3D_SCATTER,   /**< 3D Scatter Plot */
    GOOEY_PLOT_3D_SURFACE,   /**< 3D Surface Plot */
    GOOEY_PLOT_3D_WIREFRAME, /**< 3D Wireframe Plot */
    GOOEY_PLOT_NETWORK,      /**< Graph/Network Plot */
    GOOEY_PLOT_TREE,         /**< Tree Diagram */
    GOOEY_PLOT_SANKEY,       /**< Sankey Diagram */
    GOOEY_PLOT_TIME_SERIES,  /**< Time-Series Plot */
    GOOEY_PLOT_CANDLESTICK,  /**< Candlestick Chart */
    GOOEY_PLOT_OHLC,         /**< Open-High-Low-Close Chart */
    GOOEY_PLOT_CORRELOGRAM,  /**< Correlation Matrix */
    GOOEY_PLOT_PAIRPLOT,     /**< Pairwise Scatter Plot */
    GOOEY_PLOT_POLAR,        /**< Polar Plot */
    GOOEY_PLOT_RADAR,        /**< Radar Chart (Spider Plot) */
    GOOEY_PLOT_WATERFALL,    /**< Waterfall Chart */
    GOOEY_PLOT_FUNNEL,       /**< Funnel Chart */
    GOOEY_PLOT_GANTT,        /**< Gantt Chart */
    GOOEY_PLOT_COUNT         /**< Total count of plot types */
} GOOEY_PLOT_TYPE;

/**
 * @brief A structure representing plot data and metadata.
 */
typedef struct
{
    float *x_data;             /**< Pointer to X-axis data */
    float *y_data;             /**< Pointer to Y-axis data */
    size_t data_count;         /**< Number of data points */
    char *x_label;             /**< Label for X-axis */
    float x_step;              /**< Step size for X-axis */
    char *y_label;             /**< Label for Y-axis */
    float y_step;              /**< Step size for Y-axis */
    char *title;               /**< Plot title */
    float max_x_value;         /**< Maximum X-axis value */
    float min_x_value;         /**< Minimum X-axis value */
    float max_y_value;         /**< Maximum Y-axis value */
    float min_y_value;         /**< Minimum Y-axis value */
    const char **bar_labels;   /**< Labels for bar plots */
    GOOEY_PLOT_TYPE plot_type; /**< Type of the plot */
} GooeyPlotData;

/**
 * @brief A structure representing a plot widget.
 */
typedef struct
{
    GooeyWidget core;    /**< Core widget properties */
    GooeyPlotData *data; /**< Pointer to plot data */
} GooeyPlot;

/**
 * @brief A structure representing an image widget.
 */
typedef struct
{
    GooeyWidget core;        /**< Core widget properties */
    unsigned int texture_id; /**< Texture ID for the image */
    void (*callback)(void);  /**< Callback function for image interaction */
    bool needs_refresh;
    const char *image_path; /**< Path to the image file */
} GooeyImage;

/**
 * @brief A structure representing a drop surface widget.
 */
typedef struct
{
    GooeyWidget core;                              /**< Core widget properties */
    void (*callback)(char *mime, char *file_path); /**< Callback function for file drop */
    char default_message[64];                      /**< Default message displayed on the drop surface */
    bool is_file_dropped;                          /**< Whether a file has been dropped */
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
} GooeyTabs;

// gooey switch struct
typedef struct
{
    GooeyWidget core;
    bool is_toggled;
    bool show_hints;
    void (*callback)(bool state);
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
    size_t text_widget_id;
} GooeyVK;

typedef struct
{
    GooeyWidget core;
    char url[256];
    bool needs_refresh;
} GooeyWebview;

/**
 * @brief Enumeration for window types in the Gooey framework.
 */
typedef enum
{
    WINDOW_REGULAR, /**< Regular window */
    WINDOW_MSGBOX   /**< Message box window */
} WINDOW_TYPE;

/**
 * @brief A structure representing a window containing various widgets.
 */
typedef struct
{
    WINDOW_TYPE type; /**< Type of the window */
    int creation_id;  /**< Unique window ID */
    int width;
    int height;
    bool visibility; /**< Whether the window is visible */
    bool enable_debug_overlay;
    bool continuous_redraw;
    GooeyAppbar *appbar;
    GooeyVK *vk;
    GooeyButton **buttons;                       /**< List of buttons in the window */
    GooeyLabel **labels;                         /**< List of labels in the window */
    GooeyCheckbox **checkboxes;                  /**< List of checkboxes in the window */
    GooeyRadioButton **radio_buttons;            /**< List of radio buttons in the window */
    GooeySlider **sliders;                       /**< List of sliders in the window */
    GooeyDropdown **dropdowns;                   /**< List of dropdown menus in the window */
    GooeyRadioButtonGroup **radio_button_groups; /**< List of radio button groups in the window */
    GooeyTextbox **textboxes;                    /**< List of textboxes in the window */
    GooeyLayout **layouts;                       /**< List of layouts in the window */
    GooeyMenu *menu;                             /**< Menu in the window */
    GooeyList **lists;                           /**< List of list widgets in the window */
    GooeyCanvas **canvas;                        /**< List of canvas widgets in the window */
    GooeyPlot **plots;                           /**< List of plot widgets in the window */
    GooeyProgressBar **progressbars;
    GooeyWidget **widgets;           /**< Unified list of all widgets in the window */
    void *current_event;             /**< Current event being processed */
    GooeyTheme *active_theme;        /**< Active theme for the window */
    GooeyTheme *default_theme;       /**< Default theme for the window */
    GooeyImage **images;             /**< List of image widgets in the window */
    GooeyDropSurface **drop_surface; /**< List of drop surface widgets in the window */
    GooeyTabs **tabs;
    GooeyMeter **meters;
    GooeyContainers **containers;
    GooeySwitch **switches;
    GooeyWebview **webviews;


    size_t webview_count;
    size_t container_count;
    size_t switch_count;
    size_t meter_count;
    size_t tab_count;
    size_t drop_surface_count;       /**< Number of drop surface widgets */
    size_t list_count;               /**< Number of list widgets */
    size_t image_count;              /**< Number of image widgets */
    size_t scrollable_count;         /**< Number of scrollable widgets */
    size_t button_count;             /**< Number of buttons */
    size_t label_count;              /**< Number of labels */
    size_t checkbox_count;           /**< Number of checkboxes */
    size_t radio_button_count;       /**< Number of radio buttons */
    size_t slider_count;             /**< Number of sliders */
    size_t dropdown_count;           /**< Number of dropdown menus */
    size_t textboxes_count;          /**< Number of textboxes */
    size_t layout_count;             /**< Number of layouts */
    size_t radio_button_group_count; /**< Number of radio button groups */
    size_t canvas_count;             /**< Number of canvas widgets */
    size_t plot_count;               /**< Number of plot widgets */
    size_t progressbar_count;
    size_t widget_count; /**< Total number of widgets */
} GooeyWindow;

#endif
