#include "pch.h"

#include <filesystem>

#include "FileManager.h"

namespace fs = std::filesystem;

inline FILE_ITEM_TYPE GetItemType(const fs::path &filePath)
{
    WCHAR ext[MAX_PATH] = {L'\0'};
    if (!TryGetExtension(filePath.c_str(), ext))
        return FILE_ITEM_TYPE_UNKNOWN;

    if (!wcscmp(ext, L".png") || !wcscmp(ext, L".jpg") || !wcscmp(ext, L".dds") || !wcscmp(ext, L".bmp") ||
        !wcscmp(ext, L".hdr"))
        return FILE_ITEM_TYPE_TEXTURE;

    if (!wcscmp(ext, L".dom") || !wcscmp(ext, L".fbx") || !wcscmp(ext, L".gltf"))
        return FILE_ITEM_TYPE_MESH;

    if (!wcscmp(ext, L".dca"))
        return FILE_ITEM_TYPE_ANIMATION;

    return FILE_ITEM_TYPE_UNKNOWN;
}

FileNode *FileManager::InsertNodeRecursive(const fs::path &dirPath)
{
    if (!fs::exists(dirPath))
    {
        return nullptr;
    }

    bool isDirectory = fs::is_directory(dirPath);

    FileNode *pNode = (FileNode *)m_pMemPool->Alloc();
    *pNode = FileNode(dirPath.filename().c_str(), isDirectory);

    //FileNode *pNode = pNode = new FileNode(dirPath.filename().c_str(), isDirectory);
    if (!isDirectory)
    {
        pNode->itemType = GetItemType(dirPath);
    }
    else
    {
        for (const auto &entry : fs::directory_iterator(dirPath))
        {
            FileNode *child = InsertNodeRecursive(entry.path());
            LinkToLinkedListFIFO(&pNode->childrenLinkHead, &pNode->childrenLinkTail, &child->link);
        }
    }

    return pNode;
}

void FileManager::DebugPrintRecursive(FileNode *pNode, int depth)
{ 
    WCHAR buff[MAX_PATH];
    for (int i = 0; i < depth; i++)
    {
        buff[i] = L'-';
    }
    buff[depth] = L' ';
    wsprintf(buff + depth + 1, L"%s\n", pNode->name);
    OutputDebugString(buff);

    SORT_LINK *pCur = pNode->childrenLinkHead;
    while (pCur)
    {
        FileNode *pChild = (FileNode *)pCur->pItem;

        DebugPrintRecursive(pChild, depth + 1); 

        pCur = pCur->pNext;
    }
}

BOOL FileManager::Initialize(WCHAR *rootDir)
{
    m_pMemPool = new MemoryPool();
    m_pMemPool->Initialize(sizeof(FileNode), MAX_FILE_COUNT);

    fs::path p(rootDir);
    m_pRoot = InsertNodeRecursive(p);

    wcscpy_s(m_basePath, MAX_PATH, p.parent_path().c_str());
    //DebugPrintRecursive(m_pRoot, 0);
    return TRUE;
}

FileManager::~FileManager() 
{ 
    if (m_pMemPool)
    {
        delete m_pMemPool;
        m_pMemPool = nullptr;
    }
}
