# pepv (Pedrito's Epic Pacman Viewer)
#### A GTK3 program designed to inspect your locally installed packages using the pacman package manager

![showcase](./docs/showcase.png)

# Features
* Search using package name or/and description
* Filter by explicitly installed or dependencies
* Filter either by packages in the AUR or in specific repositories
* All of the filters above work together!
* Show how many packages are installed and shown
* View how many dependencies and files each package has
* View super detailed package info and its files

# Featuresn't
* No uninstalling packages
* No installing packages
* No updating packages
* Only support for `core`, `extra` and `multilib` repositories

## Why?
* Because i want this program only to work as a viewer, everything regarding actually messing with your system should be in your own hands with you knowing what you are doing instead of me deciding what packages stay and go blindly

# Building
* git clone this repository
* Open a terminal in the directory of the repo and run the next commands
* `mkdir build && cd build`
* `cmake ..`
* `cmake --build . --config Release`
* pepv needs the files `pepv.ui` and `pepv.png` to be somewhere it can find them together, list of paths is in order
    * `../pepv.ui` Only in debug build for debug purposes
    * `pepv.ui`
    * `/usr/share/pepv/pepv.ui`
    * `/app/share/pepv/pepv.ui`
    * `/app/share/runtime/share/pepv/pepv.ui`
    * `/run/host/user-share/pepv/pepv.ui`
    * `/run/host/share/pepv/pepv.ui`

* If you want you can also resize the logo to multiple resolutions and put the images in `usr/share/icons/hicolor/SIZExSIZE/apps/pepv.png` so GTK chooses the correct resolution automatically

# I dont know how to code but i want to use this program.
* Follow the instructions above, you dont need to understand them
* If you are scared of basic computer usage then you can wait for someone to package this program in your arch-based distro and i may add it in this readme

# Installing
* AUR: https://aur.archlinux.org/packages/pepv-git
