#include "OverScene.h"
#include "Constants.h"
#include "GameScene.h"

Scene* OverScene::createScene(int score) {
	auto scene = new OverScene();
	if (scene && scene->init(score)) {
		scene->autorelease();
		return scene;
	}
	delete scene;
	scene = nullptr;
	return nullptr;
}

bool OverScene::init(int score) {
	do {
		CC_BREAK_IF(!Scene::init());
		//结束场景图
		auto bg = Sprite::createWithSpriteFrameName("gameover.png");
		bg->setPosition(0, VISIBLE_ORIGIN.y + VISIBLE_SIZE.height);
		bg->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
		this->addChild(bg, BACKGROUND_ZORDER);
		//最高分标签
		auto highScore = UserDefault::getInstance()->getIntegerForKey(HIGHSCORE, 0);
		auto lblHighScore = Label::createWithBMFont("font.fnt", StringUtils::format("%d", highScore));
		lblHighScore->setPosition(VISIBLE_ORIGIN + Vec2(VISIBLE_SIZE.width / 2, VISIBLE_SIZE.height / 5 * 4));
		this->addChild(lblHighScore, 1);
		lblHighScore->setBMFontSize(68);
		//更新文件的最高分
		if (highScore < score) {
			highScore = score;
			UserDefault::getInstance()->setIntegerForKey(HIGHSCORE, highScore);
			auto seq = Sequence::create(
				ScaleTo::create(1, 5),
				CallFuncN::create([highScore](Node* node) {
				dynamic_cast<Label*>(node)->setString(StringUtils::format("%d", highScore));
			}), ScaleTo::create(1, 1), nullptr);
			lblHighScore->runAction(seq);
		}
		//当前局的分数标签
		auto lblScore = Label::createWithBMFont("font.fnt", StringUtils::format("%d", score));
		lblScore->setPosition(VISIBLE_ORIGIN + Vec2(VISIBLE_SIZE.width / 2, VISIBLE_SIZE.height / 3));
		this->addChild(lblScore, 1);
		lblScore->setBMFontSize(68);
		//回到游戏
		auto spRestart = Sprite::createWithSpriteFrameName("btn_finish.png");
		auto itemRestart = MenuItemSprite::create(spRestart, spRestart, [this](Ref *) {
			//this->onscheduleAllSelectors()
			auto scene = GameScene::CreateScene();
			Director::getInstance()->replaceScene(TransitionProgressInOut::create(1, scene));
		});
		auto menu = Menu::create(itemRestart, nullptr);
		//menu->addChild(itemRestart);
		menu->setPosition(VISIBLE_ORIGIN + Vec2(VISIBLE_SIZE.width / 2, 100));
		this->addChild(menu, UI_ZORDER);


		return true;
	} while (0);
	return false;
}