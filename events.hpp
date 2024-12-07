#pragma once
#include "pkgs.hpp"
#include <gtk/gtk.h>

extern GtkBuilder* builder;
extern GtkTreeStore* treeStore;
extern std::string selected;
extern Pkgs* p;

void populatePkgList();
void updateTotalPackagesLabel();

extern "C" {
void on_reload_button_clicked(GtkButton* b);
}
