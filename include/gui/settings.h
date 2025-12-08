#ifndef GUI_SETTINGS_H_
#define GUI_SETTINGS_H_
#include "gui.h"
#include "gui/parameters.h"
#include <stdbool.h>

void GuiSettings_draw(Gui *gui, GuiParameters *params,
                      WindowEventsData *events);

#endif // GUI_SETTINGS_H_
