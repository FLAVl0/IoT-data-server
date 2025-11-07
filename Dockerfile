# Simple Dockerfile to set up a test environment using Debian stable slim

FROM debian:stable-slim

RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		gcc \
		make \
		libsqlite3-dev \
		libbluetooth-dev \
		libdbus-1-dev && \
	rm -rf /var/lib/apt/lists/*