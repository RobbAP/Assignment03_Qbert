#pragma once

#include <GameObject.h>
#include <SDL_mixer.h>

struct Vertex;

class Obstacle : public GameObject
{
public:
	Obstacle();
	~Obstacle();
	Obstacle(Vector3 position);
	void Initialize(Graphics *graphics);
	void Update(float dt);
	void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);
	void SetPosition(Vector3 pos);
	Vector3 GetPosition();
	void SetSpawnDelay(float delay);
	void SetIsLive(bool live);
	bool GetIsLive();
	void Reset();
	bool loadObstacleMedia();
	void closeObstacleMedia();
protected:
	bool isLive;
	Vector3 *vertices;
	Vector4 *colours;
	bool moving;
	int size;
	int offset;
	unsigned int* indices;
	float spawnDelay;
	float moveDelay;
	float r, g, b;

	// Enemy Sounds
	Mix_Chunk* enemyEnterSound;
	Mix_Chunk* enemyJumpSound;
	Mix_Chunk* enemyDeathSound;

	void SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);
};