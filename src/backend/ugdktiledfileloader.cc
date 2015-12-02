
#include "backend/ugdktiledfileloader.h"

#include <ugdk/filesystem/module.h>
#include <ugdk/filesystem/file.h>
#include <tiled-reader/exceptions.h>

namespace backend {

UgdkTiledFileLoader::UgdkFile::UgdkFile(std::unique_ptr<ugdk::filesystem::File> file)
    : ugdk_file_(std::move(file))
{
}

UgdkTiledFileLoader::UgdkFile::~UgdkFile()
{
}

std::shared_ptr<tiled::FileLoader::File> UgdkTiledFileLoader::OpenFile(const std::string& path) const {
    auto file = ugdk::filesystem::manager()->OpenFile(path);
    if (file) {
        return std::make_shared<UgdkFile>(std::move(file));
    }
    else {
        return nullptr;
    }
}

std::string UgdkTiledFileLoader::GetContents(const std::shared_ptr<tiled::FileLoader::File>& file) const {
    if (auto ugdk_file = dynamic_cast<UgdkFile*>(file.get())) {
        return ugdk_file->ugdk_file().GetContents();
    }
    else {
        throw tiled::BaseException("Invalid file");
    }
}

} // namespace backend
