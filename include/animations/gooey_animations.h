#ifndef GOOEY_ANIMATIONS_H
#define GOOEY_ANIMATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

void GooeyAnimation_TranslateX(void *widget, int x_start, int x_end, float speed);
void GooeyAnimation_TranslateY(void *widget, int y_start, int y_end, float speed);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_ANIMATIONS_H */
