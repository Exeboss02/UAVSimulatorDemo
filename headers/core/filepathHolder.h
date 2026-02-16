#pragma once

#include <string>
#include <filesystem>

class FilepathHolder {
public:
	static void SetDirectiories() { 
		std::filesystem::path exePath = std::filesystem::canonical(L"lillaspel.exe");
		FilepathHolder::exeDirectory = exePath.parent_path();

		std::filesystem::path assetsPath = exeDirectory / ".." / ".." / ".." / ".." / "assets";
		FilepathHolder::assetsDirectory = std::filesystem::canonical(assetsPath);
	}

	static inline const std::filesystem::path& GetExeDirectory() { return FilepathHolder::exeDirectory; }
	static inline const std::filesystem::path& GetAssetsDirectory() { return FilepathHolder::assetsDirectory; }

private:
	static std::filesystem::path exeDirectory;
	static std::filesystem::path assetsDirectory;
};
