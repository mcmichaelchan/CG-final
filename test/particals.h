#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
using namespace std;
typedef struct//structrue for particle
{
	bool active;//active or not
	float r, g, b;//color
	float x, y, z;//the position
	float xi, yi, zi;//what direction to move
	float xg, yg, zg;//gravity
}particles;

#define MAX_PARTICLES 10000
#define NEW_PARTICLES 200

static bool init_accomplishment = false;


class particals_operator {
public:
	void init();
	void display(float cameraPosX, float cameraPosY, float cameraPosZ);
	void update();
private:
	vector<particles> particle;
};

void particals_operator::init() {
	if (particle.size() < MAX_PARTICLES) {
		for (int i = 0; i != NEW_PARTICLES; i++) {
			particles temp;
			temp.x = 1.0f + float((rand() % 50) - 25.0f) / 200.0f;
			temp.y = float((rand() % 50) - 25.0f) / 25.0f;
			temp.z = float((rand() % 50) - 25.0f) / 25.0f;
			temp.active = true;
			temp.r = 0.5f + float((rand() % 50)) / 200.0f;
			temp.g = 0.5f + float((rand() % 50)) / 200.0f;
			temp.b = 0.15f + float((rand() % 50)) / 800.0f;
			temp.xi = -1;
			temp.yi = (rand() % 100 - 200) / 1000;
			temp.zi = (rand() % 100 - 200) / 1000;
			temp.xg = 0;
			temp.yg = -0.1;
			temp.zg = 0;
			particle.push_back(temp);
		}
	}
}
void particals_operator::update() {
	for (int i = 0; i != particle.size(); i++) {
		particle[i].x = particle[i].x + (particle[i].xi + particle[i].xg) * 0.05;
		particle[i].y = particle[i].y + (particle[i].yi + particle[i].yg) * 0.05;
		particle[i].z = particle[i].z + (particle[i].zi + particle[i].zg) * 0.05;
		if (particle[i].x < -1 || particle[i].y < -1 || particle[i].z < -1 || particle[i].y > 1 || particle[i].z > 1) {
			particle[i].active = false;
		}
		if (!particle[i].active) {
			particle[i].x = 1.0f + float((rand() % 50) - 25.0f) / 200.0f;
			particle[i].y = float((rand() % 50) - 25.0f) / 25.0f;
			particle[i].z = float((rand() % 50) - 25.0f) / 25.0f;
			particle[i].active = true;
			particle[i].r = 0.5f + float((rand() % 50)) / 200.0f;
			particle[i].g = 0.5f + float((rand() % 50)) / 200.0f;
			particle[i].b = 0.15f + float((rand() % 50)) / 800.0f;
			particle[i].xi = -1;
			particle[i].yi = (rand() % 100 - 200) / 1000;
			particle[i].zi = (rand() % 100 - 200) / 1000;
			particle[i].xg = 0;
			particle[i].yg = -0.3;
			particle[i].zg = 0;
		}
	}
}
void particals_operator::display(float cameraPosX, float cameraPosY, float cameraPosZ) {
	for (int i = 0; i != particle.size(); i++) {
		float vertice[] = {
			particle[i].x + cameraPosX, particle[i].y + cameraPosY, particle[i].z + cameraPosZ, particle[i].r, particle[i].g, particle[i].b,
		};
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(2.0f);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertice), vertice, GL_STATIC_DRAW);
		glDrawArrays(GL_POINTS, 0, 1);
	}
}