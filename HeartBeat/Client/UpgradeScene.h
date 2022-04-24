#pragma once
#include "Scene.h"

class UpgradeScene :
    public Scene
{
public:
    UpgradeScene(Client* owner);

    virtual void Enter() override;
    virtual void Exit() override;
};

