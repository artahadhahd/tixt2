#ifndef MANAGER_H
#define MANAGER_H
#include "common.hpp"
#include "cursor.hpp"

enum class DirectoryFilterBy : int8_t {
    None = -1,
    Directory = DT_DIR,
    File = DT_REG,
    SymLink = DT_LNK
};

struct File {
    std::string name;
    DirectoryFilterBy filter;
    friend bool operator<(const File &lhs, const File &rhs) {
        return lhs.name < rhs.name;
    }
};

struct FileManager {
    explicit FileManager(std::string);
    [[nodiscard]] bool exists();
    std::string get_file();
    std::string path;
    Cursor cursor;
private:
    bool update_renderqueue = true;
    void exec_sequence(int);
    std::optional<std::vector<File>> getFiles(DirectoryFilterBy);
    std::vector<File> sortFiles();
    void renderFiles();
};

#endif