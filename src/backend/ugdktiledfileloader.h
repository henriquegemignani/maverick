#ifndef MAVERICK_BACKEND_UGDKTILEDFILELOADER_H_
#define MAVERICK_BACKEND_UGDKTILEDFILELOADER_H_

#include <memory>
#include <tiled-reader/fileloader.h>
#include <ugdk/filesystem.h>

namespace backend {

class UgdkTiledFileLoader : public tiled::FileLoader {
public:
    class UgdkFile : public tiled::FileLoader::File {
    public:
        UgdkFile(std::unique_ptr<ugdk::filesystem::File> file);

        ugdk::filesystem::File& ugdk_file() const {
            return *ugdk_file_;
        }

        virtual ~UgdkFile();
    private:
        std::unique_ptr<ugdk::filesystem::File> ugdk_file_;
    };

    virtual std::shared_ptr<File> OpenFile(const std::string& path) const override;
    virtual std::string GetContents(const std::shared_ptr<File>& file) const override;
};

} // namespace backend

#endif // MAVERICK_BACKEND_UGDKTILEDFILELOADER_H_
