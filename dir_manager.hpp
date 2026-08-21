#ifndef DIR_MANAGER_HPP
#define DIR_MANAGER_HPP

#include <string>
#include <vector>
#include <map>

// Pure-C++ (Qt-free) multi-directory bookkeeping for YoloLabel.
//
// Keeps an ordered list of image directories, per-directory browsing
// progress, and per-directory class-file bindings. The Qt layer
// (MainWindow) owns the actual file loading / UI; this class only
// tracks state, so it can be unit-tested without a Qt toolchain.

struct DirEntry {
    std::string path;              // absolute directory path
    int         progress = 0;      // last-viewed image index in this dir
    std::string classesFile;       // bound class file ("" = inherit global)
};

class DirManager
{
public:
    DirManager() = default;

    // Append a directory. Duplicates (case-insensitive, normalized) are
    // ignored. The list is kept sorted numerically by directory name
    // (e.g. "train2" < "train10"). Returns the index of the entry
    // (existing or new), or -1 if the path is empty.
    int addDir(const std::string& path);

    // Remove a directory by path. If the current dir is removed, the
    // current index falls back to the nearest remaining entry.
    // Returns true if an entry was removed.
    bool removeDir(const std::string& path);

    // Switch to the directory at [0, size()). Out-of-range indices
    // wrap around (cyclic navigation). Returns false if the list is
    // empty or the index is already current.
    bool switchToDir(int index);

    // Cyclic navigation helpers: current +/- 1, wrapping around.
    int  nextDir() const;   // -1 if fewer than 2 dirs
    int  prevDir() const;   // -1 if fewer than 2 dirs

    int  size() const { return static_cast<int>(m_dirs.size()); }
    bool isEmpty() const { return m_dirs.empty(); }

    int  currentIndex() const { return m_current; }
    void setCurrentIndex(int index) { m_current = index; }

    const DirEntry* entryAt(int index) const;
    const DirEntry* currentEntry() const { return entryAt(m_current); }
    const std::string& currentPath() const;

    void setProgress(const std::string& dirPath, int index);
    int  progressOf(const std::string& dirPath) const;

    void setClassesFile(const std::string& dirPath, const std::string& file);
    std::string classesFileOf(const std::string& dirPath) const;

    // Snapshot / restore of the whole state (for QSettings persistence).
    std::vector<DirEntry> snapshot() const { return m_dirs; }
    void restore(const std::vector<DirEntry>& dirs, int currentIndex);

private:
    static std::string normalize(const std::string& path);
    static bool lessThan(const std::string& a, const std::string& b);
    int  indexOf(const std::string& path) const;   // -1 if not found

    std::vector<DirEntry> m_dirs;
    int m_current = -1;
};

#endif // DIR_MANAGER_HPP
