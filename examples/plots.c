#include <Gooey/gooey.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>  // For seeding the random number generator

#define DATA_COUNT 4 // Number of data points

pthread_mutex_t data_mutex;
int received_data_count = 0;

typedef struct
{
    GooeyPlotData *plot_data;
    GooeyPlot *plot;
} ThreadArgs;

/* Function to generate a random float between min and max */
float random_float(float min, float max) {
    return min + (float)rand() / (float)(RAND_MAX / (max - min));
}

/* Thread function to simulate receiving data */
void* data_receiver_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    GooeyPlotData* data = args->plot_data;

    float x_full[DATA_COUNT];
    float y_full[DATA_COUNT];

    // Generate random data points
    for (int i = 0; i < DATA_COUNT; i++) {
        x_full[i] = random_float(-10, 10);
        y_full[i] = random_float(-10, 10);
    }

    // Simulate receiving data points one by one
    for (int i = 0; i < DATA_COUNT; i++) {
        sleep(1); // Simulate delay in receiving data

        pthread_mutex_lock(&data_mutex);
        data->x_data[i] = x_full[i];
        data->y_data[i] = y_full[i];
        received_data_count++;
                GooeyPlot_Update(args->plot, data);

        pthread_mutex_unlock(&data_mutex);

        printf("[Server] Received new data point: (%.2f, %.2f)\n", x_full[i], y_full[i]);

        // Update the plot with the new data
    }

    return NULL;
}

int main()
{
   // srand(time(NULL));  // Seed the random number generator

    // Initialize the Gooey library
    Gooey_Init(GLPS);
    GooeyWindow win = GooeyWindow_Create("Scattered Data Plot", 800, 600, true);

    // Initialize the mutex
    pthread_mutex_init(&data_mutex, NULL);

    // Initialize plot data
    GooeyPlotData data;
    char plot_title[] = "Scattered Data Plot";
    data.title = plot_title;
    data.data_count = DATA_COUNT;
    data.x_step = 2;
    data.y_step = 2;
    const char* labels[DATA_COUNT] = {"pt1", "pt2", "pt3", "pt4"};
    data.bar_labels = labels;

    // Partial data for initial plot
    float x_partial[DATA_COUNT] = {1, 2, 3, 4};
    float y_partial[DATA_COUNT] = {1, 2, 3, 4};

    data.x_data = x_partial;
    data.y_data = y_partial;

    // Add the plot to the window
    GooeyPlot *plot = GooeyPlot_Add(&win, GOOEY_PLOT_LINE, &data, 20, 20, 800, 400);

    // Create a thread to simulate receiving data
    pthread_t thread_id;
    ThreadArgs args = {&data, plot};
    //pthread_create(&thread_id, NULL, data_receiver_thread, &args);

    // Run the window
    GooeyWindow_Run(1, &win);

    // Wait for the thread to finish
   // pthread_join(thread_id, NULL);

    // Clean up
    //pthread_mutex_destroy(&data_mutex);
    GooeyWindow_Cleanup(1, &win);

    return 0;
}