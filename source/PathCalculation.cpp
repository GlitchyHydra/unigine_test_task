#include "PathCalculation.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/spline.hpp"

glm::vec3 getGradient(const std::vector<glm::vec3>& points, float s)
{
	const int i1 = (int)s;
	const int i2 = i1 + 1;
	const int i3 = i1 + 2;
	const int i4 = i1 + 3;

	//get the proper value for a certain piece of curve
	float innerS = s - (float)i1;

	const auto size = points.size();
	const glm::vec3& v1 = points[i1 % size];
	const glm::vec3& v2 = points[i2 % size];
	const glm::vec3& v3 = points[i3 % size];
	const glm::vec3& v4 = points[i4 % size];

	auto s2 = glm::pow2(innerS);

	auto f1 = -3.0f * s2 + 4.0f * innerS - 1.0f;
	auto f2 =  9.0f * s2 - 10.0f * innerS;
	auto f3 = -9.0f * s2 + 8.0f * innerS + 1.0f;
	auto f4 =  3.0f * s2 - 2.0f * innerS;

	return 0.5f * (f1 * v1 + f2 * v2 + f3 * v3 + f4 * v4);
}

glm::vec3 getPointAlongAllCurve(const std::vector<glm::vec3>& points, float s)
{
	const int i1 = (int)s;
	const int i2 = i1 + 1;
	const int i3 = i1 + 2;
	const int i4 = i1 + 3;

	//get the proper value for a certain piece of curve
	auto innerS = s - (float)i1;

	const auto size = points.size();
	const glm::vec3& v1 = points[i1 % size];
	const glm::vec3& v2 = points[i2 % size];
	const glm::vec3& v3 = points[i3 % size];
	const glm::vec3& v4 = points[i4 % size];

	return glm::catmullRom(v1, v2, v3, v4, innerS);
}

constexpr float quad[] = {
	-0.7745966f, 0.5555556f,
	0.f, 0.8888889f,
	0.7745966f, 0.5555556f
};

float lerp(float x, float y, float v)
{
	return x* (1.f - v) + (y * v);
}

float inverseLerp(float x, float y, float v)
{
	return (v - x) / (y - x);
}

float getArcLengthByGaussianQuad(float l, float u, const std::vector<glm::vec3>& points)
{
	float sum = 0.f;
	for(int i = 0; i < 6; i+=2)
	{
		float arg = quad[i];
		float weight = quad[i + 1];

		auto t = lerp(l, u, inverseLerp(-1, 1, arg));
		sum += weight * glm::length(getGradient(points, t));
	}

	return sum * (u - l) / 2;
}


float getTimeByTraversedDistance(float distance, const std::vector<glm::vec3>& points,
	const std::vector<float>& lengths)
{
	float currentLength = lengths[0];
	float offset = 0.0f;
	for (auto length : lengths)
	{
		if (distance < length)
			break;

		currentLength = length;
		offset += 1.f;
		distance -= length;
	}
	
	float t = 0.f + distance / currentLength;
	float lower = 0.f;
	float upper = 1.f;

	for (int i = 0; i < 100; ++i)
	{
		float f = getArcLengthByGaussianQuad(offset, offset + t, points) - distance;

		if (abs(f) < 0.01f)
			break;

		float dy = glm::length(getGradient(points, offset + t));
		float tNext = t - f / dy;

		//newton bisection method
		if (f > 0)
		{
			upper = t;
			if (tNext <= 0)
				t = (upper + lower) / 2.f;
			else
				t = tNext;
		} else
		{
			lower = t;
			if (tNext >= 1)
				t = (upper + lower) / 2.f;
			else
				t = tNext;
		}
	}
	return offset + t;
}



void calculateCurveLengths(const std::vector<glm::vec3>& points, std::vector<float>& lengths)
{
	for (int i = 0; i < points.size(); i++)
	{
		float low = i;
		float high = i + 1;
		lengths.push_back(getArcLengthByGaussianQuad(low, high, points));
	}
}