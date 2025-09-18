#ifndef USER_CONFIG_H
#define USER_CONFIG_H

// DEBUG/PROD
#define PROJECT_BRANCH 1 // 0=debug 1=prod TODO

#define APPBAR_HEIGHT 50
// MAX
#define MAX_TIMERS 100 /** Maximum number of timer objects. */
#define MAX_WIDGETS                                                            \
  100 /** Maximum number of widgets that can be added to a window. */
#define MAX_MENU_CHILDREN 10 /** Maximum number of menu children. */
#define MAX_RADIO_BUTTONS 10 /** Maximum number of radio buttons in a group.   \
                              */
#define MAX_PLOT_COUNT 100   /** Maximum number of plots. */
#define MAX_TABS 50          /** Maximum number of tabs. */
#define MAX_CONTAINER 50     /** Maximum number of Containers. */
#define MAX_SWITCHES 50
// ENABLE/DISABLE WIDGETS
#define ENABLE_BUTTON 1
#define ENABLE_CANVAS 1
#define ENABLE_CHECKBOX 1
#define ENABLE_CTX_MENU 1 // TODO
#define ENABLE_DEBUG_OVERLAY 1
#define ENABLE_DROP_SURFACE 1
#define ENABLE_DROPDOWN 1
#define ENABLE_IMAGE 1
#define ENABLE_LABEL 1
#define ENABLE_LAYOUT 1
#define ENABLE_LIST 1
#define ENABLE_MENU 1
#define ENABLE_METER 1
#define ENABLE_PLOT 1
#define ENABLE_PROGRESSBAR 1
#define ENABLE_RADIOBUTTON 1
#define ENABLE_SLIDER 1
#define ENABLE_TABS 1
#define ENABLE_TEXTBOX 1
#define ENABLE_CONTAINER 1
#define ENABLE_APPBAR 1
#define ENABLE_SWITCH 1
#define ENABLE_WEBVIEW 1

// ESP32 CONFIGURATION
#define TFT_ESPI_ENABLED ESP32
#define TFT_SCREEN_ROTATION 3
#define ENABLE_VIRTUAL_KEYBOARD 0

#endif
