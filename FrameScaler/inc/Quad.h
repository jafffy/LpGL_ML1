#ifndef QUAD_H_
#define QUAD_H_

class QuadImpl;

class Quad
{
public:
	Quad();
	~Quad();

	void InitContents();
	void DestroyContents();
	void SetTexture(const char* path);

	void Draw();

private:
	QuadImpl* impl = nullptr;
};

#endif // QUAD_H_