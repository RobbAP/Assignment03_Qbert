#include "Game.h"
#include <GameObject.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <InputManager.h>
#include <SDL_mixer.h>
#include <Cameras/Camera.h>
#include <Cameras/PerspectiveCamera.h>
#include <Cameras/OrthographicCamera.h>

// Initializing our static member pointer.
GameEngine* GameEngine::_instance = nullptr;

// Sound files to load
char* deathWav = "../Game/player_death.wav";
char* scoreWav = "../Game/score_sound.wav";
char* gameOverWav = "../Game/game_over.wav";
char* levelWav = "../Game/new_level.wav";
char* moveWav = "../Game/player_move.wav";

/*
 * 
 */
GameEngine* GameEngine::CreateInstance()
{
	if (_instance == nullptr)
	{
		_instance = new Game();
	}
	return _instance;
}

/*
 * Constructor
 */
Game::Game() : GameEngine()
{

}

/*
 * Game destructor
 */
Game::~Game()
{
	closeGameMedia();
}

/*
 * Game Initialize Implementation function
 */
void Game::InitializeImpl()
{
	SDL_SetWindowTitle(_window, "QBert -~- Lives: 0    Score: 0");

	deathSound = NULL;
	scoreSound = NULL;
	gameOverSound = NULL;
	levelSound = NULL;
	moveSound = NULL;

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("Failed to initialize the audio");
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not be initialized!");
	}
	loadGameMedia();

	level = 1;
	lives = 5;

	float _gridWidth = 7.0f;
	float _gridHeight = 7.0f;

	int cube = 0; //testing purposes
	int offSetY = 0; // pyramid shape instead of bipyramid

	float nearPlane = 0.01f;
	float farPlane = 100.0f;

	Vector4 position(3.25f, 2.5f, 2.5f, 0.0f);
	Vector4 lookAt = Vector4::Normalize(Vector4::Difference(Vector4(0.0f, 0.0f, 0.0f, 0.0f), position));
	Vector4 up(0.0f, 0.0f, 0.0f, 0.0f);

	//_camera = new PerspectiveCamera(100.0f, 1.0f, nearPlane, farPlane, position, lookAt, up);
	_camera = new OrthographicCamera(-8.0f, 8.0f, 8.0f, -8.0f, nearPlane, farPlane, position, lookAt, up);

	//Spawn player
	_playerPos = Vector3(0, 1, 0);
	_player.SetPosition(_playerPos);
	_objects.push_back(&_player);

	//Spawn Enemy
	for (int i = 0; i < 3; i++)
	{
		int random = rand() % 2;
		if (random == 0)
		{
			_ball[i].SetPosition(Vector3(1, 0, 0));
		}
		else
		{
			_ball[i].SetPosition(Vector3(0, 0, 1));
		}

		_objects.push_back(&_ball[i]);
		// Increase difficulty by decreasing the spawn delay on the enemies by 0.1 second per level
		_ball[i].SetSpawnDelay((i + 1) * (3.0f - (0.1f * level)));
		_ball[i].SetIsLive(false);
	}

	//Creating our grid.
	newCube = new Cube[28]();
	for (int gridX = 0; gridX < _gridWidth; gridX++)
	{
		for (int gridZ = 0; gridZ < _gridHeight - offSetY; gridZ++)
		{
			//World coordinates.
			float worldX = gridX;
			float worldY = -(gridX + gridZ);
			float worldZ = gridZ;
			//printf("Cube%d location: Z:%f, X:%f, Y:%f\n", cube + 1, worldZ, worldX, worldY); - Prints the location of each cube in the world
			//Setting position based on world coordinates.
			newCube[cube].GetTransform().position = Vector3(worldX, worldY, worldZ);
			_objects.push_back(&newCube[cube]);
			cube++;
		}
		offSetY++;
	}

	for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
	{
		(*itr)->Initialize(_graphicsObject);
	}
}

/*
 * Game's update implementation function
 */
void Game::UpdateImpl(float dt)
{
	char title[50] = "QBert - ~ - Lives: ";
	char lifeCount[10];
	char levelCount[10];
	int j = 0;
	char playAgain = NULL;
	bool again = false;

	_playerPos = _player.GetPosition();
	//Player Interactions with world tiles
	if (_player.GetPlayerMoved())
	{
		for (int i = 0; i < 28; i++)
		{
			if(_playerPos.x == newCube[i].GetTransform().position.x &&
			   _playerPos.y == newCube[i].GetTransform().position.y + 1 &&
			   _playerPos.z == newCube[i].GetTransform().position.z)
			{
				if (newCube[i].GetIsTouched() == false)
					Mix_PlayChannel(-1, scoreSound, 0);
				newCube[i].SetIsTouched(true);
				break;
			}
		}
	}

	//Player Interactions with borders
	if (_playerPos.y > 1 || _playerPos.y < -5 || _playerPos.z < 0 || _playerPos.x < 0)
	{
		// Death Sound
		Mix_PlayChannel(-1, deathSound, 0);
		lives--;
		_playerPos.x = 0;
		_playerPos.y = 1;
		_playerPos.z = 0;
		_player.Reset();

		for (int j = 0; j < 3; j++)
		{
			_ball[j].Reset();
			_ball[j].SetSpawnDelay((j + 1)*3.0f);
		}
	}

	//Player to Obstacle Interactions
	for (int i = 0; i < 3; i++)
	{
		if (_ball[i].GetIsLive())
		{
			if (V3Collision(_ball[i].GetPosition(), _playerPos))
			{
				// Death Sound
				Mix_PlayChannel(-1, deathSound, 0);
				lives--;
				_playerPos.x = 0;
				_playerPos.y = 1;
				_playerPos.z = 0;
				_player.Reset();

				for (int j = 0; j < 3; j++)
				{
					_ball[j].Reset();
					_ball[j].SetSpawnDelay((j + 1)*3.0f);
				}
				break;
			}
		}
	}

	// If the player runs out of lives, prompt them if they would like to play again
	if (lives < 1)
	{
		again = false;
		_player.Reset();
		for (int i = 0; i < 28; i++)
		{
			newCube[i].Reset();
		}
		for (int j = 0; j < 3; j++)
		{
			_ball[j].Reset();
			_ball[j].SetSpawnDelay((j + 1)*3.0f);
		}
		_player.Reset();
		_playerPos.x = 0;
		_playerPos.y = 1;
		_playerPos.z = 0;
		j = 0;
		level = 1;
		printf("Game Over! Would you like to play again? [Y/N]\n");
		// Get user input to see if they want to play again
		while (again == false)
		{
			fflush(stdin);
			fflush(stdout);
			scanf("%c", &playAgain);
			if (playAgain == 'Y' || playAgain == 'y')
			{
				lives = 5;
				again = true;
			}
			else if (playAgain == 'N' || playAgain == 'n')
			{
				exit(EXIT_SUCCESS);
			}
			else
			{
				printf("Would you like to play again? [Y/N]\n");
				scanf("%c", &playAgain);
			}
		}
	}

	// Check if a player touches a new tile
	for (int i = 0; i < 28; i++)
	{
		if (newCube[i].GetIsTouched())
		{
			j++;
		}
		else
		{
			break;
		}
	}
	// If the player has lit up all tiles then proceed to the next level
	if (j == 28)
	{
		level++;
		for (int i = 0; i < 28; i++)
		{
			newCube[i].Reset();
		}
		for (int j = 0; j < 3; j++)
		{
			_ball[j].Reset();
			_ball[j].SetSpawnDelay((j + 1)*3.0f);
		}
		_player.Reset();
		_playerPos.x = 0;
		_playerPos.y = 1;
		_playerPos.z = 0;
		j = 0;
		Mix_PlayChannel(-1, levelSound, 0);
	}

	InputManager *im = InputManager::GetInstance();
	im->Update(dt);

	// Get user input for moving the player
	if (im->GetKeyState(SDLK_UP, SDL_KEYUP) == true) //up right
	{
		// move up grid 1 space towards upper right
		_playerPos.x = _player.GetPosition().x;
		_playerPos.y = _player.GetPosition().y + 1.0f;
		_playerPos.z = _player.GetPosition().z - 1.0f;
		_player.SetPlayerMoved(true);
		Mix_PlayChannel(-1, moveSound, 0);
	}
	else if (im->GetKeyState(SDLK_DOWN, SDL_KEYUP) == true) // down left
	{
		// move down grid 1 space towards lower left
		_playerPos.x = _player.GetPosition().x;
		_playerPos.y = _player.GetPosition().y - 1.0f;
		_playerPos.z = _player.GetPosition().z + 1.0f;
		_player.SetPlayerMoved(true);
		Mix_PlayChannel(-1, moveSound, 0);
	}
	else if (im->GetKeyState(SDLK_LEFT, SDL_KEYUP) == true) //up left
	{
		// move up grid 1 space towards upper left
		_playerPos.x = _player.GetPosition().x - 1.0f;
		_playerPos.y = _player.GetPosition().y + 1.0f;
		_playerPos.z = _player.GetPosition().z;
		_player.SetPlayerMoved(true);
		Mix_PlayChannel(-1, moveSound, 0);
	}
	else if (im->GetKeyState(SDLK_RIGHT, SDL_KEYUP) == true) // downright
	{
		// move down grid 1 space towards lower right
		_playerPos.x = _player.GetPosition().x + 1.0f;
		_playerPos.y = _player.GetPosition().y - 1.0f;
		_playerPos.z = _player.GetPosition().z;
		_player.SetPlayerMoved(true);
		Mix_PlayChannel(-1, moveSound, 0);
	}
	// Prints the position of the player cube
	//printf("X:%f,Y:%f,Z:%f\n", _player.GetPosition().x, _player.GetPosition().y, _player.GetPosition().z);

	_player.SetPosition(_playerPos);
	for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
	{
		(*itr)->Update(dt);
	}

	// Cap levels at 30
	if (level > 30)
		level = 30;

	// Update title bar
	_itoa_s(lives, lifeCount, 10);
	_itoa_s(level, levelCount, 10);

	strcat_s(title, lifeCount);
	strcat_s(title, "     Level: ");
	strcat_s(title, levelCount);

	SDL_SetWindowTitle(_window, title);
}

/*
* Game's draw function
*/
void Game::DrawImpl(Graphics *graphics, float dt)
{

	std::vector<GameObject *> renderOrder = _objects;
	glPushMatrix();
	CalculateCameraViewpoint();

	for (auto itr = renderOrder.begin(); itr != renderOrder.end(); itr++)
	{
		(*itr)->Draw(graphics, _camera->GetProjectionMatrix(), dt);
	}

	glPopMatrix();
}

/*
 *
 */
void Game::CalculateDrawOrder(std::vector<GameObject *>& drawOrder)
{
	// SUPER HACK GARBAGE ALGO.
	drawOrder.clear();
	auto objectsCopy = _objects;
	auto farthestEntry = objectsCopy.begin();
	while (objectsCopy.size() > 0)
	{
		bool entryFound = true;
		for (auto itr = objectsCopy.begin(); itr != objectsCopy.end(); itr++)
		{
			if (farthestEntry != itr)
			{
				if ((*itr)->GetTransform().position.y < (*farthestEntry)->GetTransform().position.y)
				{
					entryFound = false;
					farthestEntry = itr;
					break;
				}
			}
		}

		if (entryFound)
		{
			GameObject *farthest = *farthestEntry;

			drawOrder.push_back(farthest);
			objectsCopy.erase(farthestEntry);
			farthestEntry = objectsCopy.begin();
		}
	}
}

/*
 * Sets up the camera's viewpoint into the game world
 */
void Game::CalculateCameraViewpoint()
{
	Vector4 xAxis(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 yAxis(0.0f, 1.0f, 0.0f, 0.0f);
	Vector4 zAxis(0.0f, 0.0f, 1.0f, 0.0f);

	Vector3 cameraVector(_camera->GetLookAtVector().x, _camera->GetLookAtVector().y, _camera->GetLookAtVector().z);
	Vector3 lookAtVector(0.0f, 0.0f, -1.0f);

	Vector3 cross = Vector3::Normalize(Vector3::Cross(cameraVector, lookAtVector));
	float dot = MathUtils::ToDegrees(Vector3::Dot(lookAtVector, cameraVector));

	glRotatef(cross.x * dot, 1.0f, 0.0f, 0.0f);
	glRotatef(cross.y * dot, 0.0f, 1.0f, 0.0f);
	glRotatef(cross.z * dot, 0.0f, 0.0f, 1.0f);

	glTranslatef(-_camera->GetPosition().x, -_camera->GetPosition().y, -_camera->GetPosition().z);
}

/*
 * Checks if two vector3's are occupying the same cube
 */
bool Game::V3Collision(Vector3 object1, Vector3 object2)
{
	if(object1.x == object2.x && 
	   object1.y == object2.y && 
	   object1.z == object2.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Loads the sound files used by the game
 */
bool Game::loadGameMedia()
{
	//Loading success flag
	bool success = true;

	//Load sound effects
	deathSound = Mix_LoadWAV(deathWav);
	scoreSound = Mix_LoadWAV(scoreWav);
	gameOverSound = Mix_LoadWAV(gameOverWav);
	levelSound = Mix_LoadWAV(levelWav);
	moveSound = Mix_LoadWAV(moveWav);

	if (deathSound == NULL)
	{
		printf("Failed to load Death sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (scoreSound == NULL)
	{
		printf("Failed to load the Score sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (gameOverSound == NULL)
	{
		printf("Failed to load the Game Over sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (levelSound == NULL)
	{
		printf("Failed to load the Next Level sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (moveSound == NULL)
	{
		printf("Failed to load the PLAYER MOVEMENT sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	return success;
}

/*
 * Cleans up & frees memory for the sound files used for the game
 */
void Game::closeGameMedia()
{
	//Free the sound effects
	Mix_FreeChunk(deathSound);
	Mix_FreeChunk(scoreSound);
	Mix_FreeChunk(gameOverSound);
	Mix_FreeChunk(levelSound);
	Mix_FreeChunk(moveSound);

	deathSound = NULL;
	scoreSound = NULL;
	gameOverSound = NULL;
	levelSound = NULL;
	moveSound == NULL;

	//Quit SDL mixer
	Mix_Quit();
}