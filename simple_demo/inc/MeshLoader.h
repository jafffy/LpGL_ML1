#ifndef MESHLOADER_H_
#define MESHLOADER_H_

class MeshLoaderImpl;

class MeshLoader
{
public:
	MeshLoader();
	~MeshLoader();

	static MeshLoader& Instance();

private:
	MeshLoaderImpl* impl = nullptr;
};

#endif // MESHLOADER_H_