#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/*

   /$$$$$$                                          /$$   /$$ /$$$$$$
 /$$__  $$                                        | $$  | $$|_  $$_/
| $$  \__/  /$$$$$$   /$$$$$$   /$$$$$$  /$$   /$$| $$  | $$  | $$  
| $$ /$$$$ /$$__  $$ /$$__  $$ /$$__  $$| $$  | $$| $$  | $$  | $$  
| $$|_  $$| $$  \ $$| $$  \ $$| $$$$$$$$| $$  | $$| $$  | $$  | $$  
| $$  \ $$| $$  | $$| $$  | $$| $$_____/| $$  | $$| $$  | $$  | $$  
|  $$$$$$/|  $$$$$$/|  $$$$$$/|  $$$$$$$|  $$$$$$$|  $$$$$$/ /$$$$$$
 \______/  \______/  \______/  \_______/ \____  $$ \______/ |______/
                                         /$$  | $$                  
                                        |  $$$$$$/                  
                                         \______/                   
                                                                                                                                                                           
* Gooey - A lightweight GUI library for C/C++ applications
*/

/*******************************************************************************
 *                              DEVELOPMENT SETTINGS                           *
 ******************************************************************************/

/**
 * Project Branch Configuration
 * 0 = Production mode (optimized, minimal logging)
 * 1 = Debug mode (verbose logging, additional checks)
 */
#define PROJECT_BRANCH 0

/**
 * Enable debug overlay for development
 * Shows FPS, memory usage, and other debug information
 */
#define ENABLE_DEBUG_OVERLAY 1


/*******************************************************************************
 *                             APPLICATION LAYOUT                              *
 ******************************************************************************/

/** Height of the application title bar in pixels */
#define APPBAR_HEIGHT 50


/*******************************************************************************
 *                          SYSTEM RESOURCE LIMITS                             *
 ******************************************************************************/

/** Maximum number of timer objects that can be created */
#define MAX_TIMERS 100

/** Maximum number of widgets per window */
#define MAX_WIDGETS 100

/** Maximum number of child items in a menu */
#define MAX_MENU_CHILDREN 10

/** Maximum number of radio buttons in a single group */
#define MAX_RADIO_BUTTONS 10

/** Maximum number of data points in plot widgets */
#define MAX_PLOT_COUNT 100

/** Maximum number of tabs in a tab container */
#define MAX_TABS 50

/** Maximum number of container widgets */
#define MAX_CONTAINER 50

/** Maximum number of switch widgets */
#define MAX_SWITCHES 50

/** Maximum number of context menus */
#define GOOEY_CTXMENU_MAX_ITEMS 20

/*******************************************************************************
 *                          WIDGET FEATURE TOGGLES                             *
 * 
 * Enable or disable specific widgets to reduce memory footprint or exclude
 * unused functionality. Disable widgets you don't need to improve performance.
 ******************************************************************************/

/** Button widget - Basic clickable button */
#define ENABLE_BUTTON 1

/** Canvas widget - Custom drawing surface */
#define ENABLE_CANVAS 1

/** Checkbox widget - Binary state toggle */
#define ENABLE_CHECKBOX 1

/** Context menu widget - Right-click context menus */
#define ENABLE_CTXMENU 1 

/** Drop surface widget - Area for drag-and-drop operations */
#define ENABLE_DROP_SURFACE 1

/** Dropdown widget - Selection from a list of options */
#define ENABLE_DROPDOWN 1

/** Image widget - Display static and animated images (GIF support) */
#define ENABLE_IMAGE 1

/** Label widget - Text display */
#define ENABLE_LABEL 1

/** Layout widget - Container for organizing child widgets */
#define ENABLE_LAYOUT 1

/** List widget - Scrollable list of items */
#define ENABLE_LIST 1

/** Menu widget - Application menu bar */
#define ENABLE_MENU 1

/** Meter widget - Visual gauge display */
#define ENABLE_METER 1

/** Plot widget - Data visualization charts */
#define ENABLE_PLOT 1

/** Progress bar widget - Progress indication */
#define ENABLE_PROGRESSBAR 1

/** Radio button widget - Exclusive selection from multiple options */
#define ENABLE_RADIOBUTTON 1

/** Slider widget - Range selection input */
#define ENABLE_SLIDER 1

/** Tabs widget - Tabbed interface container */
#define ENABLE_TABS 1

/** Textbox widget - Text input field */
#define ENABLE_TEXTBOX 1

/** Container widget - Generic widget container */
#define ENABLE_CONTAINER 1

/** AppBar widget - Application title bar */
#define ENABLE_APPBAR 1

/** Switch widget - Toggle switch control */
#define ENABLE_SWITCH 1

/** WebView widget - Embedded web content (requires additional dependencies) */
#define ENABLE_WEBVIEW 1

/** File dialog widget - Native file open/save dialogs */
#define ENABLE_FDIALOG 1



/*******************************************************************************
 *                           ESP32 SPECIFIC CONFIGURATION                      *
 * 
 * These settings are specific to ESP32 deployments with TFT displays
 ******************************************************************************/

/** Enable ESP32 TFT_eSPI display support */
#define TFT_ESPI_ENABLED ESP32

/** Screen rotation for TFT displays (0-3) */
#define TFT_SCREEN_ROTATION 3

/** Enable virtual keyboard for touch input (ESP32 only) */
#define ENABLE_VIRTUAL_KEYBOARD 0


/*******************************************************************************
 *                            PERFORMANCE NOTES                                *
 * 
 * - Disable unused widgets to reduce memory consumption and improve startup time
 * - In production (PROJECT_BRANCH=0), consider disabling ENABLE_DEBUG_OVERLAY
 * - Adjust resource limits based on your application's needs to optimize memory
 * - ESP32 builds may need lower limits due to constrained memory resources
 ******************************************************************************/

#endif /* USER_CONFIG_H */