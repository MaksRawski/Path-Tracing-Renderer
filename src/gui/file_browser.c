#include "gui/file_browser.h"

#ifdef _WIN32
#include "windows.h"
// https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-common-dialog-boxes#opening-a-file
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
  OPENFILENAME ofn;
  char szFile[260];

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = szFile;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "All\0*.*\0glTF\0*.gltf;*.glb\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn) == TRUE) {
    snprintf(out_path, capacity, "%s", ofn.lpstrFile);
    return true;
  } else {
    return false;
  }
}
#elif defined(__linux__)
#include <gtk/gtk.h>
bool GuiFileBrowser_open(char out_path[], size_t capacity) {
  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Select a glTF file", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
      GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "glTF (*.glb, *.gltf)");
  gtk_file_filter_add_pattern(filter, "*.glb");
  gtk_file_filter_add_pattern(filter, "*.gltf");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  GtkFileFilter *all_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(all_filter, "All Files");
  gtk_file_filter_add_pattern(all_filter, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

  bool ok = false;
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    char *filename = gtk_file_chooser_get_filename(chooser);
    if (filename != NULL) {
      snprintf(out_path, capacity, "%s", filename);
      ok = true;
    }
  }
  gtk_widget_destroy(dialog);

  while (gtk_events_pending())
    gtk_main_iteration();

  return ok;
}
#endif
