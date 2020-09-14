
Debian
====================
This directory contains files used to package flitsd/flits-qt
for Debian-based Linux systems. If you compile flitsd/flits-qt yourself, there are some useful files here.

## FLS: URI support ##


flits-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install flits-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your flits-qt binary to `/usr/bin`
and the `../../share/pixmaps/FLS128.png` to `/usr/share/pixmaps`

flits-qt.protocol (KDE)

