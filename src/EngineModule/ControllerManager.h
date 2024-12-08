#pragma once

// ��Ʈ�ѷ��� �߰��� �����ϴ� ��찡 �����ϳ�?
// ���� �������� ������ �׳� ���� �迭�� �����ϴ°� �ո������� ������?
// Controller���� �����ϴ� ������ Controller Manager���� ����.
// Controller���� ���� �ڱ� �ڽ��� �����ϴ� ����ڸ� �־� �� ��
// �ƴ� ���� ���۷��� ī��Ʈ�� �����ؾ� �� ��
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
