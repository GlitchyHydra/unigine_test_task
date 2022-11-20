#include "framework/engine.h"
#include "framework/utils.h"

#include "PathCalculation.h"
#include "Train.h"

using namespace std;

#define ROADBLOCK_COLOR glm::vec3(0.545f, 0.271f, 0.074f)
const glm::vec3 RAILROAD_COLOR(1.0f, 1.0f, 1.0f);

const glm::vec3 RAILROAD_BLOCK_SIZE(0.08f, 0.45f, 1.0f);
const glm::vec3 RAILROAD_SIZE(0.45f, 0.03f, 1.0f);

#define SURFACE_HEIGHT (-.5f)
#define RAILROAD_BLOCK_HEIGHT (SURFACE_HEIGHT + 0.001f)
#define RAILROAD_HEIGHT (SURFACE_HEIGHT + 0.002f)

#define INTERVAL 0.4f
#define TRAIN_SPEED 2.55f

//create railroad block object
Object* drawRailroadBlock(const glm::vec3& pos, const glm::vec3& rot,
	const glm::vec3 scale, const glm::vec3 color, Engine* engine, Mesh& mesh)
{
	Object* plane = engine->createObject(&mesh);
	plane->setColor(color.x, color.y, color.z);
	plane->setPosition(pos.x, pos.y, pos.z);
	plane->setRotation(rot.x, rot.y, rot.z);
	plane->setScale(scale.x, scale.y, scale.z);
	return plane;
}

/*
 * create railroad blocks
 * not accurate constant speed approximation was used (see getActualSpeed)
 * because the creating of static meshes is going off screen and
 * is not depend on frame deltaTime
 */
void drawRailroadBlocks(const std::vector<glm::vec3>& points, Engine* engine,
                    Mesh& mesh, std::vector<Object*>& environment)
{
	auto size = (float)points.size();
	for (float step = 0.f; step < size; )
	{
		auto pos = getPointAlongAllCurve(points, step);
		auto g = getGradient(points, step);
		auto ds = getActualSpeed(INTERVAL, g.x, g.z);

		auto angle = getAngleByGradient(g);
		pos.y = RAILROAD_BLOCK_HEIGHT;
		auto roadblock = drawRailroadBlock(pos, glm::vec3(-90.f, angle, 0.f),
			RAILROAD_BLOCK_SIZE, ROADBLOCK_COLOR, engine, mesh);
		environment.push_back(roadblock);

		step += ds;
	}
}

/*
 * generate one railroad mesh (left or right)
 * the same not accurate approximation was used
 */
void generateRailroadMesh(std::vector<glm::vec3> points, Mesh& mesh, float side = -1.f)
{
	auto size = points.size();

	//generate equally spaced points along spline
	std::vector<glm::vec3> tempPoints;
	
	for (float step = size - 1; step < size * 2; )
	{
		auto g = getGradient(points, step);
		auto pos = getPointAlongAllCurve(points, step);
		pos.y = RAILROAD_HEIGHT;
		tempPoints.push_back(pos);
		auto ds = getActualSpeed(0.1f, g.x, g.z);
		step = step + ds;
	}

	std::vector<Vertex> vertices;
	vertices.reserve(tempPoints.size() * 2);
	std::vector<unsigned int> indices;
	indices.reserve(2 * (tempPoints.size() - 1) * 3);
	int vertexIndex = 0;

	//generate vertices for one railroad mesh
	size = tempPoints.size();
	for (int i = 0; i < size; i+=2)
	{
		auto forward = glm::vec3(0.0f);
		if (i < size - 1)
			forward += tempPoints[i + 1] - tempPoints[i];
		if (i > 0)
			forward += tempPoints[i] - tempPoints[i - 1];
		forward = glm::normalize(forward);
		auto left = glm::vec3(-forward.z, forward.y, forward.x);

		//shift the position to right or left by roadblock length divided by 4
		tempPoints[i] += side * left * (RAILROAD_BLOCK_SIZE.y / 4.f);

		//generate two vertices to right and left from the point
		//UV coordinates will be broken, but it is ok because we render
		//using solid color and no textures 
		vertices.push_back({ tempPoints[i] + left * RAILROAD_SIZE.y,
			glm::vec3(0,0,1), glm::vec2(0,0) });
		vertices.push_back({ tempPoints[i] - left * RAILROAD_SIZE.y,
			glm::vec3(0,0,1), glm::vec2(0,0) });

		if (i < size - 1)
		{
			indices.push_back(vertexIndex);
			indices.push_back(vertexIndex + 2);
			indices.push_back(vertexIndex + 1);

			indices.push_back(vertexIndex + 1);
			indices.push_back(vertexIndex + 2);
			indices.push_back(vertexIndex + 3);
		}

		vertexIndex += 2;
	}
	mesh.set(vertices, indices);
}

//create railroad object
Object* drawRailroad(Engine* engine, Mesh& mesh)
{
	Object* plane = engine->createObject(&mesh);
	plane->setColor(RAILROAD_COLOR);
	return plane;
}

/*
* Coordinate system:
* x - right
* y - up
* z - backward
*/

int main()
{
	// initialization
	Engine *engine = Engine::get();
	engine->init(1600, 900, "UNIGINE Test Task");

	// set up camera
	Camera &cam = engine->getCamera();
	cam.Position = glm::vec3(0.0f, 12.0f, 17.0f);
	cam.Yaw = -90.0f;
	cam.Pitch = -45.0f;
	cam.UpdateCameraVectors();

	// create shared meshes
	Mesh plane_mesh = createPlane();
	Mesh sphere_mesh = createSphere();
	Mesh cube_mesh = createCube();

	// create background objects
	Object *plane = engine->createObject(&plane_mesh);
	plane->setColor(0.2f, 0.37f, 0.2f); // green
	plane->setPosition(0, SURFACE_HEIGHT, 0);
	plane->setRotation(-90.0f, 0.0f, 0.0f);
	plane->setScale(20.0f);

	// path
	const float path[] = {
		 0.0f, -0.375f,  7.0f, // 1
		-6.0f, -0.375f,  5.0f, // 2
		-8.0f, -0.375f,  1.0f, // 3
		-4.0f, -0.375f, -6.0f, // 4
		 0.0f, -0.375f, -7.0f, // 5
		 1.0f, -0.375f, -4.0f, // 6
		 4.0f, -0.375f, -3.0f, // 7
		 8.0f, -0.375f,  7.0f  // 8
	};

	vector<glm::vec3> points;
	for (int i = 0; i < 8; i++)
	{
		points.emplace_back(path[i * 3], path[i * 3 + 1], path[i * 3 + 2]);
	}

	//creation of railroad and rail blocks objects
	std::vector<Object*> railroad;
	drawRailroadBlocks(points, engine, plane_mesh, railroad);
	Mesh rightMesh;
	generateRailroadMesh(points, rightMesh, 1.f);
	Mesh leftMesh;
	generateRailroadMesh(points, leftMesh, -1.f);
	auto railroadOuter = drawRailroad(engine, leftMesh);
	auto railroadInner = drawRailroad(engine, rightMesh);

	//calculate the length of all curve pieces
	float totalLength = 0.0f;
	std::vector<float> lengths;
	calculateCurveLengths(points, lengths);
	for (float length : lengths)
		totalLength += length;

	//the train wagons creation
	Train train(8, cube_mesh, points, lengths, TRAIN_SPEED);

	// main loop
	while (!engine->isDone())
	{
		auto deltaTime = engine->getDeltaTime();
		train.makeMove(points, deltaTime, lengths, totalLength);

		engine->update();
		engine->render();

		engine->swap();
	}

	engine->shutdown();
	return 0;
}
