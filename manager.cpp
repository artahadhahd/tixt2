#include "manager.hpp"

FileManager::FileManager(std::string path) {
    this->path = path;
    cursor.shape = ">";
    cursor.win = stdscr;
    cursor.x = cursor.y = 0;
    cursor.ymin = 0;
    cursor.ymax = 10;
    cursor.setColor(random_number(1, 6));
}

bool FileManager::exists() {
    try {
        this->path = fs::canonical(path);
    } catch(...) {
        return false;
    }
    return true;
}

void FileManager::exec_sequence(int ch)
{
    int n = getch();
    int times = ch - '0';
    switch (n)
    {
    case ctrl('j'):
    case KEY_DOWN:
        cursor.moveDown(times);
        break;
    case ctrl('k'):
    case KEY_UP:
        cursor.moveUp(times);
        break;
    default:
        (void)ungetch(n);
    }
}

std::string FileManager::get_file() {
    clear();
    refresh();
    int ch = 0;
    auto files = sortFiles();
    cursor.ymax = files.size();
    // if (update_renderqueue) {
        renderQueue.push_back([this] {
            cursor.render();
        });
        renderQueue.push_back([&files] {
        for (size_t i = 0; i < (size_t)global_terminal.y && i < files.size(); ++i) {
            int color_pair = COLOR_PAIR((int)files.at(i).filter);
            attron(color_pair);
            mvprintw(i, 2, "%s", files.at(i).name.c_str());
            attroff(color_pair);
        }
    });
    // }
   
    bool chosen = false;
    render();
    do {
        switch (ch)
        {
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            exec_sequence(ch);
            break;
        case KEY_RETURN:
            chosen = true;
            break;
        case ctrl('k'):
        case KEY_UP:
            cursor.moveUp(1);
            break;
        case ctrl('j'):
        case KEY_DOWN:
            cursor.moveDown(1);
            break;
        default:
            break;
        }
        cursor.render();
    } while (!chosen && (ch = getch()) != ctrl('q'));
    if (chosen) {
        clear();
        refresh();
        update_renderqueue = false;
        if (files.at(cursor.y).filter == DirectoryFilterBy::Directory) {
            refresh();
            fs::path current(path);
            fs::path selected(files.at(cursor.y).name);
            cursor.y = 0;
            path = fs::canonical(current / selected);
            // TODO: create renderqueues for individual modules, we don't want to delete ALL things
            renderQueue.clear();
            mvprintw(2, 40, "%s", path.c_str());
            getch();
            return get_file();
        }
    }
    return path;
}

std::optional<std::vector<File>> FileManager::getFiles(DirectoryFilterBy filter)
{
    DIR * d = opendir(path.c_str());
    if (!d) {
        return std::nullopt;
    }
    struct dirent * dir;
    std::vector<File> files;
    while ((dir = readdir(d)) != NULL) {
        if ((uint8_t)filter == dir->d_type && strcmp(dir->d_name, ".")) {
            files.push_back(
            File {
                .name = dir->d_name,
                .filter = filter
            });
        }
    }
    if (filter == DirectoryFilterBy::Directory) {
        for (auto &dir : files) {
            dir.name += "/";
        }
    }
    closedir(d);
    return files;
}

std::vector<File> FileManager::sortFiles()
{
    auto dirs = getFiles(DirectoryFilterBy::Directory).value_or(
        std::vector {
            File {
                .name = "Failed to load directories :(",
                .filter = DirectoryFilterBy::None
            }
        }
    );
    std::sort(dirs.begin(), dirs.end());

    {
        auto files = getFiles(DirectoryFilterBy::File).value_or(
            std::vector {
                File {
                    .name = "Failed to load files",
                    .filter = DirectoryFilterBy::File
                },
            }
        );
        std::sort(files.begin(), files.end());
        dirs.insert(dirs.end(), files.begin(), files.end());
    }

    return dirs;
}