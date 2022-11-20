#pragma once

#include <vector>
#include "framework/object.h"

class Train final
{
public:
	Train(size_t wagonsCount, Mesh& wagonMesh, std::vector<glm::vec3>& points,
		std::vector<float>& lengths, float speed);
	~Train();

	//there can not be two trains at one line
	//at one position
	Train(const Train& other) = delete;
	Train(Train&& other);

	//move all wagons by speed
	void makeMove(std::vector<glm::vec3>& points, float deltaTime,
		std::vector<float>& lengths, float totalLength);

private:
	float speed;
	std::vector<float> distances;
	std::vector<Object*> wagons;
};
