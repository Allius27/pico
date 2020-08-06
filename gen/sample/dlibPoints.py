#
import sys
import os
import random
import numpy as np
import cv2
import struct
import dlib
from os import listdir
from os.path import isfile, join

#
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('src', help='CaltechFaces source folder')
args = parser.parse_args()

dlib_predictor = "../../../../../../faceRecognitionOveview_build/install/shape_predictor_68_face_landmarks.dat"

if not os.path.isfile(dlib_predictor):
	print ("dlib predictor does not exist")
	exit(2)

detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(dlib_predictor)

def getRect(vecX, vecY):
	tx = min(vecX)
	ty = min(vecY)
	bx = max(vecX)
	by = max(vecY)

	return (tx,ty,bx,by)

def saveMouthRect(shape, fileName):
	vecX = []
	vecY = []
	for b in range(48,68):
		vecX.append(shape.part(b).x)
		vecY.append(shape.part(b).y)

	rect = getRect(vecX, vecY)

	writeString = fileName + " " \
				  		   + str(rect[0]) + " " \
						   + str(rect[1]) + " "\
						   + str(rect[2]) + " "\
						   + str(rect[3]) + "\n"

	mouthFile.write(writeString)

onlyfiles = [f for f in listdir(args.src) if isfile(join(args.src, f)) and f.endswith(".jpg")]

mouthFile = open("mouthDlibMeta.txt", 'w')

for i in range(0, len(onlyfiles)):
	print("Process ", str(i) + "/" + str(len(onlyfiles)))

	file = onlyfiles[i]
	img = cv2.imread(args.src + "/" + file)
	dets = detector(img, 1)
	for k, d in enumerate(dets):
		shape = predictor(img, d)

		saveMouthRect(shape, file)


mouthFile.close()
