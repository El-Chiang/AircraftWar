#pragma once
#include "cocos2d.h"
#include "Enemy.h"

USING_NS_CC;
//游戏窗口
class GameScene : public Scene {
public:
	GameScene() :m_offset(Vec2::ZERO), IsGameOver(false), m_totalScore(0), m_doubleBulletCount(0), m_ufoType(0), m_totalBombCount(2), IsPause(0) {}//初始化列表
	static Scene* CreateScene();
	bool init() override;
	CREATE_FUNC(GameScene);
	void update(float) override;

private:
	Vec2 m_offset;				//记录触摸点与飞机中心的偏移向量
	Vector<Sprite *> m_bullets; //存放有效的子弹
	Vector<Sprite *> m_UFO;     //存放道具
	Vector<Enemy *> m_enemys;   //存放有效的敌机
	int m_totalScore;           //总的分数
	int m_totalBombCount;         //总的炸弹数
	int m_doubleBulletCount;    //剩余双子弹数目
	int m_ufoType;              //道具类型
	void createBullet(float);
	int IsPause;				//是否暂停；
	bool IsGameOver;				//游戏结束
	
	//void createTwoBullet(float);
	void createEnemy(const EnemyType&);
	void createSmallEnemy(float);
	void createMiddleEnemy(float);
	void createBigEnemy(float);
	void createUFO(float);
	void updateBomb();
	void gameOver();

	/*public:
	Sprite * bg1;
	Sprite * bg2;
	void initbg();
	*/
};


