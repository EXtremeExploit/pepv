#include "pkgs.hpp"
#include "events.hpp"

#include <array>
#include <tracy/Tracy.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <gtk/gtk.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

GtkBuilder* builder;
GtkTreeStore* treeStore;
GtkTreeView* treeView;
std::string selected;
Pkgs* p;

int main(int argc, char** argv) {
	ZoneScoped;
	p = new Pkgs();

	gtk_init(&argc, &argv);

	constexpr const std::array uiPaths = {
#ifndef NDEBUG
		"../pepv.ui",
#endif
		"pepv.ui",
		"/usr/share/pepv/pepv.ui",
		"/app/share/pepv/pepv.ui",
		"/app/share/runtime/share/pepv/pepv.ui",
		"/run/host/user-share/pepv/pepv.ui",
		"/run/host/share/pepv/pepv.ui",
	};

	for (auto& f : uiPaths) {
		if (fs::exists(f)) {
#ifndef NDEBUG
			std::cout << "Using UI file: " << f << std::endl;
#endif
			builder = gtk_builder_new_from_file(f);
			break;
		}
	}
	if (!builder) {
		std::cerr << "UI file not found. Exiting" << std::endl;
		return 1;
	}

	auto window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
	treeView    = GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeView"));

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_builder_connect_signals(builder, NULL);

	p->getFiles();
	populatePkgList();
    updateTotalPackagesLabel();

	gtk_widget_show(GTK_WIDGET(window));

	gtk_main();
}
