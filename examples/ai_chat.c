#include <gooey.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>


// Structure to store curl response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for curl to write response
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

// Function to call Gemini API
char* call_gemini_api(const char* api_key, const char* prompt)
{
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=%s", api_key);
        
        // Create JSON with cjson
        cJSON *root = cJSON_CreateObject();
        cJSON *contents = cJSON_CreateArray();
        cJSON *content = cJSON_CreateObject();
        cJSON *parts = cJSON_CreateArray();
        cJSON *part = cJSON_CreateObject();
        
        cJSON_AddStringToObject(part, "text", prompt);
        cJSON_AddItemToArray(parts, part);
        cJSON_AddItemToObject(content, "parts", parts);
        cJSON_AddItemToArray(contents, content);
        cJSON_AddItemToObject(root, "contents", contents);
        
        char *json_data = cJSON_PrintUnformatted(root);
        
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        
        res = curl_easy_perform(curl);
        
        // Cleanup cJSON objects
        cJSON_Delete(root);
        free(json_data);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            chunk.memory = NULL;
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return chunk.memory;
}

// Function to parse Gemini response using cjson
char* parse_gemini_response(const char* response)
{
    cJSON *root = cJSON_Parse(response);
    if (!root) {
        fprintf(stderr, "Error parsing JSON response\n");
        return NULL;
    }
    
    // Navigate through the JSON structure
    cJSON *candidates = cJSON_GetObjectItem(root, "candidates");
    if (!candidates || !cJSON_IsArray(candidates)) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *candidate = cJSON_GetArrayItem(candidates, 0);
    if (!candidate) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *content = cJSON_GetObjectItem(candidate, "content");
    if (!content) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *parts = cJSON_GetObjectItem(content, "parts");
    if (!parts || !cJSON_IsArray(parts)) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *part = cJSON_GetArrayItem(parts, 0);
    if (!part) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *text = cJSON_GetObjectItem(part, "text");
    if (!text || !cJSON_IsString(text)) {
        cJSON_Delete(root);
        return NULL;
    }
    
    char *result = strdup(text->valuestring);
    cJSON_Delete(root);
    
    return result;
}

// Global variables for widget access
GooeyTextbox *textbox = NULL;
GooeyList *list = NULL;

// Button callback function
void send_button_callback()
{
    const char *user_input = GooeyTextbox_GetText(textbox);
    
    if(user_input && strlen(user_input) > 0) {
        // Add user message to chat
        GooeyList_AddItem(list, "You", user_input);
        
        // Your Gemini API key - store this securely in production!
        const char *api_key = "AIzaSyA9x3AdIBEEq7Gymy-JF03w6zrErkzOo1k";
        
        // Call Gemini API
        char *response = call_gemini_api(api_key, user_input);
        if(response) {
            char *gemini_response = parse_gemini_response(response);
            if(gemini_response) {
                // Add Gemini response to chat
                GooeyList_AddItem(list, "Gemini", gemini_response);
                free(gemini_response);
            } else {
                GooeyList_AddItem(list, "Gemini", "Error: Could not parse response");
            }
            free(response);
        } else {
            GooeyList_AddItem(list, "Gemini", "Error: API call failed");
        }
        
        // Clear input field
      //  GooeyTextbox_SetText(textbox, "");
    }
}

int main()
{
    Gooey_Init();
    
    GooeyTheme* dark_theme = GooeyTheme_LoadFromFile("dark.json");
    
    GooeyWindow *win = GooeyWindow_Create("Google Gemini", 410, 300, true);
    GooeyWindow_MakeResizable(win, false);
    GooeyWindow_SetTheme(win, dark_theme);
   // GooeyWebview* webview = GooeyWebview_Create(win, "https://www.example.com", 0, 0, 410, 300);

    // Create widgets with proper API calls
    list = GooeyList_Create(5, 5, 400, 250, NULL);
    textbox = GooeyTextBox_Create(5, 260, 300, 30, "Type your message here...", false, NULL);
    GooeyButton *sendButton = GooeyButton_Create("Send", 320, 263, 40, 30, send_button_callback);
    
    // Add welcome message
    GooeyList_AddItem(list, "Gemini", "Hello, how can I assist you today?");
   // GooeyWindow_RegisterWidget(win, webview);
    GooeyWindow_RegisterWidget(win, sendButton);
    GooeyWindow_RegisterWidget(win, list);
    GooeyWindow_RegisterWidget(win, textbox);
    
    GooeyWindow_Run(1, win);
    return 0;
}