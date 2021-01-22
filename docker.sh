#!/bin/bash

docker build . -t abacus
docker run -v $(pwd):/abacus -w /abacus -it abacus bash