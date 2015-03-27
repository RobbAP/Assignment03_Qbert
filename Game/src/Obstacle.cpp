#include "Obstacle.h"
#include <SDL_opengl.h>
#include <iostream>
#include <InputManager.h>
#include <SDL_mixer.h>

// Sound files to load
char* enemyEnterWav = "../Game/enemy_enter.wav";
char* enemyJumpWav = "../Game/enemy_hop.wav";
char* enemyDeathWav = "../Game/enemy_death.wav";

struct Vertex
{
	Vector3 position;
	Vector4 color;
};

/*
* Constructor
*/
Obstacle::Obstacle()
{
	_transform.position = Vector3::Zero();
	_transform.rotation = Vector3::Zero();
	_transform.scale = Vector3::One();
}

/*
* Constructor to initialize an obstacle as a specific position
*/
Obstacle::Obstacle(Vector3 position)
{
	_transform.position = position;
	_transform.rotation = Vector3::Zero();
	_transform.scale = Vector3::One();
}

/*
* Destructor
*/
Obstacle::~Obstacle()
{
	closeObstacleMedia();
}

/*
 * Obstacle's initialize function
 */
void Obstacle::Initialize(Graphics *graphics)
{
	enemyEnterSound = NULL;
	enemyJumpSound = NULL;
	enemyDeathSound = NULL;
	size = 0;
	vertices = new Vector3[8]();
	colours = new Vector4[8]();
	moveDelay = 2.0f;
	moving = false;

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not be initialized!");
	}
	loadObstacleMedia();

	SetVertex(0, /*pos*/-0.5f, 0.5f, 0.5f,  /*color*/ 0.0f, 0.0f, 255.0f, 1.0f);
	SetVertex(1, /*pos*/0.5f, 0.5f, 0.5f,   /*color*/ 0.0f, 100.0f, 255.0f, 1.0f);
	SetVertex(2, /*pos*/-0.5f, -0.5f, 0.5f, /*color*/ 0.0f, 0.0f, 255.0f, 1.0f);
	SetVertex(3, /*pos*/0.5f, -0.5f, 0.5f,  /*color*/ 0.0f, 109.0f, 255.0f, 1.0f);
	SetVertex(4, /*pos*/-0.5f, 0.5f, -0.5f, /*color*/ 0.0f, 109.0f, 255.0f, 1.0f);
	SetVertex(5, /*pos*/0.5f, 0.5f, -0.5f,  /*color*/ 0.0f, 0.0f, 255.0f, 1.0f);
	SetVertex(6, /*pos*/-0.5f, -0.5f, -0.5f,/*color*/ 0.0f, 109.0f, 255.0f, 1.0f);
	SetVertex(7, /*pos*/0.5f, -0.5f, -0.5f, /*color*/ 0.0f, 109.0f, 255.0f, 1.0f);

	indices = new unsigned int[30];

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Set up our indices
	// front
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 1;
	indices[4] = 3;
	indices[5] = 2;

	// right
	indices[6] = 1;
	indices[7] = 5;
	indices[8] = 3;

	indices[9] = 5;
	indices[10] = 7;
	indices[11] = 3;

	// back
	indices[12] = 5;
	indices[13] = 4;
	indices[14] = 7;

	indices[15] = 4;
	indices[16] = 6;
	indices[17] = 7;

	// left
	indices[18] = 4;
	indices[19] = 0;
	indices[20] = 6;

	indices[21] = 6;
	indices[22] = 0;
	indices[23] = 2;

	// top
	indices[24] = 4;
	indices[25] = 5;
	indices[26] = 0;

	indices[27] = 5;
	indices[28] = 1;
	indices[29] = 0;

	// bottom
	indices[30] = 2;
	indices[31] = 3;
	indices[32] = 6;

	indices[33] = 3;
	indices[34] = 7;
	indices[35] = 6;
}

/*
 * Obstacle's update function
 */
void Obstacle::Update(float dt)
{
	if (isLive)
	{
		if (_transform.position.y < -5)
		{
			Reset();
			Mix_PlayChannel(-1, enemyDeathSound, 0);
		}
	}

	if (!isLive)
	{
		spawnDelay -= dt;
	}
	else
	{
		if (!moving)
		{
			moveDelay -= dt;
		}
		else
		{
			Vector3 tempPos = GetPosition();

			int random = rand() % 2;
			if (random == 0)
			{
				_transform.position.x++;
			}
			else
			{
				_transform.position.z++;
			}
			_transform.position.y--;

			moving = false;
			moveDelay = 2.0f;
			Mix_PlayChannel(-1, enemyJumpSound, 0);
		}
		if (moveDelay < 0.0f)
		{
			moving = true;
		}
	}

	if (spawnDelay < 0.0f)
	{
		SetIsLive(true);
		Mix_PlayChannel(-1, enemyEnterSound, 0);
		SetSpawnDelay(1.0);
	}
}

/*
* Obstacle's draw function
*/
void Obstacle::Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt)
{
	if (isLive)
	{
		GLenum error = glGetError();

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_INDEX_ARRAY);

		glPushMatrix();
		glTranslatef(_transform.position.x, _transform.position.y, _transform.position.z);
		glRotatef(_transform.rotation.x, 1.0f, 0.0f, 0.0f);
		glRotatef(_transform.rotation.y, 0.0f, 1.0f, 0.0f);
		glRotatef(_transform.rotation.z, 0.0f, 0.0f, 1.0f);

		glScalef(_transform.scale.x, _transform.scale.y, _transform.scale.z);

		glVertexPointer(3, GL_FLOAT, 0, vertices);

		glColorPointer(4, GL_FLOAT, 0, colours);

		int indexCount = sizeof(indices) / sizeof(unsigned int);
		indexCount = 30;
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);

		glPopMatrix();

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_INDEX_ARRAY);
	}
}

/*
* Sets the colour of a cubes vertices
*/
void Obstacle::SetVertex(int index, float x, float y, float z, float r, float g, float b, float a)
{
	vertices[index] = Vector3(x, y, z);
	colours[index].x = r;
	colours[index].y = g;
	colours[index].z = b;
	colours[index].w = a;
}

/*
* Sets the position of an obstacle
*/
void Obstacle::SetPosition(Vector3 pos)
{
	_transform.position = pos;
}

/*
* Gets the position of an obstacle
*/
Vector3 Obstacle::GetPosition()
{
	return this->_transform.position;
}

/*
* Sets the amount of time an obstacle waits before spawning
*/
void Obstacle::SetSpawnDelay(float delay)
{
	spawnDelay = delay;
}

/*
* Sets the isLive variable which indicates whether an obstacle is alive or not
*/
void Obstacle::SetIsLive(bool live)
{
	isLive = live;
}

/*
* Returns true if an obstacle is alive
*/
bool Obstacle::GetIsLive()
{
	return isLive;
}

/*
* Resets an obstacle to one of the default cubes
*/
void Obstacle::Reset()
{
	isLive = false;
	moving = false;
	spawnDelay = 3.0f;
	int random = rand() % 2;
	if (random == 0)
	{
		_transform.position = Vector3(1, 0, 0);
	}
	else
	{
		_transform.position = Vector3(0, 0, 1);
	}
}

/*
* Loads the sound files used by the obstacles in the game
*/
bool Obstacle::loadObstacleMedia()
{
	//Loading success flag
	bool success = true;

	// Enemy Sounds
	enemyEnterSound = Mix_LoadWAV(enemyEnterWav);
	enemyJumpSound = Mix_LoadWAV(enemyJumpWav);
	enemyDeathSound = Mix_LoadWAV(enemyDeathWav);

	if (enemyEnterSound == NULL)
	{
		printf("Failed to load the Enemy Spawn sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (enemyJumpSound == NULL)
	{
		printf("Failed to load the Enemy Jump sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	else if (enemyDeathSound == NULL)
	{
		printf("Failed to load the Enemy Death sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}

	return success;
}

/*
* Close and free memory for the sound files an obstacle uses
*/
void Obstacle::closeObstacleMedia()
{
	Mix_FreeChunk(enemyEnterSound);
	Mix_FreeChunk(enemyJumpSound);
	Mix_FreeChunk(enemyDeathSound);

	enemyEnterSound = NULL;
	enemyJumpSound = NULL;
	enemyDeathSound = NULL;
}