all: gtk4

.PHONY4: gtk4
gtk4: main.c
	gcc -o gtk_player main.c libvlc_gtkglarea.c `pkg-config --libs --cflags gtk4 gl libvlc`

.PHONY4: gtk3
gtk3: main.c
	gcc -o gtk_player main.c libvlc_gtkglarea.c `pkg-config --libs --cflags gtk+-3.0 gl libvlc`
