#ifndef __LIBVLC_GTKGLAREA__
#define __LIBVLC_GTKGLAREA__

#define __LIBVLC_GTKGLAREA__

#include <gtk/gtk.h>
#include <vlc/vlc.h>

void libvlc_media_player_set_gtkglarea(
    libvlc_media_player_t *media_player,
    GtkGLArea *area);

#endif /* __LIBVLC_GTKGLAREA__ */
