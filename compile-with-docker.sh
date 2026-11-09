#!/bin/sh
docker --debug build -t uvk5 .
docker run --rm -v ${PWD}/compiled-firmware:/app/compiled-firmware uvk5 /bin/bash -c "cd /app && make clean && make -j 14 && cp UV_K5v1_ARDF* compiled-firmware/"
