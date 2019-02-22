PACKAGE_FILE_NAME=simple_demo_lpgl
PACKAGE_NAME=com.jafffy.simple_demo_lpgl
MLCERT=/Users/jafffy/Dropbox/credentials/macos.cert

SECONDS=10
QUALITY=1080p
VIDEO_FILE_NAME=simple-demo-$(SECONDS)s.mp4

NUM_RECENT_LOGS=10

ML1_IP_ADDRESS=192.168.0.13
ML1_PORT=1131

POWER_ANALYIZER="/Users/jafffy/MagicLeap/tools/ptprofiler/1.0.0.20181206/Power and Thermal Profiler.app"

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

log:
	mldb log -T $(NUM_RECENT_LOGS)

wifi:
	mldb wifi status
	echo "Plug out your ML1 and ``make connect``."

connect:  # Could I make a dependency between wifi?
	mldb connect $(ML1_IP_ADDRESS):$(ML1_PORT)

power:
	open $(POWER_ANALYIZER)

clean:
	rm -rf .out/
