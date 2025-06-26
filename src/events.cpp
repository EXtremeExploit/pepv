#include "events.hpp"

#include "pkgs.hpp"
#include "utils.hpp"

#include <iostream>

#include <tracy/Tracy.hpp>

std::string selected;

void updatePkgInfo() {
	ZoneScopedN("updatePkgInfo");
	const std::string tracyArgs = "Package: " + selected;
	___tracy_scoped_zone.Text(tracyArgs.c_str(), tracyArgs.size());
	static auto gPkgInfo   = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "pkgInfo"));
	static auto infoBuffer = gtk_text_view_get_buffer(gPkgInfo);

	const auto desc = p->getDescriptionForPackage(selected);

	if (!desc.first) {
		std::cout << "Cannot get description for pkg: " << selected << std::endl;
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
	for (auto& [f, hash] : files) {
		gtk_tree_store_append(gPkgBackup, &iter, NULL);
		gtk_tree_store_set(gPkgBackup, &iter, COL_BACKUP_PATH, f.c_str(), -1);
		gtk_tree_store_set(gPkgBackup, &iter, COL_BACKUP_HASH, hash.c_str(), -1);
	}
}

bool populationIsHappening = false;
void populatePkgList() {
	ZoneScopedN("populatePkgList");
	static const auto selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "select"));
	static const auto treeStore = GTK_TREE_STORE(gtk_builder_get_object(builder, "treeStore"));

	static auto gSearchEntry  = GTK_SEARCH_ENTRY(gtk_builder_get_object(builder, "searchEntry"));
	static auto gSearchInName = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "searchInName"));
	static auto gSearchInDesc = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "searchInDesc"));

	static auto gTypeExplicit   = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "typeExplicit"));
	static auto gTypeDependency = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "typeDependency"));

	static auto gShownPackages = GTK_LABEL(gtk_builder_get_object(builder, "shownPackages"));
	static auto gShownSize     = GTK_LABEL(gtk_builder_get_object(builder, "shownSize"));
	static auto gShownFiles    = GTK_LABEL(gtk_builder_get_object(builder, "shownFiles"));

	static auto gFromCore     = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromCore"));
	static auto gFromExtra    = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromExtra"));
	static auto gFromMultilib = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromMultilib"));
	static auto gFromAUR      = GTK_CHECK_BUTTON(gtk_builder_get_object(builder, "fromAUR"));

	const auto searchTerm   = gtk_entry_get_text(GTK_ENTRY(gSearchEntry));
	const auto query        = std::string_view(searchTerm);
	const auto searchInName = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSearchInName));
	const auto searchInDesc = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gSearchInDesc));

	const auto typeExplicit   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gTypeExplicit));
	const auto typeDependency = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gTypeDependency));

	const auto fromCore     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gFromCore));
	const auto fromExtra    = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gFromExtra));
	const auto fromMultilib = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gFromMultilib));
	const auto fromAUR      = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gFromAUR));

	const auto names = p->getPackagesNames();

	GtkTreeIter iter;

	populationIsHappening = true;
	gtk_tree_selection_unselect_all(selection);

	{
		ZoneNamedN(__tracy_store_clear, "tree store clear", true);
		gtk_tree_store_clear(treeStore);
	}
	uint32_t shownCount = 0;
	uint64_t shownSize  = 0;
	uint64_t totalSize  = 0;
	uint64_t shownFiles = 0;
	uint64_t totalFiles = 0;

	for (auto& e : names) {
		ZoneNamedN(_tracy_zone_populate_package, "populate package", true);
		const std::string tracyArgs = "Package: " + e;
		_tracy_zone_populate_package.Text(tracyArgs.c_str(), tracyArgs.size());
		const auto pkgDescRes = p->getDescriptionForPackage(e);
		if (!pkgDescRes.first) {
			std::cout << "Couldnt get description for pkg: " << e << std::endl;
			continue;
		}
		const auto pkg   = pkgDescRes.second;
		const auto files = p->getFilesForPackage(pkg.name).size();

		totalSize += pkg.size.value_or(0);
		totalFiles += files;

		if (pkg.repo.has_value() && pkg.repo.value() == DBS_CORE && !fromCore) continue;
		if (pkg.repo.has_value() && pkg.repo.value() == DBS_EXTRA && !fromExtra) continue;
		if (pkg.repo.has_value() && pkg.repo.value() == DBS_MULTILIB && !fromMultilib) continue;
		if (!pkg.repo.has_value() && !fromAUR) continue;

		if (pkg.reason == REASON_EXPLICIT && !typeExplicit) continue;
		if (pkg.reason == REASON_DEPEND && !typeDependency) continue;

		if (!query.empty() && (searchInName || searchInDesc)) {
			bool foundQuery = false;
			if (searchInName && pkg.name.find(query) != std::string::npos) foundQuery = true;
			if (searchInDesc && pkg.desc.value_or("").find(query) != std::string::npos) foundQuery = true;
			if (!foundQuery)
				continue;
		}

		std::string_view reasonStr = reasonToStr(pkg.reason);

		const auto name    = pkg.name.c_str();
		const auto version = pkg.version.c_str();
		const auto numDeps = pkg.depends.size();
		const auto desc    = pkg.desc.value_or("");

		std::string_view repo = repoIdToStr(pkg.repo.value_or((REPOS)-1));

		gtk_tree_store_append(treeStore, &iter, NULL);

		gtk_tree_store_set(treeStore, &iter,
						   COL_LIST_NAME, name,
						   COL_LIST_VERSION, version,
						   COL_LIST_REASON, reasonStr.data(),
						   COL_LIST_NUM_DEPS, numDeps,
						   COL_LIST_DESC, desc.c_str(),
						   COL_LIST_SIZE, pkg.size.value_or(0),
						   COL_LIST_FILES, files,
						   COL_LIST_REPO, repo.data(),
						   -1);
		shownCount++;
		shownSize += pkg.size.value_or(0);
		shownFiles += files;
	}
	populationIsHappening = false;

	// Shown indicators
	const auto shownCountStr = std::to_string(shownCount);
	gtk_label_set_label(gShownPackages, shownCountStr.c_str());

	const auto shownSizeStr = formattedSize(shownSize);
	gtk_label_set_label(gShownSize, shownSizeStr.c_str());

	const auto shownFilesStr = std::to_string(shownFiles);
	gtk_label_set_label(gShownFiles, shownFilesStr.c_str());

	// Total indicators
	static auto gTotalPackages = GTK_LABEL(gtk_builder_get_object(builder, "totalPackages"));
	const auto totalStr        = std::to_string(names.size());
	gtk_label_set_label(gTotalPackages, totalStr.c_str());

	static auto gTotalSize  = GTK_LABEL(gtk_builder_get_object(builder, "totalSize"));
	const auto totalSizeStr = formattedSize(totalSize);
	gtk_label_set_label(gTotalSize, totalSizeStr.c_str());

	static auto gTotalFiles  = GTK_LABEL(gtk_builder_get_object(builder, "totalFiles"));
	const auto totalFilesStr = std::to_string(totalFiles);
	gtk_label_set_label(gTotalFiles, totalFilesStr.c_str());
}

extern "C" {
void on_reload_button_clicked(GtkButton* b) {
	ZoneScopedN("on_reload_button_clicked");

	p->uninit();
	p->init();

	populatePkgList();
}

void on_select_changed(GtkWidget* c) {
	ZoneScopedN("on_select_changed");
	if (populationIsHappening) return; // Even when there is nothing selected this function will still be called, dont want that
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

	backupUpdater.join();
	infoUpdater.join();
	filesUpdater.join();
}

void on_applyFilters_clicked(GtkButton* c) {
	ZoneScopedN("on_applyFilters_clicked");
	populatePkgList();
}
}
