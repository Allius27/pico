#
import sys
import os
import random
import numpy
import cv2
import struct
from os import listdir
from os.path import isfile, join
from pathlib import Path
import time



#
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('src', help='Source folder with faces')
parser.add_argument('dstFile', help='Destination database')
parser.add_argument('multiplier', help='Base increase factor', type=int, default=8)
args = parser.parse_args()

#
#
#

def getSquare(area):
	height = area[3] - area[1]
	width = area[2] - area[0]

	maxSide = max(height, width)

	centerX = (area[0] + area[2])/2
	centerY = (area[1] + area[3])/2

	newArea = (int(centerX - maxSide/2),
			   int(centerY - maxSide/2),
			   int(centerX + maxSide/2),
			   int(centerY + maxSide/2))

	return newArea

def checkMinSize(area, minSize):
	if ((area[2] - area[0]) < minSize) or \
		((area[3] - area[1]) < minSize):
		return False
	return True

def write_rid_to_stdout(im):
	#
	# raw intensity data
	#

	#
	h = im.shape[0]
	w = im.shape[1]

	#
	hw = struct.pack('ii', h, w)
	pixels = struct.pack('%sB' % h*w, *im.reshape(-1))

	#
	writeFile.write(hw)
	writeFile.write(pixels)
	# sys.stdout.buffer.write(hw)
	# sys.stdout.buffer.write(pixels)

#
#
#

def write_sample_to_file(img, bboxes):
	#
	if len(img.shape)==3:
		img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
	#
	write_rid_to_stdout(img)

	writeFile.write( struct.pack('i', len(bboxes)) )
	# sys.stdout.buffer.write( struct.pack('i', len(bboxes)) )

	for box in bboxes:
		writeFile.write( struct.pack('iii', box[0], box[1], box[2]) )
		# sys.stdout.buffer.write( struct.pack('iii', box[0], box[1], box[2]) )

#
#
#

def visualize_bboxes(img, bboxes):
	#
	for box in bboxes: cv2.circle(img, (int(box[1]), int(box[0])), int(box[2]/2.0), (0, 0, 255), thickness=2)
	cv2.imshow('...', img)
	cv2.waitKey(0)

def export_img_and_boxes(img, bboxes):
	#
	for i in range(0, args.multiplier):
		#
		scalefactor = 0.7 + 0.6*numpy.random.random()
		#
		resized_img = cv2.resize(img, (0, 0), fx=scalefactor, fy=scalefactor)

		flip = numpy.random.random() < 0.5
		if flip:
			resized_img = cv2.flip(resized_img, 1)

		resized_bboxes = []
		for box in bboxes:
			#
			if flip:
				resized_box = (int(scalefactor*box[0]), resized_img.shape[1] - int(scalefactor*box[1]), int(scalefactor*box[2]))
			else:
				resized_box = (int(scalefactor*box[0]), int(scalefactor*box[1]), int(scalefactor*box[2]))
			#
			if resized_box[2] >= 24:
				resized_bboxes.append(resized_box)
		#
		write_sample_to_file(resized_img, resized_bboxes)
		# visualize_bboxes(resized_img, resized_bboxes)


def saveMouth(img, mouthArea, filename):
	square = getSquare(mouthArea)

	if not checkMinSize(square, 25):
		return

	roi = img[square[1]:square[3],square[0]:square[2]]

	if not os.path.isdir("mouth"):
		os.mkdir("mouth")

	cv2.imwrite("mouth/" + filename.split("/")[1], roi )

	# show picture
	# cv2.imshow("asd", roi)
	# cv2.waitKey(0)

#
#
#
onlyfiles = list(Path(args.src).rglob("*.[j][p][g]"))
#
#
#

writeFile = open(args.dstFile, 'wb')


for index in range(0, len(onlyfiles)):
	
	#
	img = cv2.imread(str(onlyfiles[index]))
	#
	bboxes = [(int(img.shape[0]/2), int(img.shape[1]/2), int(img.shape[0]))]
	#
	export_img_and_boxes(img, bboxes)
	
	if ( not ( index % 1000) and index != 0 ):
		print ( "Process", str(index) + "/" + str(len(onlyfiles)) )
