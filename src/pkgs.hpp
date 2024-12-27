#pragma once

#include <map>
#include <set>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

enum LIST_COLS {
	COL_LIST_NAME = 0,
	COL_LIST_VERSION,
	COL_LIST_REASON,
	COL_LIST_NUM_DEPS,
	COL_LIST_DESC,
	COL_LIST_SIZE,
	COL_LIST_FILES,
};

enum BACKUP_COLS {
	COL_BACKUP_PATH = 0,
	COL_BACKUP_HASH,
};

enum SectionDesc {
	SECTION_DESC_NONE,
	SECTION_DESC_NAME,
	SECTION_DESC_VERSION,
	SECTION_DESC_BASE,
	SECTION_DESC_DESC,
	SECTION_DESC_URL,
	SECTION_DESC_ARCH,
	SECTION_DESC_BUILDDATE,
	SECTION_DESC_INSTALLDATE,
	SECTION_DESC_PACKAGER,
	SECTION_DESC_SIZE,
	SECTION_DESC_REASON,
	SECTION_DESC_GROUPS,
	SECTION_DESC_LICENSE,
	SECTION_DESC_VALIDATION,
	SECTION_DESC_REPLACES,
	SECTION_DESC_DEPENDS,
	SECTION_DESC_CONFLICTS,
	SECTION_DESC_OPTDEPENDS,
	SECTION_DESC_PROVIDES,
	SECTION_DESC_XDATA,
};

enum SectionFiles {
	SECTION_FILES_NONE,
	SECTION_FILES_FILES,
	SECTION_FILES_BACKUP
};

enum PKGReason : uint32_t {
	REASON_EXPLICIT,
	REASON_DEPEND,
	REASON_UNKNOWN
};

struct PackageDescription {
	std::string name;
	std::string version;
	std::optional<std::string> base;
	std::optional<std::string> desc;
	std::optional<std::string> url;
	std::optional<std::string> arch;
	std::optional<time_t> buildDate;
	std::optional<time_t> installDate;
	std::optional<std::string> packager;
	std::optional<uint64_t> size;
	PKGReason reason = REASON_EXPLICIT;
	std::set<std::string> groups;
	std::optional<std::string> license;
	std::optional<std::string> validation;
	std::set<std::string> replaces;
	std::set<std::string> depends;
	std::set<std::string> conflicts;
	std::set<std::string> optDepends;
	std::set<std::string> provides;
	std::optional<std::string> xdata;
	std::set<std::string> requiredBy;
	std::set<std::string> optRequiredBy;
	bool isLocal = false; // AUR / self-packaged
};

class Pkgs {
	const fs::path dbPath = "/var/lib/pacman/local/";

	std::map<std::string, fs::path> pkgPaths;

    bool inited = false;

	std::map<std::string, PackageDescription> descriptions;
	std::map<std::string, std::set<std::string>> files;
	std::map<std::string, std::map<std::string, std::string>> backupFiles;

	public:
	std::set<std::string> getPackagesNames();

	std::map<std::string, PackageDescription> getDescriptions();
	std::map<std::string, std::set<std::string>> getFiles();
	std::map<std::string, std::map<std::string, std::string>> getBackupFiles();

	std::pair<bool, PackageDescription> getDescriptionForPackage(const std::string& pkg);
	std::set<std::string> getFilesForPackage(const std::string& pkg);
	std::map<std::string, std::string> getBackupFilesForPackage(const std::string& pkg);

	void init();
	void uninit();
};
