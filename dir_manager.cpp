#include "dir_manager.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace {

// Split a path into (dirName, rest). dirName is the last non-empty
// path component; for "C:\data\train1" -> "train1", "/data/train1/" -> "train1".
std::string lastComponent(const std::string& path)
{
    std::string p = path;
    while (!p.empty() && (p.back() == '/' || p.back() == '\\'))
        p.pop_back();
    if (p.empty()) return p;

    size_t pos = p.find_last_of("/\\");
    if (pos == std::string::npos) return p;
    return p.substr(pos + 1);
}

} // namespace

std::string DirManager::normalize(const std::string& path)
{
    std::string p = path;
    while (!p.empty() && (p.back() == '/' || p.back() == '\\'))
        p.pop_back();
    return p;
}

// Compare two strings with numeric awareness (like QCollator numericMode):
// digit runs compare by numeric value, letter runs case-insensitively.
// So "train2" < "train10", "b2" < "b10" < "b2a".
static int numericCompare(const std::string& a, const std::string& b)
{
    size_t i = 0, j = 0;
    const size_t na = a.size(), nb = b.size();

    while (i < na && j < nb) {
        char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(a[i])));
        char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(b[j])));

        if (std::isdigit(static_cast<unsigned char>(ca)) &&
            std::isdigit(static_cast<unsigned char>(cb))) {
            // Extract both digit runs and compare numerically
            size_t ia = i, jb = j;
            while (ia < na && std::isdigit(static_cast<unsigned char>(a[ia]))) ++ia;
            while (jb < nb && std::isdigit(static_cast<unsigned char>(b[jb]))) ++jb;

            // Strip leading zeros to compare values
            size_t za = i, zb = j;
            while (za < ia && a[za] == '0') ++za;
            while (zb < jb && b[zb] == '0') ++zb;
            size_t lenA = ia - za, lenB = jb - zb;

            if (lenA != lenB)
                return lenA < lenB ? -1 : 1;
            for (size_t k = 0; k < lenA; ++k) {
                if (a[za + k] != b[zb + k])
                    return a[za + k] < b[zb + k] ? -1 : 1;
            }
            i = ia; j = jb;
        } else if (std::isdigit(static_cast<unsigned char>(ca))) {
            // a has digits, b has letter: digits sort before letters
            return -1;
        } else if (std::isdigit(static_cast<unsigned char>(cb))) {
            return 1;
        } else {
            if (ca != cb)
                return ca < cb ? -1 : 1;
            ++i; ++j;
        }
    }

    // One string is a prefix of the other
    if (i < na) return 1;
    if (j < nb) return -1;
    return 0;
}

bool DirManager::lessThan(const std::string& a, const std::string& b)
{
    const std::string na = lastComponent(a);
    const std::string nb = lastComponent(b);
    const int c = numericCompare(na, nb);
    if (c != 0) return c < 0;
    // Tie-break on the full normalized path for stable, deterministic order
    return a < b;
}

int DirManager::indexOf(const std::string& path) const
{
    const std::string n = normalize(path);
    for (int i = 0; i < static_cast<int>(m_dirs.size()); ++i)
        if (m_dirs[i].path == n)
            return i;
    return -1;
}

int DirManager::addDir(const std::string& path)
{
    std::string n = normalize(path);
    if (n.empty()) return -1;

    const int existing = indexOf(n);
    if (existing >= 0) {
        if (m_current < 0) m_current = existing;
        return existing;
    }

    // Remember the current path before re-sorting (index shifts after insert)
    const std::string currentPathBefore = (m_current >= 0) ? m_dirs[m_current].path : std::string();

    m_dirs.push_back(DirEntry{ n, 0, std::string() });

    // Keep the list sorted numerically (lessThan is static)
    std::stable_sort(m_dirs.begin(), m_dirs.end(),
                     [](const DirEntry& a, const DirEntry& b) {
                         return DirManager::lessThan(a.path, b.path);
                     });

    // Re-locate both the new entry and the current entry by path
    int inserted = -1;
    for (int i = 0; i < static_cast<int>(m_dirs.size()); ++i)
        if (m_dirs[i].path == n) inserted = i;

    if (m_current < 0) {
        m_current = inserted;
    } else {
        m_current = indexOf(currentPathBefore);
    }
    return inserted;
}

bool DirManager::removeDir(const std::string& path)
{
    const int idx = indexOf(path);
    if (idx < 0) return false;

    m_dirs.erase(m_dirs.begin() + idx);

    if (m_dirs.empty()) {
        m_current = -1;
        return true;
    }

    if (m_current > idx)      --m_current;
    else if (m_current == idx) m_current = std::min(idx, static_cast<int>(m_dirs.size()) - 1);

    return true;
}

const DirEntry* DirManager::entryAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_dirs.size()))
        return nullptr;
    return &m_dirs[index];
}

const std::string& DirManager::currentPath() const
{
    static const std::string empty;
    const DirEntry* e = currentEntry();
    return e ? e->path : empty;
}

bool DirManager::switchToDir(int index)
{
    if (m_dirs.empty()) return false;

    const int n = static_cast<int>(m_dirs.size());
    index = ((index % n) + n) % n;   // wrap negatives / overflow

    if (index == m_current) return false;
    m_current = index;
    return true;
}

int DirManager::nextDir() const
{
    if (m_dirs.size() < 2) return -1;
    const int n = static_cast<int>(m_dirs.size());
    return (m_current + 1) % n;
}

int DirManager::prevDir() const
{
    if (m_dirs.size() < 2) return -1;
    const int n = static_cast<int>(m_dirs.size());
    return (m_current - 1 + n) % n;
}

void DirManager::setProgress(const std::string& dirPath, int index)
{
    const int idx = indexOf(dirPath);
    if (idx >= 0) m_dirs[idx].progress = index;
}

int DirManager::progressOf(const std::string& dirPath) const
{
    const int idx = indexOf(dirPath);
    return idx >= 0 ? m_dirs[idx].progress : 0;
}

void DirManager::setClassesFile(const std::string& dirPath, const std::string& file)
{
    const int idx = indexOf(dirPath);
    if (idx >= 0) m_dirs[idx].classesFile = file;
}

std::string DirManager::classesFileOf(const std::string& dirPath) const
{
    const int idx = indexOf(dirPath);
    return idx >= 0 ? m_dirs[idx].classesFile : std::string();
}

void DirManager::restore(const std::vector<DirEntry>& dirs, int currentIndex)
{
    m_dirs.clear();
    m_current = -1;

    // Re-add through addDir() so dedup + sorting + current tracking apply
    for (const DirEntry& d : dirs)
        if (!d.path.empty()) {
            const int i = addDir(d.path);
            if (i >= 0) {
                m_dirs[i].progress    = d.progress;
                m_dirs[i].classesFile = d.classesFile;
            }
        }

    if (!m_dirs.empty()) {
        m_current = (currentIndex >= 0 && currentIndex < static_cast<int>(m_dirs.size()))
                        ? currentIndex
                        : 0;
    }
}
