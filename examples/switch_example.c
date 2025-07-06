
#include <gooey.h> // Only include if it exists and is needed

// Callback functions
void dropdown_callback(int selected_index) {
  printf("Dropdown selection changed to index: %d\n", selected_index);
}
bool state = true;
GooeyTabs *mainTabs;
void button_callback() { printf("test\n"); }
void switch_clbk(bool s) { printf(s ? "Btn ON\n" : "BTN OFF\n"); }

void setup() {
  Gooey_Init();
  GooeyWindow *win =
      GooeyWindow_Create("Advanced Tabs Example", 800, 600, true);
  //  GooeyAppbar_Set(win, "test");
  // GooeyTheme *dark_mode = GooeyTheme_LoadFromString(
  //     "{\"base\": \"0x0E0E0E\",\"widget_base\": \"0x1C1C1C\",\"neutral\": "
  //     "\"0xFFFFFF\",\"primary\": \"0x3d99f5\",\"danger\": "
  //     "\"0xCF6679\",\"info\": \"0x03DAC6\",\"success\": \"0x4CAF50\"}");
  // GooeyWindow_SetTheme(win, dark_mode);

  GooeySwitch *gswitch = GooeySwitch_Create(50, 50, false, false, switch_clbk);
  GooeyWindow_RegisterWidget(win, gswitch);

  GooeyWindow_Run(1, win);
  GooeyWindow_Cleanup(1, win);
}
int main() {
  setup();

  return 0;
}
