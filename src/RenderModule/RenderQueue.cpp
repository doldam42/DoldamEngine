#include "pch.h"

#include "CommandListPool.h"
#include "D3D12Renderer.h"
#include "D3DMeshObject.h"
#include "RaytracingManager.h"
#include "SpriteObject.h"

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
                          DWORD processCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv,
                          D3D12_CPU_DESCRIPTOR_HANDLE dsv, D3D12_GPU_DESCRIPTOR_HANDLE global,
                          const D3D12_VIEWPORT *pViewport, const D3D12_RECT *pScissorRect, UINT rtvCount,
                          DRAW_PASS_TYPE passType)
{
    ID3D12Device5     *pD3DDevice = m_pRenderer->GetD3DDevice();
    RaytracingManager *pDXRSceneManager = m_pRenderer->GetRaytracingManager();

    ID3D12GraphicsCommandList *ppCommandList[64] = {};
    UINT                       commandListCount = 0;

    ID3D12GraphicsCommandList *pCommandList = nullptr;
    UINT                       processedCount = 0;
    UINT                       processedCountPerCommandList = 0;
    const RENDER_ITEM         *pItem = nullptr;
    while (pItem = Dispatch())
    {
        pCommandList = pCommandListPool->GetCurrentCommandList();
        pCommandList->RSSetViewports(1, pViewport);
        pCommandList->RSSetScissorRects(1, pScissorRect);
        pCommandList->OMSetRenderTargets(rtvCount, &rtv, FALSE, &dsv);

        switch (pItem->type)
        {
        case RENDER_ITEM_TYPE_MESH_OBJ: {
            D3DMeshObject *pMeshObj = (D3DMeshObject *)pItem->pObjHandle;
            pMeshObj->Draw(threadIndex, pCommandList, &pItem->meshObjParam.worldTM, pItem->meshObjParam.ppMaterials,
                           pItem->meshObjParam.numMaterials, Graphics::GetRS(pItem->type, passType),
                           Graphics::GetPSO(pItem->type, passType, pItem->meshObjParam.fillMode), global, nullptr, 0,
                           passType, 1);
            if (passType == DRAW_PASS_TYPE_DEFAULT)
            {
                pMeshObj->RenderNormal(threadIndex, pCommandList, &pItem->meshObjParam.worldTM,
                                       nullptr, 0, FILL_MODE_SOLID, 1);
            }
        }
        break;
        case RENDER_ITEM_TYPE_CHAR_OBJ: {
            D3DMeshObject *pMeshObj = (D3DMeshObject *)pItem->pObjHandle;
            pMeshObj->Draw(threadIndex, pCommandList, &pItem->charObjParam.worldTM, pItem->meshObjParam.ppMaterials,
                           pItem->meshObjParam.numMaterials, Graphics::GetRS(pItem->type, passType),
                           Graphics::GetPSO(pItem->type, passType, pItem->meshObjParam.fillMode), global,
                           pItem->charObjParam.pBones, pItem->charObjParam.numBones, passType, 1);
            if (passType == DRAW_PASS_TYPE_DEFAULT)
            {
                pMeshObj->RenderNormal(threadIndex, pCommandList, &pItem->charObjParam.worldTM,
                                       pItem->charObjParam.pBones, pItem->charObjParam.numBones, FILL_MODE_SOLID, 1);
            }
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
                SpriteObject *pSpriteObj = (SpriteObject *)pItem->pObjHandle;
                Vector2       Pos = {(float)pItem->spriteParam.posX, (float)pItem->spriteParam.posY};
                Vector2       Scale = {pItem->spriteParam.scaleX, pItem->spriteParam.scaleY};

                pSpriteObj->Draw(threadIndex, pCommandList, &Pos, &Scale, Z);
            }
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
