#include "events.hpp"

#include "pkgs.hpp"
#include "utils.hpp"

#include <iostream>

#include <tracy/Tracy.hpp>

GtkTreeSelection* selection = nullptr;

void updatePkgInfo() {
	ZoneScopedN("updatePkgInfo");
	const std::string tracyArgs = "Package: " + selected;
	___tracy_scoped_zone.Text(tracyArgs.c_str(), tracyArgs.size());
	static auto gPkgInfo   = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "pkgInfo"));
	static auto infoBuffer = gtk_text_view_get_buffer(gPkgInfo);

	const auto desc = p->getDescriptionForPackage(selected);

	if (!desc.first) {
		std::cout << "Couldnt get description for pkg: " << selected << std::endl;
		return;
	}

	const auto pkg = desc.second;
	std::stringstream infoss;
	infoss << "Name: " << pkg.name << std::endl;
	infoss << "Version: " << pkg.version << std::endl;
	if (pkg.base.has_value())
		infoss << "Base: " << pkg.base.value() << std::endl;

	if (pkg.desc.has_value())
		infoss << "Description: " << pkg.desc.value() << std::endl;
	if (pkg.url.has_value())
		infoss << "URL: " << pkg.url.value() << std::endl;
	if (pkg.arch.has_value())
		infoss << "Arch: " << pkg.arch.value() << std::endl;
	if (pkg.buildDate.has_value()) {
		auto bdss = formattedTimestamp(pkg.buildDate.value());
		infoss << "Build date: " << bdss.c_str() << std::endl;
	}
	if (pkg.installDate.has_value()) {
		auto idss = formattedTimestamp(pkg.installDate.value());
		infoss << "Install date: " << idss.c_str() << std::endl;
	}
	if (pkg.packager.has_value())
		infoss << "Packager: " << pkg.packager.value() << std::endl;
	if (pkg.size.has_value()) {
		const auto s = formattedSize(pkg.size.value());
		infoss << "Size: " << s.c_str() << " (" << pkg.size.value() << " Bytes)" << std::endl;
	}
	infoss << "Reason: " << reasonToStr(pkg.reason) << std::endl;

	if (pkg.groups.size()) {
		const auto s = setToStr(pkg.groups);
		infoss << "Groups: " << s << std::endl;
	}

	if (pkg.license.has_value())
		infoss << "License: " << pkg.license.value() << std::endl;
	if (pkg.validation.has_value())
		infoss << "Validation: " << pkg.validation.value() << std::endl;

	if (pkg.replaces.size()) {
		const auto s = setToStr(pkg.replaces);
		infoss << "Replaces: " << s << std::endl;
	}

	if (pkg.depends.size()) {
		const auto s = setToStr(pkg.depends);
		infoss << "Depends: " << s << std::endl;
	}

	if (pkg.requiredBy.size()) {
		const auto s = setToStr(pkg.requiredBy);
		infoss << "Required by: " << s << std::endl;
	}

	if (pkg.conflicts.size()) {
		const auto s = setToStr(pkg.conflicts);
		infoss << "Conflicts: " << s << std::endl;
	}
	if (pkg.optDepends.size()) {
		const auto s = optDependsToStr(pkg.optDepends);
		infoss << s << std::endl;
	}
	if (pkg.optRequiredBy.size()) {
		const auto s = optRequiredByToStr(pkg.optRequiredBy);
		infoss << s << std::endl;
	}

	if (pkg.provides.size()) {
		const auto s = setToStr(pkg.provides);
		infoss << "Provides: " << s << std::endl;
	}

	const auto infoStr = infoss.str();
	gtk_text_buffer_set_text(infoBuffer, infoStr.c_str(), infoStr.length());
	gtk_text_view_set_buffer(gPkgInfo, infoBuffer);
}

void updatePkgFiles() {
	ZoneScopedN("updatePkgFiles");
	const std::string tracyArgs = "Package: " + selected;
	___tracy_scoped_zone.Text(tracyArgs.c_str(), tracyArgs.size());
	static auto gPkgFiles   = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "pkgFiles"));
	static auto filesBuffer = gtk_text_view_get_buffer(gPkgFiles);

	const auto files    = p->getFilesForPackage(selected);
	const auto filesStr = setToStr(files, "\n");
	gtk_text_buffer_set_text(filesBuffer, filesStr.c_str(), filesStr.length());
	gtk_text_view_set_buffer(gPkgFiles, filesBuffer);
}

void updatePkgBackupFiles() {
	ZoneScopedN("updatePkgBackupFiles");
	const std::string tracyArgs = "Package: " + selected;
	___tracy_scoped_zone.Text(tracyArgs.c_str(), tracyArgs.size());
	static auto gPkgBackup = GTK_TREE_STORE(gtk_builder_get_object(builder, "backupFilesStore"));

	gtk_tree_store_clear(gPkgBackup);

	const auto files = p->getBackupFilesForPackage(selected);
	GtkTreeIter iter;
	for (auto& [f, h] : files) {
		gtk_tree_store_append(gPkgBackup, &iter, NULL);
		gtk_tree_store_set(gPkgBackup, &iter, COL_BACKUP_PATH, f.c_str(), -1);
		gtk_tree_store_set(gPkgBackup, &iter, COL_BACKUP_HASH, h.c_str(), -1);
	}
}

bool populationIsHappening = false;
void populatePkgList() {
	if (!selection)
		selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "select"));
	ZoneScopedN("populatePkgList");
	if (!treeStore)
		treeStore = GTK_TREE_STORE(gtk_builder_get_object(builder, "treeStore"));

	static auto gSearchEntry  = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder, "searchEntry"));
	static auto gSearchInName = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "searchInName"));
	static auto gSearchInDesc = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "searchInDesc"));

	static auto gTypeRadioButtonAny        = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "typeRadioButtonAny"));
	static auto gTypeRadioButtonExplicit   = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "typeRadioButtonExplicit"));
	static auto gTypeRadioButtonDependency = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "typeRadioButtonDependency"));

	static auto gShownPackages = GTK_LABEL(gtk_builder_get_object(builder, "shownPackages"));

	static auto gSourceOfficial = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromOfficial"));
	static auto gSourceAUR      = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromAUR"));

	const auto searchTerm   = gtk_entry_get_text(GTK_ENTRY(gSearchEntry));
	const auto query        = std::string_view(searchTerm);
	const auto searchInName = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSearchInName));
	const auto searchInDesc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSearchInDesc));

	const auto typeAny        = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gTypeRadioButtonAny));
	const auto typeExplicit   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gTypeRadioButtonExplicit));
	const auto typeDependency = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gTypeRadioButtonDependency));

	const auto fromOfficial = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSourceOfficial));
	const auto fromAUR      = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSourceAUR));

	auto names = p->getPackagesNames();

	GtkTreeIter iter;

	gtk_tree_selection_unselect_all(selection);
	populationIsHappening = true;

	gtk_tree_store_clear(treeStore);

	uint32_t shown = 0;

	for (auto& e : names) {
		ZoneNamedN(_tracy_zone_populate_package, "populate package", true);
		const std::string tracyArgs = "Package: " + e;
		_tracy_zone_populate_package.Text(tracyArgs.c_str(), tracyArgs.size());
		const auto pkgDescRes = p->getDescriptionForPackage(e);
		if (!pkgDescRes.first) {
			std::cout << "Couldnt get description for pkg: " << e << std::endl;
			continue;
		}
		const auto pkg = pkgDescRes.second;

		if (!fromOfficial && !pkg.isLocal) continue;
		if (!fromAUR && pkg.isLocal) continue;

		if (typeExplicit && pkg.reason != REASON_EXPLICIT) continue;
		if (typeDependency && pkg.reason != REASON_DEPEND) continue;

		if (!query.empty() && (searchInName || searchInDesc)) {
			bool foundQuery = false;
			if (searchInName && pkg.name.find(query) != std::string::npos) foundQuery = true;
			if (searchInDesc && pkg.desc.value_or("").find(query) != std::string::npos) foundQuery = true;
			if (!foundQuery)
				continue;
		}

		std::string reasonStr = reasonToStr(pkg.reason);

		const auto name    = pkg.name.c_str();
		const auto version = pkg.version.c_str();
		const auto numDeps = pkg.depends.size();
		const auto desc    = pkg.desc.value_or("");

		const auto files = p->getFilesForPackage(pkg.name).size();

		gtk_tree_store_append(treeStore, &iter, NULL);

		gtk_tree_store_set(treeStore, &iter, COL_LIST_NAME, name, -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_VERSION, version, -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_REASON, reasonStr.c_str(), -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_NUM_DEPS, numDeps, -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_DESC, desc.c_str(), -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_SIZE, pkg.size.value_or(0), -1);
		gtk_tree_store_set(treeStore, &iter, COL_LIST_FILES, files, -1);
		shown++;
	}
	populationIsHappening = false;

	const auto shownStr = std::to_string(shown);
	gtk_label_set_label(gShownPackages, shownStr.c_str());
}

void updateTotalPackagesLabel() {
	static auto gTotalPackages = GTK_LABEL(gtk_builder_get_object(builder, "totalPackages"));
	const auto total           = p->getPackagesNames().size();
	const auto totalStr        = std::to_string(total);
	gtk_label_set_label(gTotalPackages, totalStr.c_str());
}

extern "C" {
void on_reload_button_clicked(GtkButton* b) {
	ZoneScopedN("on_reload_button_clicked");
	if (!treeStore)
		treeStore = GTK_TREE_STORE(gtk_builder_get_object(builder, "treeStore"));

	p->clear();

	populatePkgList();
}

void on_select_changed(GtkWidget* c) {
	ZoneScopedN("on_select_changed");
	if (populationIsHappening) return;
	gchar* value;
	GtkTreeIter iter;
	GtkTreeModel* model;

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(c), &model, &iter) == FALSE)
		return;

	gtk_tree_model_get(model, &iter, COL_LIST_NAME, &value, -1);

	selected = value;

	std::thread filesUpdater(updatePkgFiles);
	std::thread infoUpdater(updatePkgInfo);
	std::thread backupUpdater(updatePkgBackupFiles);

	infoUpdater.join();
	filesUpdater.join();
	backupUpdater.join();
}

void on_applyFilters_clicked(GtkButton* c) {
	ZoneScopedN("on_applyFilters_clicked");
	populatePkgList();
}
}
