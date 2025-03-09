/*
 Copyright (c) 2024 Yassine Ahmed Ali

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

#ifndef GOOEY_H
#define GOOEY_H

#include "common/gooey_common.h"
#include "widgets/gooey_tabs.h"
#include "widgets/gooey_drop_surface.h"
#include "core/gooey_backend.h"
#include "core/gooey_window.h"
#include "widgets/gooey_image.h"
#include "widgets/gooey_button.h"
#include "widgets/gooey_canvas.h"
#include "widgets/gooey_checkbox.h"
#include "widgets/gooey_dropdown.h"
#include "widgets/gooey_label.h"
#include "widgets/gooey_layout.h"
#include "widgets/gooey_list.h"
#include "widgets/gooey_menu.h"
#include "widgets/gooey_messagebox.h"
#include "widgets/gooey_radiobutton.h"
#include "widgets/gooey_slider.h"
#include "widgets/gooey_textbox.h"
#include "widgets/gooey_plot.h"
#include "signals/gooey_signals.h"

/**
 * @brief Initializes the Gooey system with the specified backend.
 *
 * This implementation only supports GLPS (OpenGL) backend for now.
 * @param backend you want to use.
 * @return 0 on success, non-zero on failure.
 */
int Gooey_Init(GooeyBackends backend);

#endif
