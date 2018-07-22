#include "LoadingScene.h"
#include "GameScene.h"
#include "Constants.h"
#include "Enemy.h"
#include "AudioEngine.h"
using namespace experimental;

Scene* LoadingScene::CreateScene() {
	return LoadingScene::create();
}

bool LoadingScene::init() {
	do {
		CC_BREAK_IF(!Scene::init());
		//通过加载精灵帧文件，将精灵缓存到精灵集文件；
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("shoot_background.plist");
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("shoot.plist");
		auto bg = Sprite::createWithSpriteFrameName("background.png");
		bg->setPosition(VISIBLE_ORIGIN + VISIBLE_SIZE / 2);
		this->addChild(bg, -1, 1);

		auto title = Sprite::createWithSpriteFrameName("shoot_copyright.png");
		title->setPosition(VISIBLE_ORIGIN + VISIBLE_SIZE / 2 + Vec2(0, 100));
		this->addChild(title, 1, 2);

		auto loading = Sprite::create();
		loading->setPosition(VISIBLE_ORIGIN + VISIBLE_SIZE / 2 + Vec2(0, -200));
		this->addChild(loading);

		auto ani = Animation::create();
		for (auto i = 0; i < 4; i++) {
			auto png = StringUtils::format("game_loading%d.png", i + 1);
			ani->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName(png));
		}
		ani->setDelayPerUnit(0.2f);
		ani->setLoops(4);
		auto seq = Sequence::create(Animate::create(ani), CallFunc::create([]() {
			//场景跳转
			auto scene = GameScene::CreateScene();
			Director::getInstance()->replaceScene(TransitionProgressInOut::create(1, scene));
		}), nullptr);
		loading->runAction(seq);

		//创建飞机动画并放入缓存

		//	a.创建动画对象
		auto aniHeroFly = Animation::create();
		//  b.添加动画帧,从精灵帧缓存中查找出名字
		aniHeroFly->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("hero2.png"));
		aniHeroFly->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("hero1.png"));
		//  c.设置动画切换时长
		aniHeroFly->setDelayPerUnit(TIME_BREAK_1);
		aniHeroFly->setLoops(-1);
		AnimationCache::getInstance()->addAnimation(aniHeroFly, HERO_FLY_ANIMATION);

		//大敌机飞行动画
		auto aniBigEnemyFly = Animation::create();
		aniBigEnemyFly->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n1.png"));
		aniBigEnemyFly->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n2.png"));
		aniBigEnemyFly->setDelayPerUnit(TIME_BREAK_1);
		aniBigEnemyFly->setLoops(-1);
		AnimationCache::getInstance()->addAnimation(aniBigEnemyFly, BIG_ENEMY_FLY_ANIMATION);

		//创建敌机毁灭动画放入缓存
		//小敌机坠毁
		auto aniSmallEnemyDown = Animation::create();
		for (int i = 0; i < 4; i++) {
			auto frameName = StringUtils::format("enemy1_down%d.png", i + 1);
			aniSmallEnemyDown->addSpriteFrame(CCSpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
		}
		aniSmallEnemyDown->setDelayPerUnit(TIME_BREAK_3);
		aniSmallEnemyDown->setLoops(1);
		AnimationCache::getInstance()->addAnimation(aniSmallEnemyDown, SMALL_ENEMY_DOWN_ANIMATION);
		//中敌机坠毁
		auto aniMiddleEnemyDown = Animation::create();
		for (int i = 0; i < 4; i++) {
			auto frameName = StringUtils::format("enemy2_down%d.png", i + 1);
			aniMiddleEnemyDown->addSpriteFrame(CCSpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
		}
		aniMiddleEnemyDown->setDelayPerUnit(TIME_BREAK_3);
		aniMiddleEnemyDown->setLoops(1);
		AnimationCache::getInstance()->addAnimation(aniMiddleEnemyDown, MIDDLE_ENEMY_DOWN_ANIMATION);
		//大敌机坠毁
		auto aniBigEnemyDown = Animation::create();
		for (int i = 0; i < 6; i++) {
			auto frameName = StringUtils::format("enemy3_down%d.png", i + 1);
			aniBigEnemyDown->addSpriteFrame(CCSpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
		}
		aniBigEnemyDown->setDelayPerUnit(TIME_BREAK_3);
		aniBigEnemyDown->setLoops(1);
		AnimationCache::getInstance()->addAnimation(aniBigEnemyDown, BIG_ENEMY_DOWN_ANIMATION);
		//中敌机受伤
		auto aniMiddleEnemyHit = Animation::create();
		aniMiddleEnemyHit->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2_hit.png"));
		aniMiddleEnemyHit->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy2.png"));
		aniMiddleEnemyHit->setDelayPerUnit(TIME_BREAK_3);
		aniMiddleEnemyHit->setLoops(1);
		AnimationCache::getInstance()->addAnimation(aniMiddleEnemyHit, MIDDLE_ENEMY_HIT_ANIMATION);
		//大敌机受伤
		auto aniBigEnemyHit = Animation::create();
		aniBigEnemyHit->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_hit.png"));
		aniBigEnemyHit->addSpriteFrame(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n1.png"));
		aniBigEnemyHit->setDelayPerUnit(0.2f);
		aniBigEnemyHit->setLoops(1);
		AnimationCache::getInstance()->addAnimation(aniBigEnemyHit, BIG_ENEMY_HIT_ANIMATION);

		//
		Vector<SpriteFrame*> frames;
		for (int i = 0; i < 4; i++) {
			auto frameName = StringUtils::format("hero_blowup_n%d.png", i + 1);
			frames.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName));
		}
		auto aniPlaneDown = Animation::createWithSpriteFrames(frames, 0.2f);
		AnimationCache::getInstance()->addAnimation(aniPlaneDown, PLANE_DOWN);

		//预加载声音文件
		AudioEngine::preload("achievement.mp3");
		AudioEngine::preload("game_music.mp3");
		AudioEngine::preload("big_spaceship_flying.mp3");
		AudioEngine::preload("bullet.mp3");
		AudioEngine::preload("button.mp3");
		AudioEngine::preload("enemy1_down.mp3");
		AudioEngine::preload("enemy2_down.mp3");
		AudioEngine::preload("enemy3_down.mp3");
		AudioEngine::preload("game_over.mp3");
		AudioEngine::preload("get_bomb.mp3");
		AudioEngine::preload("get_double_laser.mp3");
		AudioEngine::preload("out_porp.mp3");
		AudioEngine::preload("use_bomb.mp3");

		return true;
	} while (0);
	return false;
}

