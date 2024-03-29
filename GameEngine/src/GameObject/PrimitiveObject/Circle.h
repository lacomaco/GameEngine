#pragma once
#include "../MeshMaterialsLight.h"
#include <vector>
#include <memory>
#include "Primitive.h"

class Circle : public Primitive
{
public:
	Circle(float radius = 0.3f,
		int numSlices = 100,
		int numStacks = 100);
	~Circle();
	void Draw(const char* shaderProgramName);
};

