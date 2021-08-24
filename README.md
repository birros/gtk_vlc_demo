## Fedora Silverblue

```shell
$ toolbox enter
$ sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
$ sudo dnf install gcc vlc-devel mesa-dri-drivers
$ sudo dnf install gtk4-devel # or gtk3-devel
$ make gtk4 # or gtk3
$ ./gtk_player
```
