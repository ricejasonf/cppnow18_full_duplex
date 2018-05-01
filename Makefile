platform:
	docker build -f docker/Dockerfile-platform -t cppnow18_full_duplex_platform .

image_develop:
	cppdock build linux_x64 \
	&& docker build -f ./docker/Dockerfile-develop -t cppnow18_full_duplex:develop .

develop:
	docker run --rm -it -v ${shell pwd}:/opt/src:ro cppnow18_full_duplex:develop
