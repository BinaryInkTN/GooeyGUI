/*
 Copyright (c) 2025 Yassine Ahmed Ali

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

#include "gooey.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <GLPS/glps_audio_stream.h>

/**
 *
 * GooeyGUI
 *
 */

GooeyWindow *dashboard;
GooeyMeter *light_meter, *storage_meter;
GooeyPlot *light_plot;
GooeyButton *toggle_light;
GooeyButton *theme_toggle;
GooeyList *alert_list;
GooeyTheme *dark_theme;
glps_audio_stream *stream;
GooeyTabs *tabs;
bool dark_mode = false;
bool light_on = false;

float hours[24] = {
    0, 0, 0, 0, 0,
    10, 30, 70, 80, 85,
    90, 95, 100, 100, 100,
    90, 80, 70, 60, 50,
    30, 10, 0, 0};
float light_levels[24] = {0};

GooeyPlotData plot_data = {
    .x_data = hours,
    .y_data = light_levels,
    .data_count = 5,
    .x_step = 1.0f,
    .y_step = 1.0f,
    .title = "24h Light Levels"};

void toggle_light_callback()
{
    light_on = !light_on;

    if (alert_list)
    {
        GooeyList_UpdateItem(alert_list, 0, "Light Status", light_on ? "ON" : "OFF");
    }
}

/**
 *
 * MQTT
 *
 */

#include <MQTTClient.h>

#define ADDRESS "ssl://cf40b80b591347158d36698519a99fc5.s2.eu.hivemq.cloud:8883"
#define CLIENTID "dashboard"
#define TOPIC_LIGHT "topic_light"
#define TOPIC_STORAGE_LEVEL "topic_storage_level"
#define TOPIC_LIGHT_LEVEL "topic_light_level"
#define QOS 1
#define TIMEOUT 10000L

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

volatile MQTTClient_deliveryToken deliveredtoken;
gthread_t thread_mqtt;
bool mqtt_running = true;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char *)message->payload);

    if (strcmp(topicName, TOPIC_STORAGE_LEVEL) == 0)
    {
        long value = strtol((char *)message->payload, NULL, 10);
        printf("%ld \n", (long)((long)100 - ((float)value / 47) * 100));
        if (storage_meter)
        {
            printf("Recieved\n");

            GooeyMeter_Update(storage_meter, (long)((long)100 - ((float)value / 47) * 100));
            char storage_level[20];
            snprintf(storage_level, sizeof(storage_level), "%ld%% full!", value);
            GooeyList_UpdateItem(alert_list, 1, "storage Status", storage_level);
        }
    }
    else if (strcmp(topicName, TOPIC_LIGHT) == 0)
    {
        if (alert_list)
        {
            GooeyList_UpdateItem(alert_list, 0, "Light Status", (char *)message->payload);
        }
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    if (cause)
        printf("     cause: %s\n", cause);
}

int setup_mqtt_connection()
{
    int rc;
    const char *uri = ADDRESS;
    printf("Using server at %s\n", uri);

    if ((rc = MQTTClient_create(&client, uri, CLIENTID,
                                MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        return rc;
    }

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        MQTTClient_destroy(&client);
        return rc;
    }
    ssl_opts.enableServerCertAuth = 0;

    ssl_opts.verify = 1;
    ssl_opts.CApath = NULL;
    ssl_opts.keyStore = NULL;
    ssl_opts.trustStore = NULL;
    ssl_opts.privateKey = NULL;
    ssl_opts.privateKeyPassword = NULL;
    ssl_opts.enabledCipherSuites = NULL;

    conn_opts.ssl = &ssl_opts;
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;

    conn_opts.username = "dashboard";
    conn_opts.password = "YASSINE2002@**v";
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        MQTTClient_destroy(&client);
        return rc;
    }

    return MQTTCLIENT_SUCCESS;
}

void subscribe_to_topics()
{
    int rc;
    char *topics[] = {TOPIC_STORAGE_LEVEL, TOPIC_LIGHT};
    int qos[] = {QOS, QOS};
    int topic_count = sizeof(topics) / sizeof(topics[0]);

    if ((rc = MQTTClient_subscribeMany(client, topic_count, topics, qos)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to subscribe, return code %d\n", rc);
    }
    else
    {
        printf("Successfully subscribed to topics\n");
    }
}

void *mqtt_subscribe_thread(void *arg)
{
    subscribe_to_topics();

    while (mqtt_running)
    {
        sleep(1);
    }

    return NULL;
}

void mqtt_cleanup()
{
    mqtt_running = false;
    if (thread_mqtt)
    {
        glps_thread_join(thread_mqtt, NULL);
    }

    char *topics[] = {TOPIC_STORAGE_LEVEL, TOPIC_LIGHT};
    int topic_count = sizeof(topics) / sizeof(topics[0]);
    MQTTClient_unsubscribeMany(client, topic_count, topics);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}

void light_slider_callback(long slider_value)
{
    char payload[64];
    snprintf(payload, sizeof(payload), "LIGHT_LEVEL %ld", slider_value);
    printf("slider value at %ld \n", slider_value);
    MQTTClient_publish(client, TOPIC_LIGHT_LEVEL, strlen(payload), payload, QOS, false, NULL);
}

void toggle_dark_mode()
{
    dark_mode = !dark_mode;
    GooeyWindow_SetTheme(dashboard, dark_mode ? dark_theme : NULL);
    GooeyButton_SetText(theme_toggle, dark_mode ? "Dark Mode ON" : "Dark Mode OFF");
}

void play_audio()
{
    printf("called play \n");
    glps_audio_stream_resume(stream);
}

void pause_audio()
{
    glps_audio_stream_pause(stream);
}

void nav_overview()
{
    GooeyTabs_SetActiveTab(tabs, 0);
}

void nav_system()
{
    GooeyTabs_SetActiveTab(tabs, 1);
}

void nav_alerts()
{
    GooeyTabs_SetActiveTab(tabs, 2);
}

void initialize_dashboard()
{
    srand(time(NULL));

    if (setup_mqtt_connection() != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize MQTT connection\n");
        return;
    }

    dark_theme = GooeyTheme_LoadFromFile("dark.json");

    dashboard = GooeyWindow_Create("Smart Factory Dashboard", 1200, 700, true);
    GooeyWindow_SetTheme(dashboard, dark_theme);
    dark_theme->primary = 0x0062c4;

    GooeyButton *play_btn = GooeyButton_Create("Play", 740, 300, 100, 30, play_audio);
    GooeyButton *pause_btn = GooeyButton_Create("Pause", 860, 300, 100, 30, pause_audio);

    GooeyCanvas *canvas = GooeyCanvas_Create(0, 0, 1400, 60);
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 1400, 60, dashboard->active_theme->primary, true);
    GooeyCanvas_DrawLine(canvas, 0, 60, 1400, 60, dashboard->active_theme->neutral);
    // GooeyCanvas_DrawRectangle(canvas, 60, 1100, 1150, 60, dashboard->active_theme->widget_base, true);
    GooeyImage *logo_icon = GooeyImage_Create("logo_icon.png", 18, 17, 24, 24, NULL);

    GooeyLabel *title = GooeyLabel_Create("Smart Factory", 0.4f, 65, 35);
    theme_toggle = GooeyButton_Create("Dark Mode OFF", 1350, 10, 130, 40, toggle_dark_mode);
    GooeyImage *avatar = GooeyImage_Create("utilisateur.png", 1100, 10, 32, 32, NULL);
    GooeyCanvas *sidebar = GooeyCanvas_Create(0, 60, 250, 640);
    GooeyCanvas_DrawRectangle(sidebar, 0, 0, 60, 640, dashboard->active_theme->widget_base, true);

    GooeyButton *sidebar_button1 = GooeyButton_Create("Option 1", 10, 80, 230, 30, nav_overview);
    GooeyButton *sidebar_button2 = GooeyButton_Create("Option 2", 10, 130, 230, 30, nav_system);
    GooeyButton *sidebar_button3 = GooeyButton_Create("Option 3", 10, 180, 230, 30, nav_alerts);

    GooeyImage *dashboard_icon = GooeyImage_Create("dashboard_icon.png", 18, 80, 24, 24, nav_overview);
    GooeyImage *settings_icon = GooeyImage_Create("settings.png", 18, 130, 24, 24, nav_system);
    GooeyImage *alerts_icon = GooeyImage_Create("alerts_icon.png", 18, 180, 24, 24, nav_alerts);
    GooeyImage *help_icon = GooeyImage_Create("help_icon.png", 18, 650, 24, 24, NULL);

    GooeyWindow_RegisterWidget(dashboard, sidebar);
    GooeyWindow_RegisterWidget(dashboard, dashboard_icon);
    GooeyWindow_RegisterWidget(dashboard, settings_icon);
    GooeyWindow_RegisterWidget(dashboard, alerts_icon);
    GooeyWindow_RegisterWidget(dashboard, logo_icon);
    GooeyWindow_RegisterWidget(dashboard, help_icon);

    //  GooeyWindow_RegisterWidget(dashboard, sidebar_button1);
    //  GooeyWindow_RegisterWidget(dashboard, sidebar_button2);
    //  GooeyWindow_RegisterWidget(dashboard, sidebar_button3);

    tabs = GooeyTabs_Create(60, 60, 1150, 660);
    GooeyTabs_InsertTab(tabs, "Overview");
    GooeyTabs_InsertTab(tabs, "System");
    GooeyTabs_InsertTab(tabs, "Alerts");

    GooeyWindow_RegisterWidget(dashboard, tabs);
    GooeyLabel *dashboard_bc = GooeyLabel_Create("Home /", 0.28f, 77, 45);
    GooeyLabel *dashboard_title = GooeyLabel_Create("Dashboard", 0.6f, 77, 80);
    GooeyLabel *dashboard_desc = GooeyLabel_Create("This is the overview page, where you monitor your sensors.", 0.26f, 77, 105);

    light_meter = GooeyMeter_Create(80, 140, 240, 240, 80, "Light", "light_icon.png");
    storage_meter = GooeyMeter_Create(415, 140, 240, 240, 30, "Storage", "storage_icon.png");
    GooeyMeter *battery_meter = GooeyMeter_Create(750, 140, 240, 240, 60, "Battery", "battery_icon.png");

    GooeyLabel *temp_label = GooeyLabel_Create("Temperature", 0.25f, 100, 295);
    GooeyProgressBar *temp_bar = GooeyProgressBar_Create(100, 305, 500, 20, 25);

    GooeyLabel *humidity_label = GooeyLabel_Create("Humidity", 0.25f, 100, 355);
    GooeyProgressBar *humidity_bar = GooeyProgressBar_Create(100, 365, 500, 20, 40);

    GooeyLabel *network_label = GooeyLabel_Create("Network", 0.25f, 100, 425);
    GooeyProgressBar *network_bar = GooeyProgressBar_Create(100, 435, 500, 20, 90);

    GooeyLabel *light_slider_label = GooeyLabel_Create("Adjust Light Level:", 0.27f, 100, 490);
    GooeySlider *light_slider = GooeySlider_Create(100, 520, 500, 0, 100, true, light_slider_callback);

    toggle_light = GooeyButton_Create("Toggle Light", 100, 570, 220, 30, toggle_light_callback);
    GooeyButton *refresh_btn = GooeyButton_Create("Refresh", 380, 570, 220, 30, NULL);

    // GooeyTabs_AddWidget(tabs, 0, play_btn);
    //  GooeyTabs_AddWidget(tabs, 0, pause_btn);
    GooeyTabs_AddWidget(tabs, 0, light_meter);
    GooeyTabs_AddWidget(tabs, 0, storage_meter);
    GooeyTabs_AddWidget(tabs, 0, battery_meter);
    GooeyTabs_AddWidget(tabs, 0, dashboard_bc);

    GooeyTabs_AddWidget(tabs, 0, dashboard_title);
    GooeyTabs_AddWidget(tabs, 0, dashboard_desc);

    //  GooeyTabs_AddWidget(tabs, 0, temp_label);
    //  GooeyTabs_AddWidget(tabs, 0, temp_bar);
    //  GooeyTabs_AddWidget(tabs, 0, humidity_label);
    //   GooeyTabs_AddWidget(tabs, 0, humidity_bar);
    //   GooeyTabs_AddWidget(tabs, 0, network_label);
    //   GooeyTabs_AddWidget(tabs, 0, network_bar);
    //   GooeyTabs_AddWidget(tabs, 0, light_slider_label);
    GooeyTabs_AddWidget(tabs, 0, light_slider);
    GooeyTabs_AddWidget(tabs, 0, toggle_light);
    GooeyTabs_AddWidget(tabs, 0, refresh_btn);
    //  GooeyWindow_RegisterWidget(dashboard, play_btn);
    //  GooeyWindow_RegisterWidget(dashboard, pause_btn);
    GooeyWindow_RegisterWidget(dashboard, dashboard_bc);

    GooeyWindow_RegisterWidget(dashboard, dashboard_title);
    GooeyWindow_RegisterWidget(dashboard, dashboard_desc);

    GooeyWindow_RegisterWidget(dashboard, light_meter);
    GooeyWindow_RegisterWidget(dashboard, storage_meter);
    GooeyWindow_RegisterWidget(dashboard, battery_meter);
    // GooeyWindow_RegisterWidget(dashboard, temp_label);
    // GooeyWindow_RegisterWidget(dashboard, temp_bar);
    //  GooeyWindow_RegisterWidget(dashboard, humidity_label);
    // GooeyWindow_RegisterWidget(dashboard, humidity_bar);
    //  GooeyWindow_RegisterWidget(dashboard, network_label);
    // GooeyWindow_RegisterWidget(dashboard, network_bar);
    //  GooeyWindow_RegisterWidget(dashboard, light_slider_label);
    //  GooeyWindow_RegisterWidget(dashboard, light_slider);

    GooeyLabel *uptime_label = GooeyLabel_Create("Uptime: 3h 24m", 0.27f, 80, 80);
    GooeyLabel *battery_label = GooeyLabel_Create("Battery: 60%", 0.27f, 80, 120);
    GooeyLabel *net_label = GooeyLabel_Create("Network: Connected", 0.27f, 80, 160);
    GooeyLabel *data_label = GooeyLabel_Create("Data Usage: 12.4MB", 0.27f, 80, 200);
    GooeyLabel *load_label = GooeyLabel_Create("CPU Load: 75%", 0.27f, 80, 240);
    GooeyLabel *disk_label = GooeyLabel_Create("Disk Usage: 55%", 0.27f, 80, 280);

    GooeyTabs_AddWidget(tabs, 1, uptime_label);
    GooeyTabs_AddWidget(tabs, 1, battery_label);
    GooeyTabs_AddWidget(tabs, 1, net_label);
    GooeyTabs_AddWidget(tabs, 1, data_label);
    GooeyTabs_AddWidget(tabs, 1, load_label);
    GooeyTabs_AddWidget(tabs, 1, disk_label);

    GooeyWindow_RegisterWidget(dashboard, uptime_label);
    GooeyWindow_RegisterWidget(dashboard, battery_label);
    GooeyWindow_RegisterWidget(dashboard, net_label);
    GooeyWindow_RegisterWidget(dashboard, data_label);
    GooeyWindow_RegisterWidget(dashboard, load_label);
    GooeyWindow_RegisterWidget(dashboard, disk_label);

    alert_list = GooeyList_Create(10, 10, 1120, 580, NULL);
    GooeyList_AddItem(alert_list, "Light Status", light_on ? "ON" : "OFF");
    GooeyList_AddItem(alert_list, "Storage Level", "35% full");
    GooeyList_AddItem(alert_list, "Battery", "60%");
    GooeyList_AddItem(alert_list, "Temperature", "25°C");
    GooeyList_AddItem(alert_list, "Humidity", "40%");
    GooeyList_AddItem(alert_list, "Network", "Connected");
    GooeyList_AddItem(alert_list, "System Status", "All normal");

    GooeyTabs_AddWidget(tabs, 2, alert_list);
    GooeyWindow_RegisterWidget(dashboard, alert_list);
    GooeyWindow_RegisterWidget(dashboard, avatar);

    GooeyWindow_RegisterWidget(dashboard, canvas);
    GooeyWindow_RegisterWidget(dashboard, title);
    // GooeyWindow_RegisterWidget(dashboard, theme_toggle);
    //  GooeyWindow_RegisterWidget(dashboard, tabs);

    glps_thread_create(&thread_mqtt, NULL, mqtt_subscribe_thread, NULL);
}

int main()
{
    Gooey_Init();
    initialize_dashboard();
    // stream = glps_audio_stream_init("default", 40000, 44120, 2, 0, 1024 * 2);
    //  glps_audio_stream_play(stream, "test.mp3", 44120, 2, -1, 1024 * 2);
    //  glps_audio_stream_pause(stream);

    if (dashboard)
    {

        GooeyWindow_MakeResizable(dashboard, false);
        GooeyWindow_Run(1, dashboard);
    }

    mqtt_cleanup();

    if (dashboard)
    {
        GooeyWindow_Cleanup(1, dashboard);
    }

    return 0;
}