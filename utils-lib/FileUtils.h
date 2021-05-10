#pragma once 

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

class FileUtils {
public:
    FileUtils() = delete;

    /**
     * Gets the parent folder of the current executable or dll
     */ 
    static std::string GetCurrentExecutableFolder();

    /**
     * Combines two paths.
     * Ex:
     *       basePath = /var/log
     *       subPath = out.log
     *       result: /var/log/out.log
     */
    static std::string CombinePath(const std::string& basePath, const std::string& subPath);

    static bool ReadFile(const std::string& fileName, std::vector<std::uint8_t>& buffer);

    static bool WriteFile(const std::string& fileName, char *data, std::uint32_t size);

    static bool AppendFile(const std::string& fileName, char *data, std::uint32_t size);

    static bool CreateDirs(const boost::filesystem::path& path);
};