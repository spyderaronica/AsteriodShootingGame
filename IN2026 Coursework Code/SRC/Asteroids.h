#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include <algorithm>
#include <cctype> 

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include "SaveAndLoadData.h"

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

	bool getIsGameStart() { return isGameStart; }
	bool getIsNameEnter() { return isNameEnter; }

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> mGameStartLabel;
	shared_ptr<GUILabel> mPlayerNameLabel;
	shared_ptr<GUILabel> mPlayerDataLabel;

	uint mLevel;
	uint mAsteroidCount;
	bool isGameStart;
	bool isNameEnter;
	bool isDemo;
	std::string playerName;
	SaveAndLoadData* data;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();
	void HandleStartScreen();
	void RotateSpaceship(int value);
	void ShootAsteriod();
	void MoveSpaceship(int value);

	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint ROTATE_SPACESHIP = 3;
	const static uint SHOOT_ASTERIOD = 4;
	const static uint MOVE_ASTERIOD = 5;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif