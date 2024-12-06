#include "pkgs.hpp"
#include <cassert>
#include <iostream>
#include <linux/limits.h>
#include <tracy/Tracy.hpp>

void Pkgs::clear() {
	pkgPaths.clear();
	descriptions.clear();
	files.clear();
	backupFiles.clear();
}

std::set<std::string> Pkgs::getPackagesNames() {
	ZoneScopedN("Pkgs::getPackagesNames()");

	std::set<std::string> names = {};

	if (!descriptions.size())
		this->getDescriptions();

	for (auto& desc : descriptions) {
		names.insert(desc.first);
	}
	return names;
}

std::pair<std::map<std::string, PackageDescription>, bool> Pkgs::getDescriptions() {
	ZoneScopedN("Pkgs::getDescriptions()");

	if (descriptions.size() != 0) {
		return {descriptions, true};
	}

	std::set<fs::path> tempPkgsPaths;

	for (const auto& entry : fs::directory_iterator(this->dbPath)) {
		if (!entry.is_directory()) continue;
		auto entryPath   = entry.path();
		auto entryString = entryPath.string();
		auto cleanEntry  = entryString.replace(0, this->dbPath.string().length(), "");
		tempPkgsPaths.insert(cleanEntry);
	}

	std::map<std::string, PackageDescription> pkgsDescs;

	for (const auto& path : tempPkgsPaths) {
		auto currentSection = SECTION_DESC_NONE;

		const auto descPath = this->dbPath / path / "desc";

		auto f                 = fopen(descPath.c_str(), "rb");
		PackageDescription pkg = {};

		char charCurrentLine[1024];
		while (std::fgets(charCurrentLine, sizeof(charCurrentLine), f)) {
			charCurrentLine[strcspn(charCurrentLine, "\n")] = 0;
			auto currentLine                                = std::string_view(charCurrentLine);
			if (currentLine == "") continue;
			if (currentLine.starts_with("%") && currentLine.ends_with("%")) {
				currentSection = SECTION_DESC_NONE;
				if (currentLine == "%NAME%") currentSection = SECTION_DESC_NAME;
				if (currentLine == "%VERSION%") currentSection = SECTION_DESC_VERSION;
				if (currentLine == "%BASE%") currentSection = SECTION_DESC_BASE;
				if (currentLine == "%DESC%") currentSection = SECTION_DESC_DESC;
				if (currentLine == "%URL%") currentSection = SECTION_DESC_URL;
				if (currentLine == "%ARCH%") currentSection = SECTION_DESC_ARCH;
				if (currentLine == "%BUILDDATE%") currentSection = SECTION_DESC_BUILDDATE;
				if (currentLine == "%INSTALLDATE%") currentSection = SECTION_DESC_INSTALLDATE;
				if (currentLine == "%PACKAGER%") currentSection = SECTION_DESC_PACKAGER;
				if (currentLine == "%SIZE%") currentSection = SECTION_DESC_SIZE;
				if (currentLine == "%REASON%") currentSection = SECTION_DESC_REASON;
				if (currentLine == "%GROUPS%") currentSection = SECTION_DESC_GROUPS;
				if (currentLine == "%LICENSE%") currentSection = SECTION_DESC_LICENSE;
				if (currentLine == "%VALIDATION%") currentSection = SECTION_DESC_VALIDATION;
				if (currentLine == "%REPLACES%") currentSection = SECTION_DESC_REPLACES;
				if (currentLine == "%DEPENDS%") currentSection = SECTION_DESC_DEPENDS;
				if (currentLine == "%CONFLICTS%") currentSection = SECTION_DESC_CONFLICTS;
				if (currentLine == "%OPTDEPENDS%") currentSection = SECTION_DESC_OPTDEPENDS;
				if (currentLine == "%PROVIDES%") currentSection = SECTION_DESC_PROVIDES;
				if (currentLine == "%XDATA%") currentSection = SECTION_DESC_XDATA;

				if (currentSection == SECTION_DESC_NONE) {
					std::cout << "Unrecognized section " << currentLine << "on package " << path << std::endl;
					return {{}, false};
				}
				continue;
			}

			switch (currentSection) {
				case SECTION_DESC_NAME:
					pkg.name = currentLine;
					break;
				case SECTION_DESC_VERSION:
					pkg.version = currentLine;
					break;
				case SECTION_DESC_BASE:
					pkg.base.emplace(currentLine);
					break;
				case SECTION_DESC_DESC:
					pkg.desc.emplace(currentLine);
					break;
				case SECTION_DESC_URL:
					pkg.url.emplace(currentLine);
					break;
				case SECTION_DESC_ARCH:
					pkg.arch.emplace(currentLine);
					break;
				case SECTION_DESC_BUILDDATE: {
					auto num = std::stoull(currentLine.data());
					pkg.buildDate.emplace(num);
					break;
				}
				case SECTION_DESC_INSTALLDATE: {
					auto num = std::stoull(currentLine.data());
					pkg.installDate.emplace(num);
					break;
				}
				case SECTION_DESC_PACKAGER:
					pkg.packager.emplace(currentLine);
					if (currentLine == "Unknown Packager")
						pkg.isLocal = true; // TODO: is this correct?
					break;
				case SECTION_DESC_SIZE: {
					auto num = std::stoull(currentLine.data());
					pkg.size.emplace(num);
					break;
				}
				case SECTION_DESC_REASON: {
					auto num   = std::stoi(currentLine.data());
					pkg.reason = PKGReason(num);
					break;
				}
				case SECTION_DESC_GROUPS:
					pkg.groups.emplace(currentLine);
					break;
				case SECTION_DESC_LICENSE:
					pkg.license.emplace(currentLine);
					break;
				case SECTION_DESC_VALIDATION:
					pkg.validation.emplace(currentLine);
					break;
				case SECTION_DESC_REPLACES:
					pkg.replaces.emplace(currentLine);
					break;
				case SECTION_DESC_DEPENDS:
					pkg.depends.emplace(currentLine);
					break;
				case SECTION_DESC_CONFLICTS:
					pkg.conflicts.emplace(currentLine);
					break;
				case SECTION_DESC_OPTDEPENDS:
					pkg.optDepends.emplace(currentLine);
					break;
				case SECTION_DESC_PROVIDES:
					pkg.provides.emplace(currentLine);
					break;
				case SECTION_DESC_XDATA: {
					pkg.xdata.emplace(currentLine);
					break;
				}
				case SECTION_DESC_NONE:
					std::cerr << "Text in NONE section on desc at \"" << path << "\"; " << "Text: " << currentLine << std::endl;
					break;
			}
		}
		fclose(f);
		pkgPaths.insert({pkg.name, path});
		pkgsDescs.insert({pkg.name, pkg});
	}

	for (auto& pkg : pkgsDescs) {
		for (auto& dep : pkg.second.depends) {
			if (pkgsDescs.contains(dep))
				pkgsDescs.at(dep).requiredBy.insert(pkg.first);
		}
		for (auto& dep : pkg.second.optDepends) {
			if (pkgsDescs.contains(dep))
				pkgsDescs.at(dep).optRequiredBy.insert(pkg.first);
		}
	}

	descriptions = pkgsDescs;
	return {pkgsDescs, true};
};

std::pair<std::map<std::string, std::set<std::string>>, bool> Pkgs::getFiles() {
	ZoneScopedN("Pkgs::getFiles()");
	if (files.size())
		return std::pair{files, true};

	if (pkgPaths.size() == 0)
		return {{}, false};

	std::map<std::string, std::set<std::string>> pkgsFiles;
	std::map<std::string, std::map<std::string, std::string>> pkgsBackupFiles;
	for (const auto& [name, path] : pkgPaths) {
		auto currentSection = SECTION_FILES_NONE;

		const auto descPath = this->dbPath / path / "files";

		auto f                                            = fopen(descPath.c_str(), "rb");
		std::set<std::string> pkgFiles                    = {};
		std::map<std::string, std::string> pkgBackupFiles = {};

		char charCurrentLine[PATH_MAX];
		while (std::fgets(charCurrentLine, sizeof(charCurrentLine), f)) {
			charCurrentLine[strcspn(charCurrentLine, "\n")] = 0;
			auto currentLine                                = std::string(charCurrentLine);
			if (currentLine == "") continue;
			if (currentLine.starts_with("%") && currentLine.ends_with("%")) {
				currentSection = SECTION_FILES_NONE;
				if (currentLine == "%FILES%") currentSection = SECTION_FILES_FILES;
				if (currentLine == "%BACKUP%") currentSection = SECTION_FILES_BACKUP;

				if (currentSection == SECTION_FILES_NONE) {
					std::cout << "Unrecognized section " << currentLine << "on package files " << path << std::endl;
					return {{}, false};
				}
				continue;
			}

			switch (currentSection) {
				case SECTION_FILES_FILES:
					pkgFiles.insert(currentLine);
					break;
				case SECTION_FILES_BACKUP: {
					const auto separator = currentLine.find_last_of(0x09);
					const auto p         = currentLine.substr(0, separator);
					const auto hash      = currentLine.substr(separator + 1);
					pkgBackupFiles.insert({p, hash});
					break;
				}
				case SECTION_FILES_NONE:
					std::cerr << "Text in NONE section on files at \"" << path << "\"; " << "Text: " << currentLine << std::endl;
					break;
			}
		}
		fclose(f);
		pkgsFiles.insert({name, pkgFiles});
		pkgsBackupFiles.insert({name, pkgBackupFiles});
	}
	files       = pkgsFiles;
	backupFiles = pkgsBackupFiles;
	return {pkgsFiles, true};
};

std::pair<std::map<std::string, std::map<std::string, std::string>>, bool> Pkgs::getBackupFiles() {
	ZoneScopedN("Pkgs::getBackupFiles()");
	if (backupFiles.size())
		return {backupFiles, true};

	const auto f = this->getFiles();

	if (!f.second)
		return {{}, false};
	return {backupFiles, true};
}

std::pair<bool, PackageDescription> Pkgs::getDescriptionForPackage(const std::string& pkg) {
	ZoneScopedN("Pkgs::getDescriptionForPackage");
	if (descriptions.contains(pkg))
		return {true, descriptions.at(pkg)};
	else
		return {false, {}};
}
std::set<std::string> Pkgs::getFilesForPackage(const std::string& pkg) {
	ZoneScopedN("Pkgs::getFilesForPackage");
	if (files.contains(pkg))
		return files.at(pkg);

	const auto f = this->getFiles();
	if (!f.second)
		return {};

	if (f.first.contains(pkg))
		return f.first.at(pkg);
	else
		return {};
}

std::map<std::string, std::string> Pkgs::getBackupFilesForPackage(const std::string& pkg) {
	ZoneScopedN("Pkgs::getBackupFilesForPackage()");
	if (backupFiles.contains(pkg))
		return backupFiles.at(pkg);

	const auto f = this->getFiles();
	if (!f.second)
		return {};
	else
		return backupFiles.at(pkg);
}

std::map<std::string, PackageDescription> Pkgs::getDescriptionsUnsafe() {
	ZoneScopedN("Pkgs::getDescriptionsUnsafe()");
	if (descriptions.size())
		return descriptions;
	else {
		return this->getDescriptions().first;
	}
}

std::map<std::string, std::set<std::string>> Pkgs::getFilesUnsafe() {
	ZoneScopedN("Pkgs::getFilesUnsafe()");
	if (files.size())
		return files;
	else {
		return this->getFiles().first;
	}
}
