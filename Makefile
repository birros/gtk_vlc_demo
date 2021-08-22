all: gtk4

.PHONY4: gtk4
gtk4: main.c
	gcc -o gtk_player main.c `pkg-config --libs gtk4 libvlc` `pkg-config --cflags gtk4 libvlc`

.PHONY4: gtk3
gtk3: main.c
	gcc -o gtk_player main.c `pkg-config --libs gtk+-3.0 libvlc` `pkg-config --cflags gtk+-3.0 libvlc`
