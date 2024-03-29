#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	isGameStart = false;
	isNameEnter = false;
	isDemo = true;
	data = new SaveAndLoadData("HighScore.txt");
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	mGameWorld->AddObject(CreateSpaceship());

	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Demo scene functionality
	MoveSpaceship(10);
	RotateSpaceship(90);
	ShootAsteriod();

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (!getIsNameEnter() && isDemo)
	{
		if (key == '\r') {
			// Find the first non-whitespace character from the beginning
			auto firstNonWhitespace = std::find_if_not(playerName.begin(), playerName.end(), [](unsigned char c) { return std::isspace(c); });

			// Find the first non-whitespace character from the end
			auto lastNonWhitespace = std::find_if_not(playerName.rbegin(), playerName.rend(), [](unsigned char c) { return std::isspace(c); }).base();

			playerName = (firstNonWhitespace < lastNonWhitespace) ? std::string(firstNonWhitespace, lastNonWhitespace) : std::string();

			if (playerName != std::string()) {
				isNameEnter = true;
				mPlayerNameLabel->SetVisible(false);
				mGameStartLabel->SetVisible(true);
			}
			else
			{
				mPlayerNameLabel->SetText("Enter Name: ");
				playerName = std::string();
			}
		}
		else if (key == '\b') { // If Backspace key is pressed
			if (!playerName.empty()) {
				playerName.pop_back(); // Remove the last character
				std::string name_msg = "Enter Name: ";
				name_msg.append(playerName);
				mPlayerNameLabel->SetText(name_msg);
			}
		}
		else if (std::isalnum(key) || key == ' ') { // If alphanumeric character or space is pressed
			playerName += key; // Append the character to the player name
			std::string name_msg = "Enter Name: ";
			name_msg.append(playerName);
			mPlayerNameLabel->SetText(name_msg);
		}
	}
	/* If the game has not started yet(checked via getIsGameStart()),
	   set the flag indicating that the game has started to true,
	   and hide the game start label (mGameStartLabel).
	*/
	else if (!getIsGameStart() && isDemo)
	{
		isDemo = false;
		mScoreKeeper.ResetScore();
		mPlayer.Resetlives();
		mLevel = 0;
		mSpaceship->SelfDestroy();

		HandleStartScreen();
	}
	else
	{
		switch (key)
		{
		case ' ':
			mSpaceship->Shoot();
			break;
		default:
			break;
		}
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (!getIsGameStart() && isDemo) return;

	switch (key)
	{
		// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
		// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
		// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (!getIsGameStart() && isDemo) return;

	switch (key)
	{
		// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
		// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
		// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		// Default case - do nothing
	default: break;
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0)
		{
			SetTimer(500, START_NEXT_LEVEL);
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
		mPlayerDataLabel->SetVisible(true);
	}

	if (value == ROTATE_SPACESHIP)
	{
		RotateSpaceship(90);
	}

	if (value == SHOOT_ASTERIOD)
	{
		ShootAsteriod();
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world

	if (isDemo) mSpaceship->Thrust(2);

	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mPlayerNameLabel = shared_ptr<GUILabel>(new GUILabel("Enter Name: "));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mPlayerNameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mPlayerNameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to true (visible)
	mPlayerNameLabel->SetVisible(true);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> player_name_component
		= static_pointer_cast<GUIComponent>(mPlayerNameLabel);
	mGameDisplay->GetContainer()->AddComponent(player_name_component, GLVector2f(0.5f, 0.5f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set the visibility of the label to false (hidden)
	mScoreLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Set the visibility of the label to false (hidden)
	mLivesLabel->SetVisible(false);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameStartLabel = shared_ptr<GUILabel>(new GUILabel("Press any key to start"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameStartLabel->SetVisible(false);
	// Set the color to yellow
	mGameStartLabel->SetColor(GLVector3f(1, 1, 0));
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_start_component
		= static_pointer_cast<GUIComponent>(mGameStartLabel);
	mGameDisplay->GetContainer()->AddComponent(game_start_component, GLVector2f(0.5f, 0.5f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Set the color to red
	mGameOverLabel->SetColor(GLVector3f(1, 0, 0));
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mPlayerDataLabel = shared_ptr<GUILabel>(new GUILabel(""));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mPlayerDataLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mPlayerDataLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mPlayerDataLabel->SetVisible(false);
	// Set the color to green
	mPlayerDataLabel->SetColor(GLVector3f(0, 1, 0));
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> player_data_component
		= static_pointer_cast<GUIComponent>(mPlayerDataLabel);
	mGameDisplay->GetContainer()->AddComponent(player_data_component, GLVector2f(0.5f, 0.4f));
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	if (isDemo)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
		return;
	}

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0)
	{
		SetTimer(1000, CREATE_NEW_PLAYER);
	}
	else
	{
		std::pair<std::string, int> playerdata = data->LoadData(playerName);
		if (playerdata.first.empty())
		{
			data->SaveData({ playerName, mScoreKeeper.getScore() });
			playerdata.first = playerName;
			playerdata.second = mScoreKeeper.getScore();
		}
		else if (playerdata.second < mScoreKeeper.getScore())
		{
			data->UpdateData({ playerName, mScoreKeeper.getScore() });
			playerdata.second = mScoreKeeper.getScore();
		}
		std::string datatodisplay = "Name: " + playerdata.first + " & " + "Highscore: " + std::to_string(playerdata.second);
		mPlayerDataLabel->SetText(datatodisplay);
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

// Handle Start screen functionality
void Asteroids::HandleStartScreen()
{
	isGameStart = true; // Set the flag indicating the game has started
	mGameStartLabel->SetVisible(false); // Hide the game start label
	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);
	// Create a spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship());
}

//Below function use to handle spaceship automatically in demo scene
void Asteroids::RotateSpaceship(int value)
{
	if (!isDemo) return;

	mSpaceship->Rotate(value);
	SetTimer(500, ROTATE_SPACESHIP);
}

void Asteroids::ShootAsteriod()
{
	if (!isDemo) return;

	mSpaceship->Shoot();
	SetTimer(700, SHOOT_ASTERIOD);
}

void Asteroids::MoveSpaceship(int value)
{
	if (!isDemo) return;

	mSpaceship->Thrust(value);
	SetTimer(500, MOVE_ASTERIOD);
}
