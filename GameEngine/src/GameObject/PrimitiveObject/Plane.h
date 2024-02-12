#pragma once
#include "../VertexType.h"
#include <vector>
#include <memory>
#include "Primitive.h"

class Plane : public Primitive
{
public:
	Plane(float scale = 1.0f);
	~Plane();
	void Draw();
	void SetTexture();
};

