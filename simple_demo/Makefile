PACKAGE_FILE_NAME=simple_demo_lpgl
PACKAGE_NAME=com.jafffy.simple_demo_lpgl
MLCERT=/Users/jafffy/Dropbox/credentials/macos.cert

SECONDS=15
QUALITY=720p
VIDEO_FILE_NAME=simple-demo-$(SECONDS)s.mp4

all:
	mabu $(PACKAGE_FILE_NAME).package -t lumin -s $(MLCERT)

host:
	mabu $(PACKAGE_FILE_NAME).package --invoke -t host

device:
	mabu $(PACKAGE_FILE_NAME).package --invoke -t lumin -s $(MLCERT)

terminate:
	mldb terminate $(PACKAGE_NAME)

video:
	mldb capture video -r -q $(QUALITY) -t $(SECONDS) $(VIDEO_FILE_NAME) # NOTE: -f doesn't work, but idk why it does.
