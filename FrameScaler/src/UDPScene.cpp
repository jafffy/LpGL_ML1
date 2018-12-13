#include "UDPScene.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>

#include <thread>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

class UDPSceneImpl
{
public:
	std::thread t;
};

UDPScene::UDPScene()
{
	impl = new UDPSceneImpl();
}

UDPScene::~UDPScene()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

#define BUFSIZE 8196
static char buf[BUFSIZE];
static const char* server_name = "192.168.137.1";
static const int server_port = 7080;

bool UDPScene::InitContents()
{
	srandom(time(0));

	for (int i = 0; i < BUFSIZE; ++i) {
		buf[i] = random() % 255 + 1;
	}

	impl->t = std::thread([&]() {
		struct sockaddr_in server_address;
		memset(&server_address, 0, sizeof(server_address));
		server_address.sin_family = AF_INET;
	
		inet_pton(AF_INET, server_name, &server_address.sin_addr);

		server_address.sin_port = htons(server_port);

		int sock;
		if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("could not create socket\n");
			return;
		}

		double timer = 0.0f;

		struct timeval t1, t2;

		gettimeofday(&t1, nullptr);

		while (timer < 120.0f) {
			struct timeval start_to_send;
			gettimeofday(&start_to_send, nullptr);

			// for (int i = 0; i < 1024; ++i) {
				int ret = sendto(sock, buf, BUFSIZE, 0,
					(struct sockaddr*)&server_address, sizeof(server_address));

				if (ret == -1) {
					char* err = strerror(errno);
					ML_LOG(Error, "error: %s", err);
				}
			// }

			struct timeval end_to_send;
			gettimeofday(&end_to_send, nullptr);

			{
				useconds_t elapsedTime = (end_to_send.tv_sec - start_to_send.tv_sec) * 1000000;
				elapsedTime += end_to_send.tv_usec - start_to_send.tv_usec;

				if (1000000 - elapsedTime > 0) {
					// usleep(1000000 - elapsedTime);
				}
			}

			gettimeofday(&t2, nullptr);

			double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
			elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

			timer = elapsedTime / 1000.0f;
		}

		close(sock);
	});

	return true;
}

void UDPScene::DestroyContents()
{

}

void UDPScene::OnRender(int cameraIndex, float dt)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void UDPScene::OnPressed()
{

}