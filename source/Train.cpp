#include "Train.h"

#include "PathCalculation.h"
#include "framework/engine.h"


Train::Train(size_t wagonsCount, Mesh& wagonMesh, std::vector<glm::vec3>& points,
	std::vector<float>& lengths, float speed)
	: speed(speed)
{
	auto engine = Engine::get();
	wagons.reserve(wagonsCount);
	distances.reserve(wagonsCount);

	const glm::vec3 wagonSize = glm::vec3(0.7f, 0.7f, 0.7f);
	float distance =  0.f;
	//the next wagon position is the
	//wagon length plus some delta 
	float offset = wagonSize.x + 0.05f;

	for (int i = 0; i < wagonsCount; i++)
	{
		auto t = getTimeByTraversedDistance(distance, points, lengths);
		auto pos = getPointAlongAllCurve(points, t);
		
		auto g = getGradient(points, t);
		auto angle = getAngleByGradient(g);

		Object* wagon = engine->createObject(&wagonMesh);
		wagon->setColor(0.75, 0.5, 0.2f);
		wagon->setPosition(pos.x, pos.y, pos.z);
		wagon->setRotation(0.0f, angle, 0.0f);
		wagon->setScale(wagonSize.x, wagonSize.y, wagonSize.z);
		wagons.push_back(wagon);
		
		distances.push_back(distance);
		distance += offset;
	}
}

Train::~Train()
{
	auto engine = Engine::get();
	for (auto& wagon : wagons)
	{
		engine->deleteObject(wagon);
	}
}

Train::Train(Train&& other) :
 speed(std::exchange(other.speed, 0.0f)),
 distances(std::exchange(other.distances, std::vector<float>())),
 wagons(std::exchange(other.wagons, std::vector<Object*>()))
{
}

void Train::makeMove(std::vector<glm::vec3>& points, float deltaTime,
                     std::vector<float>& lengths, float totalLength)
{
	for (size_t i = 0; i < distances.size(); i++)
	{
		auto wagon = wagons[i];
		auto& d = distances[i];

		//more accurate method for calculating position were used
		//where the gaussian quadrature was calculated
		auto t = getTimeByTraversedDistance(d, points, lengths);
		auto pos = getPointAlongAllCurve(points, t);
		wagon->setPosition(pos);

		auto g = getGradient(points, t);
		auto angle = getAngleByGradient(g);
		wagon->setRotation(0.0f, angle, 0.0f);
		
		d += speed * deltaTime;
		if (d > totalLength)
		{
			d = d - totalLength;
		}
	}
}
