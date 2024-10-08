::: {#top}
::: {#titlearea}
+-----------------------------------------------------------------------+
| ::: {#projectname}                                                    |
| DoldamEngine                                                          |
| :::                                                                   |
|                                                                       |
| ::: {#projectbrief}                                                   |
| this is synopsis                                                      |
| :::                                                                   |
+-----------------------------------------------------------------------+
:::

::: {#main-nav}
:::
:::

::: {#doc-content}
::: header
::: summary
[Public Member Functions](#pub-methods) \| [List of all
members](class_i_game_engine-members.html)
:::

::: headertitle
::: title
IGameEngine Interface Reference[[abstract]{.mlabel}]{.mlabels}
:::
:::
:::

::: contents
게임에 사용하는 객체들을 생성/삭제/업데이트/렌더링 하는 엔진 본제
인터페이스 [More\...](#details)

`#include <`[`EngineModule.h`](_engine_module_8h_source.html){.el}`>`

+-----------------------------------+-----------------------------------+
| ## []{#pub-method                 |                                   |
| s} Public Member Functions {#publ |                                   |
| ic-member-functions .groupheader} |                                   |
+-----------------------------------+-----------------------------------+
| virtual BOOL                      | [Initialize](#a57f38              |
|                                   | 4bd55b9d3b663ebe99f55007a46){.el} |
|                                   | (HWND hWnd)=0                     |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **PreUpdate** (ULONGLONG          |
| 52dd41bd401fa11fa4a684cbede96a0a} | curTick)=0                        |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **Update** (ULONGLONG curTick)=0  |
| 3ea70e2237c5fed12b4459a36611d436} |                                   |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **LateUpdate** (ULONGLONG         |
| 5e0e629bc260b5527d021b0930ae3d37} | curTick)=0                        |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **Render** ()=0                   |
| c9164b4ebd55c3c2f7e2b155b8219ed7} |                                   |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **OnKeyDown** (UINT nChar, UINT   |
| 3751b6bd066264086d637b3fa9d31e90} | uiScanCode)=0                     |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| []{#a                             | **OnKeyUp** (UINT nChar, UINT     |
| f0900510b028ce40a9d5326f8af0831c} | uiScanCode)=0                     |
| virtual void                      |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [OnMouseMove](#abd48b             |
|                                   | 912c3dc763adb82f0a1ba99e00d){.el} |
|                                   | (int mouseX, int mouseY)=0        |
+-----------------------------------+-----------------------------------+
|                                   | 마우스 이동시 호출\               |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual BOOL                      | [OnUpdateWindowSize](#ab92fd      |
|                                   | 88d39a5ed2a3bb1d87b8d31e63b){.el} |
|                                   | (UINT width, UINT height)=0       |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual                           | [CreateGameObject](#a53526        |
| [IGameObjec                       | 853000cd8ce64c3c5f1bf009332){.el} |
| t](class_i_game_object.html){.el} | ()=0                              |
| \*                                |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteGameObject](#a225d9        |
|                                   | ce7a3b226465edcc7cfa7cbdb07){.el} |
|                                   | ([IGameObjec                      |
|                                   | t](class_i_game_object.html){.el} |
|                                   | \*pGameObj)=0                     |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteAllGameObject](#a95422     |
|                                   | e1d65951a37bef5ee57a077d4bd){.el} |
|                                   | ()=0                              |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual                           | [GetPrimitiveModel](#abfb10       |
| [IModel](class_i_model.html){.el} | 7b1803a05f2068c851ade3dc898){.el} |
| \*                                | (PRIMITIVE_MODEL_TYPE type)=0     |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual                           | [CreateModelFromFile](#a1c386     |
| [IModel](class_i_model.html){.el} | 123c16b495f31205a5b6e426115){.el} |
| \*                                | (const WCHAR \*basePath, const    |
|                                   | WCHAR \*filename)=0               |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteModel](#a7fa5a             |
|                                   | 6efe0358d26eedc025f94cfc0d8){.el} |
|                                   | (                                 |
|                                   | [IModel](class_i_model.html){.el} |
|                                   | \*pModel)=0                       |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteAllModel](#a17631          |
|                                   | 2e77da26de38910c19f6be33ccd){.el} |
|                                   | ()=0                              |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual                           | [CreateSpriteFromFile](#a40a71    |
| [I                                | 09cb52a8fb4a8610a46e89f6ba0){.el} |
| Sprite](class_i_sprite.html){.el} | (const WCHAR \*basePath, const    |
| \*                                | WCHAR \*filename, UINT width,     |
|                                   | UINT height)=0                    |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual                           | [CreateDynamicSprite](#aa567c     |
| [I                                | bb941c4e90bb440a125e56785a9){.el} |
| Sprite](class_i_sprite.html){.el} | (UINT width, UINT height)=0       |
| \*                                |                                   |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteSprite](#a107dd            |
|                                   | 86a44986e0231cb4fa2a12cea1a){.el} |
|                                   | ([I                               |
|                                   | Sprite](class_i_sprite.html){.el} |
|                                   | \*pSprite)=0                      |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual void                      | [DeleteAllSprite](#aa79b7         |
|                                   | f06ed621d83fccaa104d7eb3ddf){.el} |
|                                   | ()=0                              |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+
| virtual IRenderer \*              | [GetRenderer](#a5e1b4             |
|                                   | 0d3310a1f68fa2f82c7df1e4e71){.el} |
|                                   | () const =0                       |
+-----------------------------------+-----------------------------------+
|                                   |                                   |
+-----------------------------------+-----------------------------------+

[]{#details}

## Detailed Description {#detailed-description .groupheader}

::: textblock
게임에 사용하는 객체들을 생성/삭제/업데이트/렌더링 하는 엔진 본제
인터페이스
:::

## Member Function Documentation {#member-function-documentation .groupheader}

[]{#aa567cbb941c4e90bb440a125e56785a9}

## [[◆ ](#aa567cbb941c4e90bb440a125e56785a9)]{.permalink}CreateDynamicSprite() {#createdynamicsprite .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   --------------------            | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ---------------------------- ---  |                                   |
| ------ -------------------------- |                                   |
|   virtual [ISprite                |                                   |
| ](class_i_sprite.html){.el} \* IG |                                   |
| ameEngine::CreateDynamicSprite    |                                   |
| (   UINT   [*width*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|                                   |                                   |
|   UINT   [*height*]{.paramname} ) |                                   |
|   --------------------            |                                   |
| --------------------------------- |                                   |
| ---------------------------- ---  |                                   |
| ------ -------------------------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
이미지를 변경 가능한 스프라이트를 생성한다.

Parameters

:   -------- -------- ----------------------
      \[in\]   width    스프라이트 가로 크기
      \[in\]   height   스프라이트 세로 크기
      -------- -------- ----------------------

```{=html}
<!-- -->
```

Returns
:   스프라이트 객체 핸들
:::
:::

[]{#a53526853000cd8ce64c3c5f1bf009332}

## [[◆ ](#a53526853000cd8ce64c3c5f1bf009332)]{.permalink}CreateGameObject() {#creategameobject .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ----------------                | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| --------------------------------- |                                   |
| ----- --- ---------------- --- -- |                                   |
|   virtual [IGame                  |                                   |
| Object](class_i_game_object.html) |                                   |
| {.el} \* IGameEngine::CreateGameO |                                   |
| bject   (   []{.paramname}   )    |                                   |
|   ----------------                |                                   |
| --------------------------------- |                                   |
| --------------------------------- |                                   |
| ----- --- ---------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
게임 오브젝트를 생성한다.

Returns
:   게임 오브젝트 객체 핸들
:::
:::

[]{#a1c386123c16b495f31205a5b6e426115}

## [[◆ ](#a1c386123c16b495f31205a5b6e426115)]{.permalink}CreateModelFromFile() {#createmodelfromfile .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ------------------------------  | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ---------------- --- ------------ |                                   |
| ---- ---------------------------- |                                   |
|   virtual [IModel](class_i_mo     |                                   |
| del.html){.el} \* IGameEngine::Cr |                                   |
| eateModelFromFile   (   const WCH |                                   |
| AR \*   [*basePath*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|                        const WCHA |                                   |
| R \*   [*filename*]{.paramname} ) |                                   |
|   ------------------------------  |                                   |
| --------------------------------- |                                   |
| ---------------- --- ------------ |                                   |
| ---- ---------------------------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
파일로 저장된 모델 파일(메시)을 불러온다.

Parameters

:   -------- ---------- ----------------------
      \[in\]   basePath   파일의 디렉토리 경로
      \[in\]   filename   파일명
      -------- ---------- ----------------------

```{=html}
<!-- -->
```

Returns
:   모델 객체 핸들

```{=html}
<!-- -->
```

Warning
:   basePath는 /로 끝나야 된다.
:::
:::

[]{#a40a7109cb52a8fb4a8610a46e89f6ba0}

## [[◆ ](#a40a7109cb52a8fb4a8610a46e89f6ba0)]{.permalink}CreateSpriteFromFile() {#createspritefromfile .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|                                   | [[pure                            |
|  -------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| --------------------------------- |                                   |
| ----------------- --- ----------- |                                   |
| ----- --------------------------- |                                   |
|   virtual [ISprite](class_i_spri  |                                   |
| te.html){.el} \* IGameEngine::Cre |                                   |
| ateSpriteFromFile   (   const WCH |                                   |
| AR \*   [*basePath*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|                         const WCH |                                   |
| AR \*   [*filename*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|                            UINT   |                                   |
|            [*width*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|                          UINT     |                                   |
|          [*height*]{.paramname} ) |                                   |
|                                   |                                   |
|  -------------------------------- |                                   |
| --------------------------------- |                                   |
| ----------------- --- ----------- |                                   |
| ----- --------------------------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
이미지(.png/.jpg/.dds)로 부터 스프라이트를 생성한다.

Parameters

:   -------- ---------- ----------------------
      \[in\]   basePath   파일의 디렉토리 경로
      \[in\]   filename   파일명
      \[in\]   width      스프라이트 가로 크기
      \[in\]   height     스프라이트 세로 크기
      -------- ---------- ----------------------

```{=html}
<!-- -->
```

Returns
:   스프라이트 객체 핸들
:::
:::

[]{#a95422e1d65951a37bef5ee57a077d4bd}

## [[◆ ](#a95422e1d65951a37bef5ee57a077d4bd)]{.permalink}DeleteAllGameObject() {#deleteallgameobject .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ---------                       | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ----- --- ---------------- --- -- |                                   |
|   virtual                         |                                   |
|  void IGameEngine::DeleteAllGameO |                                   |
| bject   (   []{.paramname}   )    |                                   |
|   ---------                       |                                   |
| --------------------------------- |                                   |
| ----- --- ---------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
모든 게임 오브젝트를 삭제한다.
:::
:::

[]{#a176312e77da26de38910c19f6be33ccd}

## [[◆ ](#a176312e77da26de38910c19f6be33ccd)]{.permalink}DeleteAllModel() {#deleteallmodel .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ----                            | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ----- --- ---------------- --- -- |                                   |
|   vi                              |                                   |
| rtual void IGameEngine::DeleteAll |                                   |
| Model   (   []{.paramname}   )    |                                   |
|   ----                            |                                   |
| --------------------------------- |                                   |
| ----- --- ---------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
모든 모델들을 삭제한다.
:::
:::

[]{#aa79b7f06ed621d83fccaa104d7eb3ddf}

## [[◆ ](#aa79b7f06ed621d83fccaa104d7eb3ddf)]{.permalink}DeleteAllSprite() {#deleteallsprite .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   -----                           | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ----- --- ---------------- --- -- |                                   |
|   vir                             |                                   |
| tual void IGameEngine::DeleteAllS |                                   |
| prite   (   []{.paramname}   )    |                                   |
|   -----                           |                                   |
| --------------------------------- |                                   |
| ----- --- ---------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
모든 스프라이트들을 삭제한다.
:::
:::

[]{#a225d9ce7a3b226465edcc7cfa7cbdb07}

## [[◆ ](#a225d9ce7a3b226465edcc7cfa7cbdb07)]{.permalink}DeleteGameObject() {#deletegameobject .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|                                   | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| ----------- --- ----------------- |                                   |
| --------------------------------  |                                   |
| -------------------------- --- -- |                                   |
|   virtual void IGameEngine::Delet |                                   |
| eGameObject   (   [IGameObject](c |                                   |
| lass_i_game_object.html){.el} \*  |                                   |
|   [*pGameObj*]{.paramname}   )    |                                   |
|                                   |                                   |
| --------------------------------- |                                   |
| ----------- --- ----------------- |                                   |
| --------------------------------  |                                   |
| -------------------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
게임 오브젝트를 삭제한다.
:::
:::

[]{#a7fa5a6efe0358d26eedc025f94cfc0d8}

## [[◆ ](#a7fa5a6efe0358d26eedc025f94cfc0d8)]{.permalink}DeleteModel() {#deletemodel .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ---------------                 | [[pure                            |
| ------------------------ --- ---- | virtual]{.mlabel}]{.mlabels}      |
| --------------------------------- |                                   |
| - ------------------------ --- -- |                                   |
|   virtual void                    |                                   |
| IGameEngine::DeleteModel   (   [I |                                   |
| Model](class_i_model.html){.el} \ |                                   |
| *   [*pModel*]{.paramname}   )    |                                   |
|   ---------------                 |                                   |
| ------------------------ --- ---- |                                   |
| --------------------------------- |                                   |
| - ------------------------ --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
모델(메시)을 삭제한다.
:::
:::

[]{#a107dd86a44986e0231cb4fa2a12cea1a}

## [[◆ ](#a107dd86a44986e0231cb4fa2a12cea1a)]{.permalink}DeleteSprite() {#deletesprite .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   -------------------             | [[pure                            |
| --------------------- --- ------- | virtual]{.mlabel}]{.mlabels}      |
| --------------------------------- |                                   |
|  ------------------------- --- -- |                                   |
|   virtual void IGam               |                                   |
| eEngine::DeleteSprite   (   [ISpr |                                   |
| ite](class_i_sprite.html){.el} \* |                                   |
|    [*pSprite*]{.paramname}   )    |                                   |
|   -------------------             |                                   |
| --------------------- --- ------- |                                   |
| --------------------------------- |                                   |
|  ------------------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
스프라이트를 삭제한다.
:::
:::

[]{#abfb107b1803a05f2068c851ade3dc898}

## [[◆ ](#abfb107b1803a05f2068c851ade3dc898)]{.permalink}GetPrimitiveModel() {#getprimitivemodel .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   --                              | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
| --------------------------------- |                                   |
| --------- --- ------------------- |                                   |
| --- ---------------------- --- -- |                                   |
|                                   |                                   |
| virtual [IModel](class_i_model.ht |                                   |
| ml){.el} \* IGameEngine::GetPrimi |                                   |
| tiveModel   (   PRIMITIVE_MODEL_T |                                   |
| YPE   [*type*]{.paramname}   )    |                                   |
|   --                              |                                   |
| --------------------------------- |                                   |
| --------------------------------- |                                   |
| --------- --- ------------------- |                                   |
| --- ---------------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
박스/원/구 등 원시 모델(메시)을 받는다.

Returns
:   모델 객체 핸들
:::
:::

[]{#a5e1b40d3310a1f68fa2f82c7df1e4e71}

## [[◆ ](#a5e1b40d3310a1f68fa2f82c7df1e4e71)]{.permalink}GetRenderer() {#getrenderer .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   --------------                  | [[pure                            |
| --------------------------------- | virtual]{.mlabel}]{.mlabels}      |
|  --- ---------------- --- ------- |                                   |
|   virtual IRen                    |                                   |
| derer \* IGameEngine::GetRenderer |                                   |
|    (   []{.paramname}   )   const |                                   |
|   --------------                  |                                   |
| --------------------------------- |                                   |
|  --- ---------------- --- ------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc

Returns
:   렌더러 인터페이스 객체
:::
:::

[]{#a57f384bd55b9d3b663ebe99f55007a46}

## [[◆ ](#a57f384bd55b9d3b663ebe99f55007a46)]{.permalink}Initialize() {#initialize .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   -------------                   | [[pure                            |
| ------------------------- --- --- | virtual]{.mlabel}]{.mlabels}      |
| --- ---------------------- --- -- |                                   |
|   virtual BOO                     |                                   |
| L IGameEngine::Initialize   (   H |                                   |
| WND   [*hWnd*]{.paramname}   )    |                                   |
|   -------------                   |                                   |
| ------------------------- --- --- |                                   |
| --- ---------------------- --- -- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
윈도우 핸들을 받아 엔진을 초기화 한다.

Returns
:   BOOL
:::
:::

[]{#abd48b912c3dc763adb82f0a1ba99e00d}

## [[◆ ](#abd48b912c3dc763adb82f0a1ba99e00d)]{.permalink}OnMouseMove() {#onmousemove .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ----------                      | [[pure                            |
| ----------------------------- --- | virtual]{.mlabel}]{.mlabels}      |
|  ----- -------------------------- |                                   |
|   virtual                         |                                   |
|  void IGameEngine::OnMouseMove    |                                   |
| (   int   [*mouseX*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|    int   [*mouseY*]{.paramname} ) |                                   |
|   ----------                      |                                   |
| ----------------------------- --- |                                   |
|  ----- -------------------------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
마우스 이동시 호출

Parameters

:   -------- -------- -----------------------
      \[in\]   mouseX   마우스의 스크린 X좌표
      \[in\]   mouseY   마우스의 스크린 Y좌표
      -------- -------- -----------------------
:::
:::

[]{#ab92fd88d39a5ed2a3bb1d87b8d31e63b}

## [[◆ ](#ab92fd88d39a5ed2a3bb1d87b8d31e63b)]{.permalink}OnUpdateWindowSize() {#onupdatewindowsize .memtitle}

::: memitem
::: memproto
+-----------------------------------+-----------------------------------+
|   ------------------              | [[pure                            |
| ---------------------------- ---  | virtual]{.mlabel}]{.mlabels}      |
| ------ -------------------------- |                                   |
|   virtual BOOL I                  |                                   |
| GameEngine::OnUpdateWindowSize    |                                   |
| (   UINT   [*width*]{.paramname}, |                                   |
|                                   |                                   |
|                                   |                                   |
|   UINT   [*height*]{.paramname} ) |                                   |
|   ------------------              |                                   |
| ---------------------------- ---  |                                   |
| ------ -------------------------- |                                   |
+-----------------------------------+-----------------------------------+
:::

::: memdoc
창 크기가 변경되었을 때 호출

Parameters

:   -------- -------- ----------------
      \[in\]   width    변경된 창 너비
      \[in\]   height   변경된 창 폭
      -------- -------- ----------------
:::
:::

------------------------------------------------------------------------

The documentation for this interface was generated from the following
file:

-   pages/[EngineModule.h](_engine_module_8h_source.html){.el}
:::

------------------------------------------------------------------------

[Generated by [![doxygen](doxygen.svg){.footer width="104"
height="31"}](https://www.doxygen.org/index.html) 1.12.0]{.small}
:::
