@echo off
docker build -t uvk5 .
docker run --rm -v %CD%\compiled-firmware:/app/compiled-firmware uvk5 /bin/bash -c "cd /app && make clean && make && cp UV_K5v1_ARDF* compiled-firmware/"
pause