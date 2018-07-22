#pragma once

#include "cocos2d.h"

USING_NS_CC;

class LoadingScene :public Scene {
public:
	static Scene* CreateScene();
	bool init() override;
	CREATE_FUNC(LoadingScene);
};