#pragma once

using namespace DirectX;



Vertex* CreateTriangleVertex()
{
	Vertex* vertices = new Vertex[3]
	{
		{ { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f, -1.0f}, {0.5f, 1.0f}},
		{ { 1.0f, -1.0f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 1.0f, 1.0f} },
		{ { -1.0f, -1.0f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 0.0f, 0.0f} }
	};
	return vertices;
}

Vertex* CreateSquareVertex()
{
	Vertex* vertices = new Vertex[6]
	{
		{ { -0.5f, 0.5f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 0.0f, 0.0f} },
		{ { 0.5f, 0.5f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 1.0f, 0.0f} },
		{ { 0.5f, -0.5f, 0.0f },{0.0f, 0.0f, -1.0f}, { 1.0f, 1.0f} },

		{ { -0.5f, 0.5f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 0.0f, 0.0f} },
		{ { 0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 1.0f, 1.0f} },
		{ { -0.5f, -0.5f, 0.0f }, {0.0f, 0.0f, -1.0f}, { 0.0f, 1.0f} }
	};

	return vertices;
}

