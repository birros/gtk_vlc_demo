#include <gtk/gtk.h>
#include <vlc/vlc.h>
#include <gdk/x11/gdkx.h>

GtkWidget *player_widget;
GtkWidget *window;

libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;

void player_widget_on_realize(GtkWidget *widget, gpointer data) {
    printf("Hello\n");

    GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
    guint32 xid = (guint32) gdk_x11_surface_get_xid (surface);

    printf("xid: %d\n", xid);

    // libvlc_media_player_set_xwindow(media_player, GDK_WINDOW_XID(gtk_widget_get_window(widget)));

    libvlc_media_player_set_xwindow(media_player, xid);
}

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *button;

    printf("world\n");
  player_widget = gtk_drawing_area_new();
  g_signal_connect(G_OBJECT(player_widget), "realize", G_CALLBACK(player_widget_on_realize), NULL);


  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  gtk_window_set_child (GTK_WINDOW (window), player_widget);

//   button = gtk_button_new_with_label ("Hello World");
//   g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
//   gtk_window_set_child (GTK_WINDOW (window), button);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  vlc_inst = libvlc_new(0, NULL);
  media_player = libvlc_media_player_new(vlc_inst);

    const char* uri = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
  libvlc_media_t *media;
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);
    libvlc_media_player_play(media_player);
    libvlc_media_release(media);

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
