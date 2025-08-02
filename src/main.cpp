#include "events.hpp"
#include "pkgs.hpp"

#include <array>
#include <filesystem>
#include <iostream>

#include <tracy/Tracy.hpp>

GtkBuilder* builder = 0;
Pkgs* p;

int main(int argc, char** argv) {
	ZoneScoped;
	gtk_init(&argc, &argv);

	constexpr const std::array uiPaths = {
#ifdef DEBUG
		"../data/pepv.ui",
#endif
		PREFIX "/" DATADIR "/pepv/pepv.ui",
		"/usr/share/pepv/pepv.ui",
		"/usr/local/share/pepv/pepv.ui",
		"/app/share/pepv/pepv.ui",
		"/app/share/runtime/share/pepv/pepv.ui",
		"/run/host/user-share/pepv/pepv.ui",
		"/run/host/share/pepv/pepv.ui",
	};

	for (auto& f : uiPaths) {
		if (fs::exists(f)) {
#ifdef DEBUG
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

	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_builder_connect_signals(builder, NULL);

	p = new Pkgs();
	p->init();
	populatePkgList();

	gtk_widget_show(GTK_WIDGET(window));

	gtk_main();
}
