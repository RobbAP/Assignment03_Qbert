#pragma once

#include <GameObject.h>

struct Vertex;

class Cube : public GameObject
{
public:
	Cube();
	Cube(Vector3 position);
	void Initialize(Graphics *graphics);
	void Update(float dt);
	void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);
	void SetIsTouched(bool isTouched);
	bool GetIsTouched();
	void Reset();

protected:
	Vector3 *vertices;
	Vector4 *colours;
	float r, g, b;
	int size;
	int offset;
	bool _isTouched = false;
	bool _touchedSet;
	unsigned int* indices;

	void SetVertex(int index, float x, float y, float z, float r, float g, float b, float a);
};
