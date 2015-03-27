#pragma once

#include <GameObject.h>

struct Vertex;

class Player : public GameObject
{
public:
	Player();
	Player(Vector3 position);

	void Initialize(Graphics *graphics);
	void Update(float dt);
	void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);
	void SetPosition(Vector3 pos);
	Vector3 GetPosition();
	bool GetPlayerMoved();
	void SetPlayerMoved(bool moved);
	void Reset();

protected:
	Vector3 *vertices;
	Vector4 *colours;
	bool _playerMoved;
	float r, g, b;
	int size;
	int offset;
	unsigned int* indices;

	void SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);
};