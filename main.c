#include <gtk/gtk.h>
#include <vlc/vlc.h>

#if GTK_MAJOR_VERSION == 3
#include <gdk/gdkx.h>
#endif

#if GTK_MAJOR_VERSION == 4
#include <gdk/x11/gdkx.h>
#endif

void player_widget_on_realize(GtkWidget *widget, gpointer user_data)
{
#if GTK_MAJOR_VERSION == 3
  GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
  GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(toplevel));
  guint32 xid = (guint32)gdk_x11_window_get_xid(window);
#endif

#if GTK_MAJOR_VERSION == 4
  GtkNative *native = gtk_widget_get_native(widget);
  GdkSurface *surface = gtk_native_get_surface(native);
  guint32 xid = (guint32)gdk_x11_surface_get_xid(surface);
#endif

  libvlc_media_player_t *media_player = user_data;
  libvlc_media_player_set_xwindow(media_player, xid);
}

static void
activate(GtkApplication *app, gpointer user_data)
{
  // window
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

  // window's background-color
  const char *css = ".background { background-color: black; }";
  GtkCssProvider *cssProvider = gtk_css_provider_new();
  GtkStyleContext *context = gtk_widget_get_style_context(window);
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

  // media_player
  const char *uri = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
  libvlc_instance_t *vlc_inst = libvlc_new(0, NULL);
  libvlc_media_player_t *media_player = libvlc_media_player_new(vlc_inst);
  libvlc_media_t *media = libvlc_media_new_location(vlc_inst, uri);
  libvlc_media_player_set_media(media_player, media);
  libvlc_media_player_play(media_player);
  libvlc_media_release(media);

  // player_widget
  GtkWidget *player_widget = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(player_widget), "realize", G_CALLBACK(player_widget_on_realize), media_player);

#if GTK_MAJOR_VERSION == 3
  gtk_css_provider_load_from_data(cssProvider, css, -1, NULL);
  gtk_container_add(GTK_CONTAINER(window), player_widget);
  gtk_widget_show_all(GTK_WIDGET(window));
#endif

#if GTK_MAJOR_VERSION == 4
  gtk_css_provider_load_from_data(cssProvider, css, -1);
  gtk_window_set_child(GTK_WINDOW(window), player_widget);
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
