#include "stdafx.h"
#include "FileUtils.h"
#include "LogHelper.h"

#include <boost/dll.hpp>
#include <boost/algorithm/string.hpp>

#define PATH_SEPARATOR std::string(1, boost::filesystem::path::preferred_separator)

std::string FileUtils::GetCurrentExecutableFolder() {
    boost::filesystem::path currentDir(boost::dll::program_location().parent_path());
    return currentDir.string();
}

std::string FileUtils::CombinePath(const std::string& basePath, const std::string& subPath) {
    return boost::trim_right_copy_if(basePath, boost::is_any_of(PATH_SEPARATOR))
        + PATH_SEPARATOR + boost::trim_left_copy_if(subPath, boost::is_any_of(PATH_SEPARATOR));
}

bool FileUtils::ReadFile(const std::string& fileName, std::vector<std::uint8_t>& buffer) {
    std::ifstream file(fileName, std::ios::binary);

    if (!file) {
        return false;
    }

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    auto len = file.tellg();
    file.seekg(0, std::ios::beg);
    buffer.resize(len);
    

    file.read((char *)&buffer[0], len);
    file.close();
    return true;
}

bool FileUtils::CreateDirs(const boost::filesystem::path& path) {
    boost::system::error_code ec;
    boost::filesystem::create_directories(path, ec);

    if (ec) {
        LOG_EX_ERROR("Enable to create dir: %s, error: %s", path.string().c_str(), ec.message().c_str());
        
        return false;
    }

    return true;
}