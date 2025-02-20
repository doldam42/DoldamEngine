#pragma once

#include <filesystem>

enum FILE_ITEM_TYPE
{
    FILE_ITEM_TYPE_UNKNOWN = 0,
    FILE_ITEM_TYPE_TEXTURE,
    FILE_ITEM_TYPE_MESH,
    FILE_ITEM_TYPE_ANIMATION,
};

struct FileNode
{
    WCHAR      name[MAX_NAME];
    SORT_LINK  link;
    SORT_LINK *childrenLinkHead;
    SORT_LINK *childrenLinkTail;

    FILE_ITEM_TYPE itemType;
    bool           isDirectory;

    FileNode &operator=(const FileNode &ref)
    {
        wcscpy_s(name, MAX_NAME, ref.name);

        isDirectory = ref.isDirectory;
        itemType = ref.itemType;
        childrenLinkHead = ref.childrenLinkHead;
        childrenLinkTail = ref.childrenLinkTail;
        link.pItem = this;
        link.pPrev = ref.link.pPrev;
        link.pNext = ref.link.pNext;

        return *this;
    }

    explicit FileNode(const WCHAR *name_, bool isDirectory_)
        : isDirectory(isDirectory_), itemType(FILE_ITEM_TYPE_UNKNOWN)
    {
        wcscpy_s(name, MAX_NAME, name_);
        childrenLinkHead = nullptr;
        childrenLinkTail = nullptr;
        link.pItem = this;
        link.pPrev = nullptr;
        link.pNext = nullptr;
    }
};

class FileManager
{
    static const UINT MAX_FILE_COUNT = 512;

    WCHAR     m_basePath[MAX_PATH];

    FileNode *m_pRoot = nullptr;

    MemoryPool *m_pMemPool = nullptr;

    FileNode *InsertNodeRecursive(const std::filesystem::path &dirPath);

    void DebugPrintRecursive(FileNode* pNode, int depth);

  public:
    BOOL Initialize(WCHAR *rootDir);

    FileNode *GetRootDir() { return m_pRoot; }

    const WCHAR *GetBasePath() { return m_basePath; }

    FileManager() = default;
    ~FileManager();
};
