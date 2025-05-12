#include "pch.h"

#include "CommandListPool.h"
#include "D3D12Renderer.h"
#include "RaytracingMeshObject.h"
#include "RaytracingManager.h"
#include "SpriteObject.h"
#include "Terrain.h"

#include "RenderQueue.h"`

const RENDER_ITEM *RenderQueue::Dispatch()
{
    const RENDER_ITEM *pItem = nullptr;
    if (m_readBufferPos + sizeof(RENDER_ITEM) > m_allocatedSize)
        goto lb_return;

    pItem = (const RENDER_ITEM *)(m_pBuffer + m_readBufferPos);
    m_readBufferPos += sizeof(RENDER_ITEM);

lb_return:
    return pItem;
}

void RenderQueue::Cleanup()
{
    if (m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = nullptr;
    }
}

BOOL RenderQueue::Initialize(D3D12Renderer *pRenderer, UINT maxItemNum)
{
    m_pRenderer = pRenderer;
    m_maxBufferSize = sizeof(RENDER_ITEM) * maxItemNum;
    m_pBuffer = (char *)malloc(m_maxBufferSize);

    memset(m_pBuffer, 0, m_maxBufferSize);

    return TRUE;
}

BOOL RenderQueue::Add(const RENDER_ITEM *pItem)
{
    BOOL result = FALSE;

    if (m_allocatedSize + sizeof(RENDER_ITEM) > m_maxBufferSize)
    {
        goto lb_return;
    }

    char *pDest = m_pBuffer + m_allocatedSize;
    memcpy(pDest, pItem, sizeof(RENDER_ITEM));
    m_allocatedSize += sizeof(RENDER_ITEM);
    m_itemCount++;
    result = TRUE;

lb_return:
    return result;
}

UINT RenderQueue::Process(UINT threadIndex, CommandListPool *pCommandListPool, ID3D12CommandQueue *pCommandQueue,
                          DWORD processCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE *rtvs,
                          D3D12_CPU_DESCRIPTOR_HANDLE dsv,
                          const D3D12_VIEWPORT *pViewport, const D3D12_RECT *pScissorRect, UINT rtvCount,
                          DRAW_PASS_TYPE passType)
{
    ID3D12Device5     *pD3DDevice = m_pRenderer->GetD3DDevice();

    ID3D12GraphicsCommandList4 *ppCommandList[64] = {};
    UINT                       commandListCount = 0;

    ID3D12GraphicsCommandList4 *pCommandList = nullptr;
    UINT                       processedCount = 0;
    UINT                       processedCountPerCommandList = 0;
    const RENDER_ITEM         *pItem = nullptr;
    while (pItem = Dispatch())
    {
        pCommandList = pCommandListPool->GetCurrentCommandList();
        pCommandList->RSSetViewports(1, pViewport);
        pCommandList->RSSetScissorRects(1, pScissorRect);
        pCommandList->OMSetRenderTargets(rtvCount, rtvs, FALSE, &dsv);
        
        if (passType == DRAW_PASS_TYPE_TRANSPARENCY)
        {
            pCommandList->SetGraphicsRootDescriptorTable(4, m_pRenderer->GetOITDescriptorHandle(threadIndex));
        }

        switch (pItem->type)
        {
        case RENDER_ITEM_TYPE_MESH_OBJ: {
            RaytracingMeshObject *pMeshObj = (RaytracingMeshObject *)pItem->pObjHandle;
            pMeshObj->DrawDeferred(threadIndex, pCommandList, &pItem->meshObjParam.worldTM,
                                   pItem->meshObjParam.ppMaterials, pItem->meshObjParam.numMaterials, passType,
                                   pItem->fillMode, nullptr, 0);
        }
        break;
        case RENDER_ITEM_TYPE_CHAR_OBJ: {
            RaytracingMeshObject *pMeshObj = (RaytracingMeshObject *)pItem->pObjHandle;
            pMeshObj->DrawDeferred(threadIndex, pCommandList, &pItem->charObjParam.worldTM,
                                   pItem->charObjParam.ppMaterials, pItem->charObjParam.numMaterials, passType,
                                   pItem->fillMode, pItem->charObjParam.pBones, pItem->charObjParam.numBones);
        }
        break;
        case RENDER_ITEM_TYPE_SPRITE: {
            SpriteObject   *pSpriteObj = (SpriteObject *)pItem->pObjHandle;
            TEXTURE_HANDLE *pTextureHandle = (TEXTURE_HANDLE *)pItem->spriteParam.pTexHandle;
            float           Z = pItem->spriteParam.Z;
            if (pTextureHandle)
            {
                Vector2 Pos = {(float)pItem->spriteParam.posX, (float)pItem->spriteParam.posY};
                Vector2 Scale = {pItem->spriteParam.scaleX, pItem->spriteParam.scaleY};

                const RECT *pRect = nullptr;
                if (pItem->spriteParam.isUseRect)
                {
                    pRect = &pItem->spriteParam.rect;
                }
                pSpriteObj->DrawWithTex(threadIndex, pCommandList, &Pos, &Scale, pRect, Z, pTextureHandle);
            }
            else
            {
                Vector2       Pos = {(float)pItem->spriteParam.posX, (float)pItem->spriteParam.posY};
                Vector2       Scale = {pItem->spriteParam.scaleX, pItem->spriteParam.scaleY};

                pSpriteObj->Draw(threadIndex, pCommandList, &Pos, &Scale, Z);
            }
        }
        break;
        case RENDER_ITEM_TYPE_TERRAIN:
        {
            Terrain *pTerrain = (Terrain *)pItem->pObjHandle;
            pTerrain->Draw(threadIndex, pCommandList, passType, &pItem->terrainParam.scale,
                           pItem->terrainParam.fillMode);
        }
        break;
        default:
            __debugbreak();
            break;
        }
        processedCount++;
        processedCountPerCommandList++;
        if (processedCountPerCommandList > processCountPerCommandList)
        {
            pCommandListPool->Close();
            ppCommandList[commandListCount] = pCommandList;
            commandListCount++;
            pCommandList = nullptr;
            processedCountPerCommandList = 0;
        }
    }

    if (processedCountPerCommandList)
    {
        pCommandListPool->Close();
        ppCommandList[commandListCount] = pCommandList;
        commandListCount++;
        pCommandList = nullptr;
        processedCountPerCommandList = 0;
    }
    if (commandListCount)
    {
        pCommandQueue->ExecuteCommandLists(commandListCount, (ID3D12CommandList **)ppCommandList);
    }
    m_itemCount = 0;
    return processedCount;
}

void RenderQueue::Reset()
{
    m_allocatedSize = 0;
    m_readBufferPos = 0;
}

void RenderQueue::Revert() { m_readBufferPos = 0; }

RenderQueue::~RenderQueue() { Cleanup(); }
