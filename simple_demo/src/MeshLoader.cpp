#include "MeshLoader.h"

class MeshLoaderImpl
{
public:
};

static MeshLoader g_Instance;

MeshLoader::MeshLoader()
{
	impl = new MeshLoaderImpl();
}

MeshLoader::~MeshLoader()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

MeshLoader& MeshLoader::Instance()
{
	return g_Instance;
}
