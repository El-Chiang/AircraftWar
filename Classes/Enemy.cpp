#include "Enemy.h"

#include "Constants.h"

#include "AudioEngine.h"

using namespace experimental;


bool Enemy::initWithEnemyType(const EnemyType& type) {
	//这里不要调用成了createWithSpriteFrameName
	//用成员变量保存类型，记住之后后续可以随时访问
	m_type = type;
	std::string frameName = "";
	switch (m_type)
	{
	case EnemyType::SMALL_ENEMY:
		frameName = "enemy1.png";
		this->m_speed = -SMALL_ENEMY_SPEED;
		this->m_hp = 1;
		this->m_score = 100;
		break;
	case EnemyType::MIDDLE_ENEMY:
		frameName = "enemy2.png";
		this->m_speed = -MIDDLE_ENEMY_SPEED;
		this->m_hp = 15;
		this->m_score = 800;
		break;
	case EnemyType::BIG_ENEMY:
		frameName = "enemy3_n1.png";
		this->m_speed = -BIG_ENEMY_SPEED;
		this->m_hp = 30;
		this->m_score = 1000;
		break;
	default:
		break;
	}
	if (!Sprite::initWithSpriteFrameName(frameName)) {
		return false;
	}

	//大敌机的飞行动画
	if (this->m_type == EnemyType::BIG_ENEMY) {
		auto ani = AnimationCache::getInstance()->getAnimation(BIG_ENEMY_FLY_ANIMATION);
		//2.将动画封装为动作
		auto animator = Animate::create(ani);
		//3.精灵运行动作
		this->stopAllActions();//在执行非循环，先停止其他动作
		this->runAction(animator);
	}
	return true;
}

Enemy*  Enemy::create(const EnemyType& type) {
	auto enemy = new Enemy();
	if (enemy && enemy->initWithEnemyType(type)) {
		enemy->autorelease();  // 放入自动计数器
		return enemy;
	}
	delete enemy;
	enemy = nullptr;
	return nullptr;
}

void Enemy::down() {
	auto ani = AnimationCache::getInstance()->getAnimation(SMALL_ENEMY_DOWN_ANIMATION);
	switch (m_type)
	{
	case EnemyType::SMALL_ENEMY:
		AudioEngine::play2d("enemy1_down.mp3", 1, 2);
		break;
	case EnemyType::MIDDLE_ENEMY:
		ani = AnimationCache::getInstance()->getAnimation(MIDDLE_ENEMY_DOWN_ANIMATION);
		AudioEngine::play2d("enemy2_down.mp3", 1, 2);
		break;
	case EnemyType::BIG_ENEMY:
		ani = AnimationCache::getInstance()->getAnimation(BIG_ENEMY_DOWN_ANIMATION);
		AudioEngine::play2d("enemy3_down.mp3", 1, 2);
		break;
	default:
		break;
	}

	//2.将动画封装为动作
	auto animator = Animate::create(ani);
	//动作函数序列
	/*auto seq = Sequence::create(animator, CallFuncN::create([this](Node *node) {
	this->removeChild(node);
	}), nullptr);

	*/
	auto seq = Sequence::create(animator, RemoveSelf::create(), NULL);
	this->runAction(seq);
	//3.精灵运行动作
	this->runAction(animator);
}

void Enemy::move() {
	this->setPosition(this->getPositionX(), this->getPositionY() + this->m_speed);
}

void Enemy::avoidMove()
{
	int direction = (rand() % 2) ? 1 : 0;
	auto move1 = MoveBy::create(1.5, Vec2(VISIBLE_SIZE.width - this->getPositionX() - 64, -80));  // 向右
	auto move2 = MoveBy::create(1.5, Vec2(-this->getPositionX() + 64, -80));  // 向左
	if (this->getPositionX() >= VISIBLE_SIZE.width - this->getContentSize().width)
		this->runAction(Sequence::create(move2, move1, nullptr));
	else if (this->getPositionX() <= this->getContentSize().width)
		this->runAction(Sequence::create(move1, move2, nullptr));
	else
		rand() % 2 ? this->runAction(Sequence::create(move1, move2, nullptr)) : this->runAction(Sequence::create(move2, move1, nullptr));
}

/*
受伤函数默认一次掉一点血
return 是否减血后死亡
*/
bool Enemy::hit() {
	//已经死亡直接退出
	if (this->m_hp <= 0) {
		return true;
	}
	this->m_hp--;
	//现在减血是否死亡·

	if (this->m_hp <= 0) {
		this->down();
		return true;
	}
	//受伤动画
	else {
		switch (m_type)
		{
		case EnemyType::MIDDLE_ENEMY:
		{
			auto ani = AnimationCache::getInstance()->getAnimation(MIDDLE_ENEMY_HIT_ANIMATION);
			runAction(Animate::create(ani));
			break;
		}
		case EnemyType::BIG_ENEMY:
		{
			auto ani = AnimationCache::getInstance()->getAnimation(BIG_ENEMY_HIT_ANIMATION);
			runAction(Animate::create(ani));
			break;
		}
		break;
		default:
			break;
		}
	}
	return false;

}