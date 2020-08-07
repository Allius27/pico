#!/bin/bash

set -e
set -x

pushd ..
	make 
	cp -f picolrn sample
popd

python3 dlibPoints.py caltechfaces/

python3 caltechfaces.py face > trface
python3 caltechfaces.py mouth > trmouth
python3 caltechfaces.py eyes > treyes

./picolrn trface ../../rnt/cascades/_facefinder
./picolrn trmouth ../../rnt/cascades/_mouthfinder
./picolrn treyes ../../rnt/cascades/_eyesfinder

echo "finish"