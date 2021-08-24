#include <gtk/gtk.h>
#include <vlc/vlc.h>
#include "libvlc_gtkglarea.h"

static void
activate(GtkApplication *app, gpointer user_data)
{
  // window
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

  // player_widget
  GtkWidget *player_widget = gtk_gl_area_new();

#if GTK_MAJOR_VERSION == 3
  gtk_container_add(GTK_CONTAINER(window), player_widget);
#endif

#if GTK_MAJOR_VERSION == 4
  gtk_window_set_child(GTK_WINDOW(window), player_widget);
#endif

  // media_player
  const char *uri = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
  libvlc_instance_t *vlc_inst = libvlc_new(0, NULL);
  libvlc_media_player_t *media_player = libvlc_media_player_new(vlc_inst);
  libvlc_media_t *media = libvlc_media_new_location(vlc_inst, uri);
  libvlc_media_player_set_media(media_player, media);
  libvlc_media_player_set_gtkglarea(media_player, GTK_GL_AREA(player_widget));
  libvlc_media_player_play(media_player);
  libvlc_media_release(media);

#if GTK_MAJOR_VERSION == 3
  gtk_widget_show_all(GTK_WIDGET(window));
#endif

#if GTK_MAJOR_VERSION == 4
  gtk_window_present(GTK_WINDOW(window));
#endif
}

int main(int argc, char **argv)
{
  GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
