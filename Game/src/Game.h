#pragma once

#include <GameEngine.h>
#include "Cube.h"
#include "Player.h"
#include "Obstacle.h"
#include <SDL_mixer.h>

union SDL_Event;
class Graphics;
class Camera;

class Game : public GameEngine
{
	friend class GameEngine;
public:
	~Game();
protected:
	Game();
	void InitializeImpl();
	void UpdateImpl(float dt);
	void DrawImpl(Graphics *graphics, float dt);

	void Reset();
	void CalculateDrawOrder(std::vector<GameObject *>& drawOrder);
	void CalculateCameraViewpoint();
	void closeGameMedia();
	bool loadGameMedia();
	bool V3Collision(Vector3 object1, Vector3 object2);

	float _cubesRotation;
	float _cubesSpeed;
	float _cubesScale;
	float _gridWidth;
	float _gridHeight;
	int level;
	int lives;

	Camera *_camera;
	Player _player;
	Vector3 _playerPos;
	
	// Obstacles to avoid
	Obstacle _ball[3];

	// Game sounds
	Mix_Chunk* deathSound;
	Mix_Chunk* scoreSound;
	Mix_Chunk* gameOverSound;
	Mix_Chunk* levelSound;
	Mix_Chunk* moveSound;

	Cube *newCube;
};