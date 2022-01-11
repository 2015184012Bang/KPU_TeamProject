#pragma once

#include "Mesh.h"
#include "Texture.h"

struct MeshRendererComponent
{
	MeshRendererComponent(Mesh* mesh, Texture* texture)
		: Mesi(mesh)
		, Tex(texture) {}

	Mesh* Mesi;
	Texture* Tex;
};