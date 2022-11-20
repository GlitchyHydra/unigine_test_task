#pragma once

#include <vector>
#include "glm/glm.hpp"

//get first derivative at point
glm::vec3 getGradient(const std::vector<glm::vec3>& points, float s);

//get angle from derivative
inline float getAngleByGradient(glm::vec3& gradient)
{
	return glm::degrees(atan2(-gradient.z, gradient.x));
}

//get approximated velocity
//that is not accurate method to receive constant velocity
//the small current speed is required
inline float getActualSpeed(float currentSpeed, float dx, float dy)
{
	return currentSpeed / glm::sqrt(dx * dx + dy * dy);
}

glm::vec3 getPointAlongAllCurve(const std::vector<glm::vec3>& points, float step);

//calculate the length of all spline pieces;
void calculateCurveLengths(const std::vector<glm::vec3>& points, std::vector<float>& lengths);

//get t when the object will reach the distance (length) at spline
float getTimeByTraversedDistance(float distance, const std::vector<glm::vec3>& points, const std::vector<float>& lengths);

inline glm::vec3 getPointByDistance(float distance, const std::vector<glm::vec3>& points, const std::vector<float>& lengths)
{
	auto t = getTimeByTraversedDistance(distance, points, lengths);
	return getPointAlongAllCurve(points, t);
}