## Fedora Silverblue

```shell
$ toolbox enter
$ sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
$ sudo dnf install gcc vlc-devel
$ sudo dnf install gtk4-devel # or gtk3-devel
$ make gtk4 # or gtk3
$ GDK_BACKEND=x11 ./gtk_player
```
