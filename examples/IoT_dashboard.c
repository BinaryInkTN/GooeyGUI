#include "gooey.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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

#define ADDRESS ""
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
        if (storage_meter)
        {
            GooeyMeter_Update(storage_meter, (long)  100 - ((float) value / 47) * 100);
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

    // declare values for ssl options, here we use only the ones necessary for TLS, but you can optionally define a lot more
    // look here for an example: https://github.com/eclipse/paho.mqtt.c/blob/master/src/samples/paho_c_sub.c
    ssl_opts.verify = 1;
    ssl_opts.CApath = NULL;
    ssl_opts.keyStore = NULL;
    ssl_opts.trustStore = NULL;
    ssl_opts.privateKey = NULL;
    ssl_opts.privateKeyPassword = NULL;
    ssl_opts.enabledCipherSuites = NULL;

    // use TLS for a secure connection, "ssl_opts" includes TLS
    conn_opts.ssl = &ssl_opts;
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    // use your credentials that you created with the cluster
    conn_opts.username = "dashboard";
    conn_opts.password = "";
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

void initialize_dashboard()
{
    srand(time(NULL));

    if (setup_mqtt_connection() != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize MQTT connection\n");
        return;
    }
    dark_theme = GooeyTheme_LoadFromFile("dark.json");

    dashboard = GooeyWindow_Create("Smart Lighting Dashboard", 800, 450, true);

    for (int i = 0; i < 24; i++)
    {
        hours[i] = (float)i;
    }

    const int left_col = 20;
    const int meter_size = 120;

    light_meter = GooeyMeter_Create(left_col, 60, meter_size, meter_size, 80, "light", "sun.png");
    toggle_light = GooeyButton_Create("Toggle Light", left_col, 160, meter_size, 30, toggle_light_callback);
    storage_meter = GooeyMeter_Create(left_col, 250, meter_size, meter_size, 30, "storage", "cube.png");

    const int middle_col = 180;
    const int plot_width = 300;
    const int plot_height = 150;

    light_plot = GooeyPlot_Create(GOOEY_PLOT_LINE, &plot_data,
                                  middle_col, 60, plot_width, plot_height);

    alert_list = GooeyList_Create(middle_col, 240, plot_width, 180, NULL);

    GooeyList_AddItem(alert_list, "Light Status", light_on ? "ON" : "OFF");
    GooeyList_AddItem(alert_list, "storage Level", "35% full");
    GooeyList_AddItem(alert_list, "Last Check", "2 mins ago");
    GooeyList_AddItem(alert_list, "System Status", "All normal");

    GooeyCanvas *canvas = GooeyCanvas_Create(0, 0, 800, 450);
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 800, 40, dashboard->active_theme->primary, true);

    GooeyLabel *title = GooeyLabel_Create("Smart Lighting", 0.3f, 25, 25);

    theme_toggle = GooeyButton_Create("Dark Mode OFF", 650, 5, 100, 30, toggle_dark_mode);

    // Light slider

    GooeyCanvas_DrawRectangle(canvas, 520, 64, 250, 350, dashboard->active_theme->widget_base, false);
    GooeyLabel *light_slider_label = GooeyLabel_Create("Adjust light level:", 0.27f, 530, 90);
    GooeySlider *light_slider = GooeySlider_Create(550, 120, 150, 0, 100, true, light_slider_callback);

    GooeyWindow_RegisterWidget(dashboard, title);
    GooeyWindow_RegisterWidget(dashboard, theme_toggle);
    GooeyWindow_RegisterWidget(dashboard, light_slider_label);
    GooeyWindow_RegisterWidget(dashboard, light_slider);
    GooeyWindow_RegisterWidget(dashboard, canvas);
    GooeyWindow_RegisterWidget(dashboard, light_meter);
    GooeyWindow_RegisterWidget(dashboard, storage_meter);
    GooeyWindow_RegisterWidget(dashboard, light_plot);
    GooeyWindow_RegisterWidget(dashboard, alert_list);

    glps_thread_create(&thread_mqtt, NULL, mqtt_subscribe_thread, NULL);
}

int main()
{
    Gooey_Init();
    initialize_dashboard();

    if (dashboard)
    {
        // GooeyWindow_EnableDebugOverlay(dashboard, true);
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