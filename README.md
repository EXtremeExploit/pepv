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
    * Also supports viewing AUR/locally built packages

## Why?
* Because i want this program only to work as a viewer, everything regarding actually messing with your system should be in your own hands with you knowing what you are doing instead of me deciding what packages stay and go blindly

# Installing
* AUR: https://aur.archlinux.org/packages/pepv-git


# Building
## Requirements
* Meson (1.1 or higher)
* git
## Instructions
* git clone this repository
* Open a terminal in the directory of the repo and run the next commands for a release build
* `meson setup build -Dtracy_enable=false -Dbuildtype=release`
* `meson compile -C build`
* `meson install -C build --skip-subprojects` To install locally using built binary, this also includes:
    * Desktop entry
    * Icons
    * README.md in doc
    * UI file
* pepv will also look for `pepv.ui` in other paths, list of paths is in order
    * `../data/pepv.ui` Only in debug build for debug purposes
    * `PREFIX "/" DATADIR "/pepv/pepv.ui"` (PREFIX and DATADIR may be set by meson options for packaging purposes)
    * `/usr/share/pepv/pepv.ui`
    * `/usr/local/share/pepv/pepv.ui`
    * `/app/share/pepv/pepv.ui`
    * `/app/share/runtime/share/pepv/pepv.ui`
    * `/run/host/user-share/pepv/pepv.ui`
    * `/run/host/share/pepv/pepv.ui`

# I dont know how to code but i want to use this program in a distro that doesnt have AUR.
* Follow the instructions above, you dont need to understand them
* If you are scared of basic computer usage then you can wait for someone to package this program in your distro and i may add it in this README or you can just open a new issue to ask for help
