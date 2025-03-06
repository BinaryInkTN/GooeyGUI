/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GOOEY_WIDGETS_INTERNAL_H
#define GOOEY_WIDGETS_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include "gooey_event_internal.h"

/** Maximum number of widgets that can be added to a window. */
#define MAX_WIDGETS 100

/** Size of a checkbox widget. */
#define CHECKBOX_SIZE 20

/** Radius of the radio button widget. */
#define RADIO_BUTTON_RADIUS 10

/** Width of the slider widget. */
#define SLIDER_WIDTH 100

/** Height of the slider widget. */
#define SLIDER_HEIGHT 5

/** Maximum number of menu children. */
#define MAX_MENU_CHILDREN 10

/** Maximum number of radio buttons in a group. */
#define MAX_RADIO_BUTTONS 10

/**< Maxiumum number of plots. */
#define MAX_PLOT_COUNT 100

/**
 * @brief Enumeration for widget types in the Gooey framework.
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
  WIDGET_LAYOUT,
  WIDGET_PLOT
} WIDGET_TYPE;

/**
 * @brief A base structure for all Gooey widgets containing their position and size.
 */
typedef struct
{
  WIDGET_TYPE type;  /**< Type of the widget */
  int x, y;          /**< Position of the widget (top-left corner) */
  int width, height; /**< Dimensions of the widget */
} GooeyWidget;

typedef enum
{
  GOOEY_CURSOR_ARROW,        /**< The regular arrow cursor shape. */
  GOOEY_CURSOR_TEXT,         /**< The text input I-beam cursor shape. */
  GOOEY_CURSOR_CROSSHAIR,    /**< The crosshair cursor shape. */
  GOOEY_CURSOR_HAND,         /**< The pointing hand cursor shape. */
  GOOEY_CURSOR_RESIZE_H,     /**< The horizontal resize/move arrow shape. */
  GOOEY_CURSOR_RESIZE_V,     /**<The vertical resize/move arrow shape. */
  GOOEY_CURSOR_RESIZE_TL_BR, /**< The top-left to bottom-right diagonal resize/move arrow shape. */
  GOOEY_CURSOR_RESIZE_TR_BL, /**< The top-right to bottom-left diagonal resize/move arrow shape. */
  GOOEY_CURSOR_RESIZE_ALL,   /**< The omni-directional resize/move cursor shape. */
  GOOEY_CURSOR_NOT_ALLOWED   /**< The operation-not-allowed shape. */
} GOOEY_CURSOR;

typedef enum
{
  MSGBOX_SUCCES,
  MSGBOX_INFO,
  MSGBOX_FAIL
} MSGBOX_TYPE;

/**
 * @brief A structure representing a button widget.
 */
typedef struct
{
  GooeyWidget core;   /**< Core widget properties */
  char label[256];    /**< Text label of the button */
  void (*callback)(); /**< Callback function for button click */
  bool clicked;       /**< State of the button (clicked or not) */
  bool hover;         /**< State of the button (hovered or not) */
  bool is_highlighted;
  int click_timer; /**< Timer for click effect */ 
} GooeyButton;

typedef enum
{
  CANVA_DRAW_RECT,
  CANVA_DRAW_LINE,
  CANVA_DRAW_ARC,
  CANVA_DRAW_SET_FG
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
  int cursor_position;          /**< Cursor position */
  void (*callback)(char *text); /**< Callback when text changes */
} GooeyTextbox;

/**
 * @brief A structure representing a label widget.
 */
typedef struct
{
  GooeyWidget core; /**< Core widget properties */
  char text[256];   /**< Text displayed on the label */
  float font_size;
  unsigned long color;
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
 * @brief A list widget item.
 */
typedef struct
{
  char title[256];
  char description[256];
} GooeyListItem;

/**
 * @brief A structure representing a List widget.
 */
typedef struct
{
  GooeyWidget core;     /**< Core widget properties */
  GooeyListItem *items; /**< Items in a list widget */
  int scroll_offset;    /**< Mouse scroll offset */
  int thumb_y;          /**< Thumb's y-coordinate */
  int thumb_height;     /**< Thumb's height */
  int thumb_width;      /**< Thumb's width */
  int item_spacing;     /**< Item spacing */
  size_t item_count;    /**< List widget item count */
  bool show_separator;  /**< Show or hide separator */
  void (*callback)(int index);
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
} GooeySlider;

/**
 * @brief A structure representing a dropdown menu widget.
 */
typedef struct
{
  GooeyWidget core;                     /**< Core widget properties */
  int selected_index;                   /**< Index of the selected item */
  const char **options;                 /**< List of options for the dropdown */
  int num_options;                      /**< Number of options in the dropdown */
  bool is_open;                         /**< Boolean flag to check if the dropdown menu is opened or not*/
  void (*callback)(int selected_index); /**< Callback function when an option is selected */
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
} GooeyMenuChild;

/**
 * @brief A structure representing a menu widget.
 */
typedef struct
{
  GooeyMenuChild children[MAX_MENU_CHILDREN]; /**< List of menu items */
  int children_count;                         /**< Number of menu items */
  bool is_busy;
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
  GooeyWidget core;

  GooeyLayoutType layout_type; /**< Type of the layout (horizontal, vertical, or grid) */
  int padding;                 /**< Padding around the layout */
  int margin;                  /**< Margin around the layout */
  int rows;                    /**< Number of rows in the layout (for grid layouts) */
  int cols;                    /**< Number of columns in the layout (for grid layouts) */
  void *widgets[MAX_WIDGETS];  /**< List of widgets in the layout */
  int widget_count;            /**< Number of widgets in the layout */
} GooeyLayout;

typedef struct
{
  CANVA_DRAW_OP operation;
  void *args;
} CanvaElement;

/**
 * @brief A structure representing a canvas widget.
 */
typedef struct
{
  GooeyWidget core;       /**< Core widget properties. */
  CanvaElement *elements; /**< Functions registered to draw by user. */
  int element_count;      /**< Count of draw functions. */
} GooeyCanvas;

typedef struct
{
  int x;
  int y;
  int width;
  int height;
  unsigned long color;
  bool is_filled;
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

/**
 * @enum GOOEY_PLOT_TYPE
 * @brief Enumeration of available plot types in Gooey.
 */
typedef enum
{
  GOOEY_PLOT_LINE,      /**< Line Plot */
  GOOEY_PLOT_SCATTER,   /**< Scatter Plot */
  GOOEY_PLOT_BAR,       /**< Bar Chart */
  GOOEY_PLOT_HISTOGRAM, /**< Histogram */
  GOOEY_PLOT_PIE,       /**< Pie Chart */

  GOOEY_PLOT_BOX,     /**< Box Plot */
  GOOEY_PLOT_VIOLIN,  /**< Violin Plot */
  GOOEY_PLOT_DENSITY, /**< Density Plot (KDE) */
  GOOEY_PLOT_ECDF,    /**< Empirical CDF */

  GOOEY_PLOT_STACKED_BAR,  /**< Stacked Bar Chart */
  GOOEY_PLOT_GROUPED_BAR,  /**< Grouped Bar Chart */
  GOOEY_PLOT_AREA,         /**< Area Chart */
  GOOEY_PLOT_STACKED_AREA, /**< Stacked Area Chart */
  GOOEY_PLOT_BUBBLE,       /**< Bubble Chart */

  GOOEY_PLOT_HEATMAP, /**< Heatmap */
  GOOEY_PLOT_CONTOUR, /**< Contour Plot */

  GOOEY_PLOT_3D_SCATTER,   /**< 3D Scatter Plot */
  GOOEY_PLOT_3D_SURFACE,   /**< 3D Surface Plot */
  GOOEY_PLOT_3D_WIREFRAME, /**< 3D Wireframe Plot */

  GOOEY_PLOT_NETWORK, /**< Graph/Network Plot */
  GOOEY_PLOT_TREE,    /**< Tree Diagram */
  GOOEY_PLOT_SANKEY,  /**< Sankey Diagram */

  GOOEY_PLOT_TIME_SERIES, /**< Time-Series Plot */
  GOOEY_PLOT_CANDLESTICK, /**< Candlestick Chart */
  GOOEY_PLOT_OHLC,        /**< Open-High-Low-Close Chart */

  GOOEY_PLOT_CORRELOGRAM, /**< Correlation Matrix */
  GOOEY_PLOT_PAIRPLOT,    /**< Pairwise Scatter Plot */

  GOOEY_PLOT_POLAR,     /**< Polar Plot */
  GOOEY_PLOT_RADAR,     /**< Radar Chart (Spider Plot) */
  GOOEY_PLOT_WATERFALL, /**< Waterfall Chart */
  GOOEY_PLOT_FUNNEL,    /**< Funnel Chart */
  GOOEY_PLOT_GANTT,     /**< Gantt Chart */

  GOOEY_PLOT_COUNT /**< Total count of plot types */
} GOOEY_PLOT_TYPE;

/**
 * @struct GooeyPlotData
 * @brief Structure to hold plot data and metadata.
 */
typedef struct
{
  float *x_data; /**< Pointer to X-axis data */
  float *y_data; /**< Pointer to Y-axis data */

  size_t data_count; /**< Number of data points */

  char *x_label; /**< Label for X-axis */
  float x_step;
  char *y_label; /**< Label for Y-axis */
  float y_step;
  char *title; /**< Plot title */

  float max_x_value;
  float min_x_value;
  float max_y_value;
  float min_y_value;

  // BAR PLOT SPECIFIC
  const char **bar_labels;

  GOOEY_PLOT_TYPE plot_type; /**< Type of the plot */

} GooeyPlotData;

/**
 * @struct GooeyPlot
 * @brief Represents a plot widget in the Gooey GUI system.
 *
 * This structure encapsulates a plot widget, inheriting core widget properties
 * from GooeyWidget and containing a pointer to GooeyPlotData, which holds
 * the actual plotting data.
 */
typedef struct
{
  GooeyWidget core;    /**< Base widget properties for integration in the Gooey GUI system */
  GooeyPlotData *data; /**< Pointer to the data structure containing plot-specific information */
} GooeyPlot;


typedef enum
{
    WINDOW_REGULAR,
    WINDOW_MSGBOX

} WINDOW_TYPE;

/**
 * @brief A structure representing a window containing various widgets.
 */
typedef struct
{

    int creation_id; /**< Unique window ID. */
    bool visibility;
    WINDOW_TYPE type;

    GooeyButton *buttons;                       /**< List of buttons in the window */
    GooeyLabel *labels;                         /**< List of labels in the window */
    GooeyCheckbox *checkboxes;                  /**< List of checkboxes in the window */
    GooeyRadioButton *radio_buttons;            /**< List of radio buttons in the window */
    GooeySlider *sliders;                       /**< List of sliders in the window */
    GooeyDropdown *dropdowns;                   /**< List of dropdown menus in the window */
    GooeyRadioButtonGroup *radio_button_groups; /**< List of radio button groups in the window */
    GooeyTextbox *textboxes;                    /**< List of textboxes in the window */
    GooeyLayout *layouts;                       /**< List of layouts in the window */
    GooeyMenu *menu;                            /**< Menu in the window */
    GooeyList *lists;                           /**< List of list widgets in the window. */
    GooeyCanvas *canvas;                        /**< List of canvas widgets in the window. */
    GooeyPlot *plots;                            /**< List of all plot widgets. */
    GooeyWidget **widgets;                      /**< List containing unified definition of every widget. */
    GooeyEvent *current_event;

    size_t list_count;
    size_t scrollable_count;         /**< Number of scrollables in the window */
    size_t button_count;             /**< Number of buttons in the window */
    size_t label_count;              /**< Number of labels in the window */
    size_t checkbox_count;           /**< Number of checkboxes in the window */
    size_t radio_button_count;       /**< Number of radio buttons in the window */
    size_t slider_count;             /**< Number of sliders in the window */
    size_t dropdown_count;           /**< Number of dropdown menus in the window */
    size_t textboxes_count;          /**< Number of textboxes in the window */
    size_t layout_count;             /**< Number of layouts in the window */
    size_t radio_button_group_count; /**< Number of radio button groups in the window */
    size_t canvas_count;             /**< Number of all canvas widgets in the window */
    size_t plot_count;               /**< Number of all plot widgets. */
    size_t widget_count;             /**< Total number of registered widgets in the window. */



} GooeyWindow;


#endif
