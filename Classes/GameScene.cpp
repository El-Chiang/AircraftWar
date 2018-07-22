#include "GameScene.h"
#include "Constants.h"
#include "time.h"
#include "AudioEngine.h"
//#include "SimpleAudioEngine.h"
#include "OverScene.h"
using namespace experimental;

auto cnt = 0;

Scene* GameScene::CreateScene() {
	return GameScene::create();
}

bool GameScene::init() {
	//1.先调用父类的init()
	if (!Scene::init()) {
		return false;
	}

	srand((unsigned int)time(NULL));
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("shoot_background.plist");
	auto bg = Sprite::createWithSpriteFrameName("background.png");

	//添加声音文件
	AudioEngine::play2d("game_music.mp3");

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("shoot.plist");

	// 添加飞机
	auto plane = Sprite::createWithSpriteFrameName("hero1.png");
	plane->setPosition(VISIBLE_ORIGIN + VISIBLE_SIZE / 2);
	this->addChild(plane, FOREROUND_ZORDER, PLANE_TAG);
	planeHitNum = 0;

	// 创建飞机动画
	auto ani = AnimationCache::getInstance()->getAnimation(HERO_FLY_ANIMATION);
	auto animator = Animate::create(ani);
	plane->runAction(animator);

	// 触摸事件监听
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = [=](Touch* t, Event* e) {
		Vec2 touchPos = t->getLocation();
		this->m_offset = plane->getPosition() - touchPos;
		bool isContains = plane->getBoundingBox().containsPoint(touchPos);
		return isContains && !Director::getInstance()->isPaused() && !this->IsGameOver;
	};
	listener->onTouchMoved = [=](Touch* t, Event* e) {
		Vec2 touchPos = t->getLocation();
		if (this->IsGameOver) return;
		plane->setPosition(touchPos + this->m_offset);
		Vec2 deltaPos = t->getDelta();
		auto minX = plane->getContentSize().width / 2;
		auto minY = plane->getContentSize().height / 2;
		auto maxX = VISIBLE_SIZE.width - minX;
		auto maxY = VISIBLE_SIZE.height / 1.5 + minY;
		auto x = MAX(minX, MIN(maxX, plane->getPositionX()));
		auto y = MIN(maxY, plane->getPositionY());
		y = MAX(minY, y);
		plane->setPosition(x, y);
		//}			
	};
	listener->onTouchEnded = [](Touch* t, Event* e) {
	};
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, plane);
	bg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	//开启抗锯齿
	bg->getTexture()->setAliasTexParameters();
	this->addChild(bg, BACKGROUND_ZORDER, BACKGROUND_TAG_1);
	auto bg2 = Sprite::createWithSpriteFrameName("background.png");
	bg2->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	bg2->setPositionY(bg->getContentSize().height);
	bg2->getTexture()->setAliasTexParameters();
	this->addChild(bg2, BACKGROUND_ZORDER, BACKGROUND_TAG_2);

	// UI
	// 计分标签
	auto lblScore = Label::createWithBMFont("font.fnt", StringUtils::format("%d", this->m_totalScore));
	lblScore->setPosition(20, VISIBLE_SIZE.height - lblScore->getContentSize().height - 10);
	this->addChild(lblScore, UI_ZORDER, LBL_SCORE_TAG);
	lblScore->setColor(Color3B(200, 2, 100));
	lblScore->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

	// 炸弹菜单
	auto normalBomb = Sprite::createWithSpriteFrameName("bomb.png");
	auto itemBomb = MenuItemSprite::create(normalBomb, normalBomb, [=](Ref *) {
		if (Director::getInstance()->isPaused()) return;
		if (this->m_totalBombCount <= 0) return;
		AudioEngine::play2d("use_bomb.mp3", 1, 4);
		if (this->IsGameOver) return;
		for (auto enemy : this->m_enemys) {
			enemy->down();
			m_totalScore += enemy->getScore();
		}
		auto nodeScore = this->getChildByTag(LBL_SCORE_TAG);
		auto lblScore = dynamic_cast<Label *>(nodeScore);
		std::string strScore = StringUtils::format("%d", m_totalScore);
		lblScore->setString(strScore);
		m_totalBombCount--;
		this->updateBomb();
		this->m_enemys.clear();
	});

	auto lblCount = Label::createWithBMFont("font.fnt", StringUtils::format("X%d", m_totalBombCount));
	lblCount->setPosition(35 + lblCount->getContentSize().width / 2, 32);
	this->addChild(lblCount, UI_ZORDER, BOMBCOUNT_TAG);
	lblCount->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	// 暂停菜单
	auto SpPauseNormal = Sprite::createWithSpriteFrameName("game_pause_nor.png");
	auto SpPauseSelected = Sprite::createWithSpriteFrameName("game_pause_pressed.png");
	auto SpResumeNormal = Sprite::createWithSpriteFrameName("game_resume_nor.png");
	auto SpResumeSelected = Sprite::createWithSpriteFrameName("game_resume_pressed.png");
	auto itemPause = MenuItemSprite::create(SpPauseNormal, SpPauseSelected);
	auto itemResume = MenuItemSprite::create(SpResumeNormal, SpResumeSelected);

	auto toggle = MenuItemToggle::createWithCallback([this](Ref *sender) {
		AudioEngine::play2d("button.mp3");
		AudioEngine::pauseAll();
		int index = dynamic_cast<MenuItemToggle *>(sender)->getSelectedIndex();
		if (index) {
			Director::getInstance()->pause();
			this->IsPause = 1;
		}
		else {
			Director::getInstance()->resume();
			this->IsPause = 0;
		}
	}, itemPause, itemResume, nullptr);
	toggle->setPosition(VISIBLE_SIZE - toggle->getContentSize()*1.5);

	auto menu = Menu::create();
	menu->addChild(itemBomb, UI_ZORDER, ITEMBOMB_TAG);
	menu->addChild(toggle);
	menu->setPosition(itemBomb->getContentSize() / 2);
	this->addChild(menu, UI_ZORDER, MENU_TAG);

	// 生命
	for (int i = 0; i < 3; i++)
	{
		auto heart = Sprite::create("heart.png");
		heart->setPosition(Point(VISIBLE_SIZE.width * 3 / 5 + i * 36, VISIBLE_SIZE.height * 15 / 16));
		heart->setTag(HEART_TAG + i + 1);
		addChild(heart, 1);
	}

	//每帧调用update函数
	scheduleUpdate();
	//定时创建子弹
	schedule(schedule_selector(GameScene::createBullet), TIME_BREAK_1);

	// RedMode：定时创建敌机子弹 
	// schedule(schedule_selector(GameScene::createEnemyBullet), TIME_BREAK_2);

	//定时创建敌机
	schedule(schedule_selector(GameScene::createSmallEnemy), TIME_BREAK_2, CC_REPEAT_FOREVER, CREATE_SMALL_DELAY);
	schedule(schedule_selector(GameScene::createMiddleEnemy), TIME_BREAK_4, CC_REPEAT_FOREVER, CREATE_MIDDLE_DELAY);
	schedule(schedule_selector(GameScene::createBigEnemy), TIME_BREAK_5, CC_REPEAT_FOREVER, CREATE_BIG_DELAY);
	schedule(schedule_selector(GameScene::createUFO), TIME_BREAK_6, CC_REPEAT_FOREVER, CREATE_UFO_DELAY);

	// BlueMode: 定时移动敌机
	// this->schedule(schedule_selector(GameScene::moveEnemy), 0.5f, CC_REPEAT_FOREVER, 0);
	return true;
}

void GameScene::update(float delta) {
	auto bg = this->getChildByTag(1);
	auto bg2 = this->getChildByTag(2);
	auto plane = this->getChildByTag(4);

	// 背景滚动
	bg->setPositionY(bg->getPositionY() - BACKGROUND_SPEED);
	bg2->setPositionY(bg->getPositionY() + bg->getContentSize().height);
	bg2->setPositionY(bg->getPositionY() + bg->getContentSize().height);
	if (bg2->getPositionY() <= 0)
		bg->setPositionY(0);

	Vector<Sprite *> removableBullets;
	Vector<Enemy *> removableEnemys;
	Vector<Sprite *> removableEnemysBullets;
	Vector<Sprite *> removableUfos;

	// 检查子弹是否出屏幕顶部
	for (auto bullet : m_bullets)
	{
		bullet->setPositionY(bullet->getPositionY() + BULLET_SPEED);
		if (bullet->getPositionY() >= VISIBLE_SIZE.height + bullet->getContentSize().height / 2) {
			this->removeChild(bullet);
			removableBullets.pushBack(bullet);
		}
	}

	// 检查敌机子弹是否出边界
	for (auto bullet : m_enBullets)
	{
		bullet->setPositionY(bullet->getPositionY() - BULLET_SPEED);
		if (plane->getBoundingBox().intersectsRect(bullet->getBoundingBox()))
		{
			// 飞机被击中次数+1
			this->planeHitNum++;
			// 子弹消失
			removableEnemysBullets.pushBack(bullet);
			this->removeChild(bullet);
			if (planeHitNum >= 3)
			{
				AudioEngine::pauseAll();
				this->gameOver();
				this->IsGameOver = true;
			}
			// 减一条命
			auto heart = this->getChildByTag(HEART_TAG + this->planeHitNum);
			heart->removeFromParent();
		}
		if (bullet->getPositionY() <= bullet->getContentSize().height / 2) {
			this->removeChild(bullet);
			removableEnemysBullets.pushBack(bullet);
		}
	}

	// 检测敌机是否出屏幕底界
	for (auto enemy : m_enemys) {
		enemy->move();
		if (enemy->getPositionY() <= 0 - enemy->getContentSize().height / 2) {
			this->removeChild(enemy);
			removableEnemys.pushBack(enemy);
		}
	}

	// 检查道具是否出屏幕顶部或者接触到飞机
	for (auto Ufo : m_UFO)
	{
		if (Ufo->getPositionY() >= 600) {
			Ufo->setPositionY(Ufo->getPositionY() - 1);
		}
		else {
			Ufo->setPositionY(Ufo->getPositionY() - UFO_SPEED);
		}
		if (Ufo->getPositionY() <= 0 - Ufo->getContentSize().height / 2) {
			this->removeChild(Ufo);
			removableUfos.pushBack(Ufo);
		}

		// 道具碰撞检测
		if (plane->getBoundingBox().intersectsRect(Ufo->getBoundingBox())) {
			this->removeChild(Ufo);
			if (this->m_ufoType == UFO1) {  // 双子弹道具
				m_doubleBulletCount = DOUBLE_BULLET_COUNT;
			}
			if ((this->m_ufoType == UFO2) && (m_totalBombCount < 3)) {  // 炸弹道具
				m_totalBombCount++;
				this->updateBomb();
			}
			if (this->m_ufoType == UFO3 && this->planeHitNum > 0)  // 生命道具
			{
				auto heart = Sprite::create("heart.png");
				heart->setTag(HEART_TAG + planeHitNum);
				planeHitNum--;
				heart->setPosition(Point(VISIBLE_SIZE.width * 3 / 5 + planeHitNum * 36, VISIBLE_SIZE.height * 15 / 16));
				this->addChild(heart);
			}
			if (this->m_ufoType == UFO4)
			{
				schedule(schedule_selector(GameScene::createEnemyBullet), TIME_BREAK_2, 10, 0);
			}
			removableUfos.pushBack(Ufo);
		}
	}

	// 敌机、子弹的碰撞检测
	for (auto enemy : m_enemys) {
		for (auto bullet : m_bullets) {
			if (enemy->getBoundingBox().intersectsRect(bullet->getBoundingBox())) {
				enemy->hit();
				if (enemy->hit()) {
					removableEnemys.pushBack(enemy);
					// 更新得分
					m_totalScore += enemy->getScore();
					auto nodeScore = this->getChildByTag(LBL_SCORE_TAG);
					auto lblScore = dynamic_cast<Label *>(nodeScore);
					std::string strScore = StringUtils::format("%d", m_totalScore);
					lblScore->setString(strScore);
				}
				// 子弹消失
				removableBullets.pushBack(bullet);
				this->removeChild(bullet);
			}
		}
		// 飞机与敌机的碰撞
		if (enemy->getBoundingBox().intersectsRect(plane->getBoundingBox())) {
			removableEnemys.pushBack(enemy);
			enemy->down();
			this->planeHitNum++;
			if (planeHitNum >= 3)
			{
				// 爆炸动画	
				AudioEngine::pauseAll();
				// 游戏结束
				this->gameOver();
				this->IsGameOver = true;
			}
			// 减一条命
			auto heart = this->getChildByTag(HEART_TAG + this->planeHitNum);
			heart->removeFromParent();
		}
	}	

	// 移除子弹
	for (auto bullet : removableBullets) {
		m_bullets.eraseObject(bullet);
	}
	removableBullets.clear();

	for (auto bullet : removableEnemysBullets) {
		m_enBullets.eraseObject(bullet);
	}
	removableEnemysBullets.clear();

	// 移除敌机
	for (auto enemy : removableEnemys) {
		m_enemys.eraseObject(enemy);
	}
	removableEnemys.clear();

	// 移除道具
	for (auto Ufo : removableUfos) {
		m_UFO.eraseObject(Ufo);
	}
	removableUfos.clear();
}

void GameScene::createBullet(float) {
	AudioEngine::play2d("bullet.mp3");
	auto plane = this->getChildByTag(PLANE_TAG);
	if (m_doubleBulletCount > 0) {
		//创建双发子弹
		auto bullet1 = Sprite::createWithSpriteFrameName("bullet2.png");
		auto bullet2 = Sprite::createWithSpriteFrameName("bullet2.png");
		bullet1->setPosition(plane->getPositionX() - 32, plane->getPositionY() + 22);
		bullet2->setPosition(plane->getPositionX() + 32, plane->getPositionY() + 22);
		this->addChild(bullet1, FOREROUND_ZORDER);
		this->addChild(bullet2, FOREROUND_ZORDER);
		//新建的子弹加入集合
		m_bullets.pushBack(bullet1);
		m_bullets.pushBack(bullet2);
		//消耗掉双子弹的数目
		this->m_doubleBulletCount--;
	}
	else {
		auto bullet = Sprite::createWithSpriteFrameName("bullet1.png");
		bullet->setPosition(plane->getPositionX() + 2, plane->getPositionY() + plane->getContentSize().height / 2 + 10);
		this->addChild(bullet, FOREROUND_ZORDER);
		//新建的子弹加入集合
		m_bullets.pushBack(bullet);
	}

}

void GameScene::createEnemy(const EnemyType& type) {
	auto enemy = Enemy::create(type);
	auto minX = enemy->getContentSize().width / 2;
	auto maxX = VISIBLE_SIZE.width - minX;
	auto x = rand() % (int)(maxX - minX + 1) + minX;
	auto y = VISIBLE_SIZE.height + enemy->getContentSize().height / 2;
	enemy->setPosition(x, y);
	this->addChild(enemy, FOREROUND_ZORDER, ENEMY_TAG);
	m_enemys.pushBack(enemy);
}

void GameScene::createEnemyBullet(float)
{
	for (auto enemy : m_enemys)
	{
		auto bullet = Sprite::createWithSpriteFrameName("bullet1.png");
		bullet->setPosition(enemy->getPositionX() + 2, enemy->getPositionY() - enemy->getContentSize().height / 2 - 10);
		this->addChild(bullet, FOREROUND_ZORDER);
		// 新建的子弹加入集合
		m_enBullets.pushBack(bullet);
	}
}

void GameScene::createSmallEnemy(float) {
	this->createEnemy(EnemyType::SMALL_ENEMY);
}

void GameScene::createMiddleEnemy(float) {
	this->createEnemy(EnemyType::MIDDLE_ENEMY);
}

void GameScene::createBigEnemy(float) {
	this->createEnemy(EnemyType::BIG_ENEMY);
}

void GameScene::createUFO(float) {
	int r = rand() % 10;
	Sprite* Ufo;
	if (r >= 7)
	{
		Ufo = Sprite::createWithSpriteFrameName("ufo1.png");
		this->m_ufoType = UFO1;
	}
	else if (r >= 5)
	{
		Ufo = Sprite::createWithSpriteFrameName("ufo2.png");
		this->m_ufoType = UFO2;
	}
	else if (r >= 2)
	{
		Ufo = Sprite::create("heart.png");
		this->m_ufoType = UFO3;
	}
	else if (r >= 1)
	{
		Ufo = Sprite::create("enemy1_red.png");
		this->m_ufoType = UFO4;
	}
	else
	{
		Ufo = Sprite::create("enemy1_blue.png");
		this->m_ufoType = UFO5;
	}
	auto minX = Ufo->getContentSize().width / 2;
	auto maxX = VISIBLE_SIZE.width - minX;
	auto x = rand() % (int)(maxX - minX + 1) + minX;
	auto y = VISIBLE_SIZE.height + Ufo->getContentSize().height / 2;
	Ufo->setPosition(x, y);
	this->addChild(Ufo, FOREROUND_ZORDER);
	//新建的道具加入集合
	m_UFO.pushBack(Ufo);
}

void GameScene::moveEnemy(float)
{
	for (auto enemy : m_enemys)
	{
		enemy->avoidMove();
	}
}

void GameScene::updateBomb() {

	auto menu = this->getChildByTag(MENU_TAG);
	auto itemBomb = menu->getChildByTag(ITEMBOMB_TAG);//注意这里是menu获取itemBomb
	auto lblCount = this->getChildByTag(BOMBCOUNT_TAG);
	if (this->m_totalBombCount <= 0) {
		itemBomb->setVisible(false);
		lblCount->setVisible(false);
	}
	else if (this->m_totalBombCount == 1) {
		itemBomb->setVisible(true);
		lblCount->setVisible(false);
	}
	else {
		itemBomb->setVisible(true);
		lblCount->setVisible(true);
		//子类强转成父类对象
		dynamic_cast<Label *>(lblCount)->setString(StringUtils::format("X%d", this->m_totalBombCount));
	}
}

void GameScene::gameOver() {

	auto plane = this->getChildByTag(PLANE_TAG);
	for (auto node : this->getChildren()) {
		node->stopAllActions();
	}

	auto ani = AnimationCache::getInstance()->getAnimation(PLANE_DOWN);

	auto  seq = Sequence::create(Animate::create(ani), CallFunc::create([=]() {
		auto scene = OverScene::createScene(this->m_totalScore);
		Director::getInstance()->replaceScene(scene);
	}), nullptr);
	plane->runAction(seq);
	this->unscheduleAllCallbacks();
}