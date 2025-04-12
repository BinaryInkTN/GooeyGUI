#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Gooey/gooey.h>
#include "glps_thread.h"

#ifdef _WIN32
#include <windows.h>
#define CLEAR "cls"
#define APACHE_SERVICE "Apache2.4"
#define MYSQL_SERVICE "MySQL80"
#define OPEN_CMD(cmd) system(cmd)
#define OPEN_APACHE_CONF "notepad C:\\Apache24\\conf\\httpd.conf"
#define OPEN_MYSQL_CONF "notepad C:\\ProgramData\\MySQL\\MySQL Server 8.0\\my.ini"
#define OPEN_PHPMYADMIN "start http://localhost/phpmyadmin"
#define PHPMYADMIN_PATH "C:\\phpmyadmin"
#else
#define CLEAR "clear"
#define APACHE_SERVICE "apache2"
#define MYSQL_SERVICE "mysql"
#define OPEN_CMD(cmd) system(cmd)
#define OPEN_APACHE_CONF "x-www-browser /etc/apache2/apache2.conf"
#define OPEN_MYSQL_CONF "x-www-browser /etc/mysql/my.cnf"
#define OPEN_PHPMYADMIN "x-www-browser http://localhost/phpmyadmin"
#define PHPMYADMIN_PATH "/usr/share/phpmyadmin"
#endif

// Mutex and condition variables for thread synchronization
gthread_mutex_t service_mutex;
gthread_cond_t service_cond;

typedef enum
{
    SERVICE_STATUS_UNKNOWN,
    SERVICE_STATUS_RUNNING,
    SERVICE_STATUS_STOPPED
} service_status_t;

service_status_t apache_status = SERVICE_STATUS_UNKNOWN;
service_status_t mysql_status = SERVICE_STATUS_UNKNOWN;

// Declare UI elements globally
GooeyButton *start_all_btn, *execute_btn;
GooeyDropdown *services_dropdown, *config_dropdown;
GooeyImage *apache_status_img, *mysql_status_img, *control_logo;

// Service options
const char *service_options[] = {
    "Start/Stop Apache",
    "Start/Stop MySQL"};
int num_service_options = sizeof(service_options) / sizeof(service_options[0]);

// Configuration options
const char *config_options[] = {
    "Apache Config",
    "MySQL Config",
    "phpMyAdmin"};
int num_config_options = sizeof(config_options) / sizeof(config_options[0]);

// Function to clear the screen
void clear_screen()
{
    system(CLEAR);
}

int check_php_installed()
{
#ifdef _WIN32
    return system("where php >nul 2>&1") == 0;
#else
    return system("which php > /dev/null 2>&1") == 0;
#endif
}

int check_phpmyadmin_installed()
{
#ifdef _WIN32
    if (system("if exist \"C:\\phpmyadmin\\index.php\" exit 0") == 0)
        return 1;
    if (system("if exist \"C:\\xampp\\phpMyAdmin\\index.php\" exit 0") == 0)
        return 1;
    if (system("if exist \"C:\\wamp\\phpmyadmin\\index.php\" exit 0") == 0)
        return 1;
    return 0;
#else
    if (system("test -f /usr/share/phpmyadmin/index.php") == 0)
        return 1;
    if (system("test -f /var/www/html/phpmyadmin/index.php") == 0)
        return 1;
    if (system("test -f /usr/share/phpMyAdmin/index.php") == 0)
        return 1;
    return 0;
#endif
}

int check_service_installed(const char *service)
{
#ifdef _WIN32
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sc query %s >nul 2>&1", service);
    return system(cmd) == 0;
#else
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "systemctl list-unit-files | grep -q ^%s", service);
    return system(cmd) == 0;
#endif
}

void set_service_status(const char *service, service_status_t status)
{
    glps_thread_mutex_lock(&service_mutex);

    if (strcmp(service, APACHE_SERVICE) == 0)
    {
        apache_status = status;
        // Update Apache status image
        if (status == SERVICE_STATUS_RUNNING)
        {
            GooeyImage_SetImage(apache_status_img, "apache_started.png");
        }
        else
        {
            GooeyImage_SetImage(apache_status_img, "apache_stopped.png");
        }
    }
    else if (strcmp(service, MYSQL_SERVICE) == 0)
    {
        mysql_status = status;
        // Update MySQL status image
        if (status == SERVICE_STATUS_RUNNING)
        {
            GooeyImage_SetImage(mysql_status_img, "mysql_started.png");
        }
        else
        {
            GooeyImage_SetImage(mysql_status_img, "mysql_stopped.png");
        }
    }

    glps_thread_cond_signal(&service_cond);
    glps_thread_mutex_unlock(&service_mutex);
}

service_status_t get_service_status(const char *service)
{
    glps_thread_mutex_lock(&service_mutex);

    service_status_t status = SERVICE_STATUS_UNKNOWN;
    if (strcmp(service, APACHE_SERVICE) == 0)
        status = apache_status;
    else if (strcmp(service, MYSQL_SERVICE) == 0)
        status = mysql_status;

    glps_thread_mutex_unlock(&service_mutex);
    return status;
}

void *check_service_thread(void *arg)
{
    const char *service = (const char *)arg;
    printf("[*] Checking service: %s\n", service);

    service_status_t status = SERVICE_STATUS_STOPPED;
    if (check_service_installed(service) && is_service_running(service))
    {
        status = SERVICE_STATUS_RUNNING;
    }

    set_service_status(service, status);
    return NULL;
}

void install_service(const char *service)
{
#ifdef _WIN32
    printf("[!] Please install %s manually on Windows.\n", service);
#else
    char cmd[256];
    if (strcmp(service, APACHE_SERVICE) == 0)
        snprintf(cmd, sizeof(cmd), "sudo apt install -y apache2");
    else if (strcmp(service, MYSQL_SERVICE) == 0)
        snprintf(cmd, sizeof(cmd), "sudo apt install -y mysql-server");
    else if (strcmp(service, "php") == 0)
        snprintf(cmd, sizeof(cmd), "sudo apt install -y php libapache2-mod-php");
    else if (strcmp(service, "phpmyadmin") == 0)
        snprintf(cmd, sizeof(cmd), "sudo apt install -y phpmyadmin");

    printf("[*] Installing %s...\n", service);
    system(cmd);
#endif
}

void start_service(const char *service)
{
    char cmd[256];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "net start %s", service);
#else
    snprintf(cmd, sizeof(cmd), "sudo systemctl start %s", service);
#endif
    system(cmd);
    set_service_status(service, SERVICE_STATUS_RUNNING);
}

void stop_service(const char *service)
{
    char cmd[256];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "net stop %s", service);
#else
    snprintf(cmd, sizeof(cmd), "sudo systemctl stop %s", service);
#endif
    system(cmd);
    set_service_status(service, SERVICE_STATUS_STOPPED);
}

int is_service_running(const char *service)
{
    char cmd[256];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "sc query %s | findstr /C:\"RUNNING\" >nul 2>&1", service);
#else
    snprintf(cmd, sizeof(cmd), "systemctl is-active --quiet %s", service);
#endif
    return system(cmd) == 0;
}

void update_button_labels()
{
    if (apache_status == SERVICE_STATUS_RUNNING || mysql_status == SERVICE_STATUS_RUNNING)
    {
        GooeyButton_SetText(start_all_btn, "Stop All Services");
    }
    else
    {
        GooeyButton_SetText(start_all_btn, "Start All Services");
    }
}

void toggle_apache()
{
    if (apache_status == SERVICE_STATUS_RUNNING)
    {
        stop_service(APACHE_SERVICE);
    }
    else
    {
        start_service(APACHE_SERVICE);
    }
    update_button_labels();
}

void toggle_mysql()
{
    if (mysql_status == SERVICE_STATUS_RUNNING)
    {
        stop_service(MYSQL_SERVICE);
    }
    else
    {
        start_service(MYSQL_SERVICE);
    }
    update_button_labels();
}

void toggle_all()
{
    if (apache_status == SERVICE_STATUS_RUNNING || mysql_status == SERVICE_STATUS_RUNNING)
    {
        if (apache_status == SERVICE_STATUS_RUNNING)
            stop_service(APACHE_SERVICE);
        if (mysql_status == SERVICE_STATUS_RUNNING)
            stop_service(MYSQL_SERVICE);
    }
    else
    {
        start_service(APACHE_SERVICE);
        start_service(MYSQL_SERVICE);
    }
}

void install_phpmyadmin()
{
#ifdef _WIN32
    printf("[!] Please install phpMyAdmin manually on Windows.\n");
    system("start https://www.phpmyadmin.net/downloads/");
#else
    printf("[*] Installing phpMyAdmin...\n");
    system("sudo apt-get update && sudo apt-get install -y phpmyadmin");
#endif
}

void open_phpmyadmin()
{
    if (!check_phpmyadmin_installed())
    {
        printf("[!] phpMyAdmin not installed. Install now? (y/n): ");
        char response;
        scanf(" %c", &response);
        if (response == 'y' || response == 'Y')
        {
            install_phpmyadmin();
        }
        else
        {
            return;
        }
    }
    system(OPEN_PHPMYADMIN);
}

void open_apache_conf()
{
    system(OPEN_APACHE_CONF);
}

void open_mysql_conf()
{
    system(OPEN_MYSQL_CONF);
}

int selected_service_index = 0;
int selected_config_index = 0;

void services_dropdown_callback(int selected_index)
{
    selected_service_index = selected_index;
}

void config_dropdown_callback(int selected_index)
{
    selected_config_index = selected_index;
}

void execute_service_command()
{
    switch (selected_service_index)
    {
    case 0: // Apache
        toggle_apache();
        break;
    case 1: // MySQL
        toggle_mysql();
        break;
    }
}

void execute_config_command()
{
    switch (selected_config_index)
    {
    case 0: // Apache Config
        open_apache_conf();
        break;
    case 1: // MySQL Config
        open_mysql_conf();
        break;
    case 2: // phpMyAdmin
        open_phpmyadmin();
        break;
    }
}

int main()
{
    Gooey_Init();

    // Initialize mutex and condition variables
    glps_thread_mutex_init(&service_mutex, NULL);
    glps_thread_cond_init(&service_cond, NULL);

    // Create smaller window (300x300)
    GooeyWindow *win = GooeyWindow_Create("PocketU Server Control", 300, 280, true);
    GooeyWindow_SetContinuousRedraw(win);

    GooeyTabs *tabs = GooeyTabs_Create(0, 0, 300, 280);

    // Control Tab
    GooeyLayout *control_layout = GooeyLayout_Create(LAYOUT_VERTICAL, 50, 20, 200, 260);
    control_logo = GooeyImage_Create("logo.png", 50, 10, 64, 64, NULL);

    // Status images (initially stopped)
    apache_status_img = GooeyImage_Create("./apache_stopped.png", 30, 80, 24, 24, NULL);
    mysql_status_img = GooeyImage_Create("./mysql_stopped.png", 80, 80, 24, 24, NULL);

    start_all_btn = GooeyButton_Create("Start All Services", 30, 120, 120, 30, toggle_all);

    GooeyLayout_AddChild(control_layout, control_logo);
    GooeyLayout *icon_layout = GooeyLayout_Create(LAYOUT_HORIZONTAL, 40, 0, 40, 40);
    GooeyLayout_AddChild(icon_layout, apache_status_img);
    GooeyLayout_AddChild(icon_layout, mysql_status_img);
    GooeyLayout_AddChild(control_layout, start_all_btn);

    GooeyLayout_AddChild(control_layout, icon_layout);

    GooeyLayout_Build(control_layout);

    // Services Tab
    GooeyLayout *services_layout = GooeyLayout_Create(LAYOUT_VERTICAL, 30, 20, 240, 260);
    services_dropdown = GooeyDropdown_Create(30, 40, 150, 30, service_options, num_service_options, services_dropdown_callback);
    execute_btn = GooeyButton_Create("Execute", 190, 40, 80, 30, execute_service_command);

    GooeyLayout_AddChild(services_layout, services_dropdown);
    GooeyLayout_AddChild(services_layout, execute_btn);
    GooeyLayout_Build(services_layout);

    // Config Tab
    GooeyLayout *config_layout = GooeyLayout_Create(LAYOUT_VERTICAL, 30, 20, 240, 260);
    config_dropdown = GooeyDropdown_Create(30, 40, 150, 30, config_options, num_config_options, config_dropdown_callback);
    GooeyButton *config_execute_btn = GooeyButton_Create("Execute", 190, 40, 80, 30, execute_config_command);

    GooeyLayout_AddChild(config_layout, config_dropdown);
    GooeyLayout_AddChild(config_layout, config_execute_btn);
    GooeyLayout_Build(config_layout);

    // Setup tabs
    GooeyTabs_InsertTab(tabs, "Control");
    GooeyTabs_InsertTab(tabs, "Services");
    GooeyTabs_InsertTab(tabs, "Config");

    // Add widgets to tabs
    GooeyTabs_AddWidget(tabs, 0, control_logo);
    GooeyTabs_AddWidget(tabs, 0, apache_status_img);
    GooeyTabs_AddWidget(tabs, 0, mysql_status_img);
    GooeyTabs_AddWidget(tabs, 0, start_all_btn);

    GooeyTabs_AddWidget(tabs, 1, services_dropdown);
    GooeyTabs_AddWidget(tabs, 1, execute_btn);

    GooeyTabs_AddWidget(tabs, 2, config_dropdown);
    GooeyTabs_AddWidget(tabs, 2, config_execute_btn);

    // Register all widgets
    GooeyWindow_RegisterWidget(win, tabs);
    GooeyWindow_RegisterWidget(win, control_logo);
    GooeyWindow_RegisterWidget(win, apache_status_img);
    GooeyWindow_RegisterWidget(win, mysql_status_img);
    GooeyWindow_RegisterWidget(win, start_all_btn);
    GooeyWindow_RegisterWidget(win, services_dropdown);
    GooeyWindow_RegisterWidget(win, execute_btn);
    GooeyWindow_RegisterWidget(win, config_dropdown);
    GooeyWindow_RegisterWidget(win, config_execute_btn);

    // Check services status
    gthread_t apache_thread, mysql_thread;
    glps_thread_create(&apache_thread, NULL, check_service_thread, (void *)APACHE_SERVICE);
    glps_thread_create(&mysql_thread, NULL, check_service_thread, (void *)MYSQL_SERVICE);
    glps_thread_join(apache_thread, NULL);
    glps_thread_join(mysql_thread, NULL);

    GooeyWindow_Run(1, win);

    // Cleanup
    glps_thread_mutex_destroy(&service_mutex);
    glps_thread_cond_destroy(&service_cond);
    GooeyWindow_Cleanup(1, win);
    return 0;
}