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
import subprocess

#
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('src', help='CaltechFaces source folder')
args = parser.parse_args()

dlib_predictor = "shape_predictor_68_face_landmarks.dat"

# visualize = True
visualize = False

if not os.path.isfile(dlib_predictor):
	if not os.path.isfile(dlib_predictor + ".bz2"):
		subprocess.call(['wget', 'http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2'])
	subprocess.call(['bzip2', "-d", 'shape_predictor_68_face_landmarks.dat.bz2'])

if not os.path.isfile(dlib_predictor):
	print ("dlib predictor does not exist")
	exit(2)

detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(dlib_predictor)

def getSquare(rect):
	height = rect[3] - rect[1]
	width = rect[2] - rect[0]

	maxSide = max(height, width)

	centerX = (rect[0] + rect[2]) / 2
	centerY = (rect[1] + rect[3]) / 2

	newRect = (int(centerX - maxSide / 2),
			   int(centerY - maxSide / 2),
			   int(centerX + maxSide / 2),
			   int(centerY + maxSide / 2))

	return newRect


def getRect(vecX, vecY):
	tx = min(vecX)
	ty = min(vecY)
	bx = max(vecX)
	by = max(vecY)

	return (tx,ty,bx,by)



def getStringFromArray(array):
	string = " "
	for i in range(0, len(array)):
		string += str(array[i]) + " "
	return string

def getDlibRect(img, shape, min:int, max:int):
	vecX = []
	vecY = []
	for b in range(min, max):
		vecX.append(shape.part(b).x)
		vecY.append(shape.part(b).y)

	square = getSquare(getRect(vecX, vecY))
	img = drawRect(img, square)

	return img, square

def saveRect(img, rect, filename, directoryForSave, mirror=False):
	roi = img[rect[1]:rect[3], rect[0]:rect[2]]

	if mirror:
		roi = cv2.flip(roi, 1)

	if not os.path.isdir(directoryForSave):
		os.mkdir(directoryForSave)

	cv2.imwrite(directoryForSave + "/" + filename, roi)

def drawRect(img, rect):
	# visualize
	if visualize:
		cv2.rectangle(img, (rect[0], rect[1]), (rect[2],rect[3]), (255,255,0))
	return img

onlyfiles = [f for f in listdir(args.src) if isfile(join(args.src, f)) and f.endswith(".jpg")]

writeFile = open(args.src +"/dlibMeta.txt", 'w')

for i in range(0, len(onlyfiles)):
	print("Process ", str(i+1) + "/" + str(len(onlyfiles)))

	file = onlyfiles[i]
	img = cv2.imread(args.src + "/" + file)
	dets = detector(img, 1)
	for k, d in enumerate(dets):
		shape = predictor(img, d)

		imgViz = img

		imgViz, faceRect  = getDlibRect(imgViz, shape, 0, 68)
		imgViz, mouthRect = getDlibRect(imgViz, shape, 48, 68)
		imgViz, leftEye   = getDlibRect(imgViz, shape, 42, 48)
		imgViz, rightEye  = getDlibRect(imgViz, shape, 36, 42)

		writeString = file \
					  + getStringFromArray(faceRect) \
					  + getStringFromArray(mouthRect) \
					  + getStringFromArray(leftEye) \
					  + getStringFromArray(rightEye) +"\n"

		# writeFile.write(writeString)

		filenameBase = file.split(".")[0] + "_" + str(k)
		filenameEnd  = file.split(".")[1]
		filename     = filenameBase + "."    + filenameEnd
		filenameRE   = filenameBase + "_RE." + filenameEnd
		filenameLE   = filenameBase + "_LE." + filenameEnd

		saveRect(img, faceRect, filename, "face")
		saveRect(img, mouthRect, filename, "mouth")
		saveRect(img, leftEye, filenameLE, "eyes")
		saveRect(img, rightEye, filenameRE, "eyes")

		if visualize:
			cv2.imshow("image", imgViz)
			cv2.waitKey(0)
