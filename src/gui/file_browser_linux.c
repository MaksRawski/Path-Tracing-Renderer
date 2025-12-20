#include "gui/file_browser.h"
#include <gtk/gtk.h>

// TODO: instead use this? seems like an async thing
// https://developer.gnome.org/documentation/tutorials/beginners/components/file_dialog.html 
char *GuiFileBrowser_open_linux(void) {
  // 1. Create the File Chooser Dialog
  // We define the title, parent (NULL), action (Open), and the buttons.
  GtkWidget *dialog = gtk_file_chooser_dialog_new(
      "Select a glTF file", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
      GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

  // 2. Create and Apply the Filter
  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "glTF (*.glb, *.gltf)");
  gtk_file_filter_add_pattern(filter, "*.glb");
  /* gtk_file_filter_add_pattern(filter, "*.gltf"); */

  // Add the filter to the dialog
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  // Optional: Add a "All Files" filter so the user can switch views
  GtkFileFilter *all_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(all_filter, "All Files");
  gtk_file_filter_add_pattern(all_filter, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

  // 3. Run the Dialog and Check Response
  // This creates a blocking loop that waits for user input
  int res = gtk_dialog_run(GTK_DIALOG(dialog));

  char *filename = NULL;
  if (res == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    filename = gtk_file_chooser_get_filename(chooser);
  } else {
    printf("Dialog cancelled.\n");
  }

  // 4. Cleanup
  // Destroys the widget and frees associated memory
  gtk_widget_destroy(dialog);

  // Process any pending events (essential for proper cleanup of the visual
  // window)
  while (gtk_events_pending())
    gtk_main_iteration();

  return filename;
}
