
Debian
====================
This directory contains files used to package sssolutionsd/sssolutions-qt
for Debian-based Linux systems. If you compile sssolutionsd/sssolutions-qt yourself, there are some useful files here.

## sss: URI support ##


sssolutions-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install sssolutions-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your sssolutions-qt binary to `/usr/bin`
and the `../../share/pixmaps/sss128.png` to `/usr/share/pixmaps`

sssolutions-qt.protocol (KDE)

