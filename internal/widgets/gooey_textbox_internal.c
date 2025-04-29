#include "gooey_textbox_internal.h"
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include <ctype.h>

void GooeyTextbox_Draw(GooeyWindow *win) {

  for (size_t index = 0; index < win->textboxes_count; ++index) {

    GooeyTextbox *textbox = (GooeyTextbox *)win->textboxes[index];
    if (!textbox->core.is_visible)
      continue;
    active_backend->FillRectangle(textbox->core.x, textbox->core.y,
                                  textbox->core.width, textbox->core.height,
                                  win->active_theme->base, win->creation_id,
                                  false, 0.0f);

    active_backend->DrawRectangle(textbox->core.x, textbox->core.y,
                                  textbox->core.width, textbox->core.height,
                                  textbox->focused ? win->active_theme->primary
                                                   : win->active_theme->neutral,
                                  1.0f, win->creation_id, false, 0.0f);

    int text_x = textbox->core.x + 5;
    int text_y = textbox->core.y + (textbox->core.height / 2) + 5;

    int max_text_width = textbox->core.width - 10;
    size_t len = strlen(textbox->text);
    size_t start_index = textbox->scroll_offset;

    while (start_index < len &&
           active_backend->GetTextWidth(textbox->text + start_index,
                                        len - start_index) > max_text_width) {
      start_index++;
    }

    char display_text[256];
    strncpy(display_text, textbox->text + start_index,
            sizeof(display_text) - 1);
    display_text[sizeof(display_text) - 1] = '\0';

    active_backend->DrawText(text_x, text_y, display_text,
                             win->active_theme->neutral, 0.27f,
                             win->creation_id);

    if (textbox->focused) {
      const int cursor_x = text_x + active_backend->GetTextWidth(
                                        display_text, strlen(display_text) +
                                                          textbox->cursor_pos);

      active_backend->DrawLine(cursor_x, textbox->core.y + 5, cursor_x,
                               textbox->core.y + textbox->core.height - 5,
                               win->active_theme->neutral, win->creation_id);
    } else {

      if (strcmp(textbox->placeholder, "") != 0 && strlen(textbox->text) == 0)
        active_backend->DrawText(text_x, text_y, textbox->placeholder,
                                 win->active_theme->neutral, 0.27f,
                                 win->creation_id);
    }
  }
}

void GooeyTextbox_HandleKeyPress(GooeyWindow *win, void *key_event) {
  GooeyEvent *event = (GooeyEvent *)key_event;
  static bool is_capslock_on = false;
  static int ascii_offset = 'a' - 'A';
  printf("Key pressed\n");
  char *buf = event->key_press.value;

  for (size_t i = 0; i < win->textboxes_count; i++) {
    if (!win->textboxes[i]->focused)
      continue;

    size_t len = strlen(win->textboxes[i]->text);

    switch (event->key_press.keycode) {
    case 22:
      if (len > 0) {
        win->textboxes[i]->text[len - 1] = '\0';

        if (win->textboxes[i]->scroll_offset > 0) {
          win->textboxes[i]->scroll_offset--;
        }

        if (win->textboxes[i]->callback) {
          win->textboxes[i]->callback(win->textboxes[i]->text);
        }
      }
      break;
    case 36:
      win->textboxes[i]->focused = false;
      break;

    case 66:
      is_capslock_on = !is_capslock_on;
      break;

    case 65:
      strcat(win->textboxes[i]->text, " ");
      break;

    case 23:
      // TODO: Implement (TAB)
      break;
    case 113:
      // LEFT ARROW
      printf("%d %ld %d\n", win->textboxes[i]->cursor_pos, -len, win->textboxes[i]->cursor_pos > -len);
      if((int) win->textboxes[i]->cursor_pos > (int) -len) {
        win->textboxes[i]->cursor_pos--;
      }
      break;
    case 114:
      // RIGHT ARROW
      if((int) win->textboxes[i]->cursor_pos <= (int) len) {
        win->textboxes[i]->cursor_pos++;
      }
      break;

    default:
      break;
    }

    if (isprint(buf[0]) && len < sizeof(win->textboxes[i]->text) - 1) {
      char ch = buf[0];
      if (is_capslock_on && ch >= 'a' && ch <= 'z') {
        ch -= ascii_offset;
      }
      win->textboxes[i]->text[len] = ch;
      win->textboxes[i]->text[len + 1] = '\0';

      if (win->textboxes[i]->callback) {
        win->textboxes[i]->callback(win->textboxes[i]->text);
      }

      int text_width =
          active_backend->GetTextWidth(win->textboxes[i]->text, len + 1);
      int max_text_width = win->textboxes[i]->core.width - 10;

      if (text_width > max_text_width) {
        win->textboxes[i]->scroll_offset++;
      }
    }
  }

  //
}

bool GooeyTextbox_HandleClick(GooeyWindow *win, int x, int y) {
  for (size_t i = 0; i < win->textboxes_count; i++) {
    GooeyTextbox *textbox = win->textboxes[i];
    if (!textbox || !textbox->core.is_visible)
      continue;
    if (x >= textbox->core.x && x <= textbox->core.x + textbox->core.width &&
        y >= textbox->core.y && y <= textbox->core.y + textbox->core.height) {
      // Focus the clicked textbox and unfocus others
      textbox->focused = true;
      for (size_t j = 0; j < win->textboxes_count; j++) {
        if (j != i)
          win->textboxes[j]->focused = false;
      }
      return true;
    }
  }
  return false;
}
