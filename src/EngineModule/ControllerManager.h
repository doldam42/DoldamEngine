#pragma once

// 컨트롤러를 중간에 삭제하는 경우가 존재하나?
// 만약 존재하지 않으면 그냥 동적 배열로 관리하는게 합리적이지 않을까?
// Controller들을 삭제하는 권한은 Controller Manager에게 없음.
// Controller들은 따로 자기 자신을 삭제하는 담당자를 둬야 할 듯
// 아님 추후 레퍼런스 카운트를 도입해야 할 듯
class ControllerManager
{
    static const UINT DEFAULT_SCRIPT_COUNT = 64;

    std::vector<IController *> m_controllers;
    /*SORT_LINK *m_pControllerLinkHead = nullptr;
    SORT_LINK *m_pControllerLinkTail = nullptr;*/

  public:
    void RegisterController(IController *pController);

    void UpdateControllers(float dt);
    void StartControllers();

    ControllerManager() { m_controllers.reserve(DEFAULT_SCRIPT_COUNT); };
    ~ControllerManager();
};
