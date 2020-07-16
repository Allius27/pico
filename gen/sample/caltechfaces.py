#
import sys
import os
import random
import numpy
import cv2
import struct

#
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('src', help='CaltechFaces source folder')
args = parser.parse_args()

#
#
#

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
	sys.stdout.buffer.write(hw)
	sys.stdout.buffer.write(pixels)

#
#
#

def write_sample_to_stdout(img, bboxes):
	#
	if len(img.shape)==3:
		img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
	#
	write_rid_to_stdout(img)

	sys.stdout.buffer.write( struct.pack('i', len(bboxes)) )

	for box in bboxes:
		sys.stdout.buffer.write( struct.pack('iii', box[0], box[1], box[2]) )

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
	for i in range(0, 8):
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
		write_sample_to_stdout(resized_img, resized_bboxes)
		#visualize_bboxes(resized_img, resized_bboxes)


def saveMouth(img, mouthArea, filename):

	height = mouthArea[0][3] - mouthArea[0][1]
	width = mouthArea[0][2] - mouthArea[0][0]

	if height < 30 or width < 50:
		return

	roi = img[mouthArea[0][1]:mouthArea[0][3],mouthArea[0][0]:mouthArea[0][2]]

	if not os.path.isdir("mouth"):
		os.mkdir("mouth")

	cv2.imwrite("mouth/" + filename.split("/")[1], roi )

	# show picture
	# cv2.imshow("asd", roi)
	# cv2.waitKey(0)

#
#
#

annots = open(os.path.join(args.src, 'WebFaces_GroundThruth.txt'), 'r')
imgpaths = []
faces = []
dict = {}

for line in annots.readlines():
	#
	if line.strip() != '':
		imgname = line.split()[0]
		if imgname in dict:
			i = dict[imgname]
			faces[i].append([float(x) for x in line.split()[1:]])
		else:
			dict[imgname] = len(imgpaths)
			imgpaths.append(os.path.join(args.src, imgname))
			faces.append([[float(x) for x in line.split()[1:]]])

#
#
#

for i in range(0, len(imgpaths)):
	#
	img = cv2.imread(imgpaths[i])
	#
	bboxes = []
	mouthArea = []

	for face in faces[i]:
		#
		eyedist = ( (face[0]-face[2])**2 + (face[1]-face[3])**2 )**0.5
		r = (face[1]+face[3])/2.0 + 0.25*eyedist
		c = (face[0]+face[2])/2.0
		s = 2.0*1.5*eyedist
		#
		bboxes.append((r, c, s))

		dY = (int(face[7]) - int(face[5]))

		mouthAreaX = int(face[0] - eyedist * 0.25)
		mouthAreaY = int(face[5] + dY / 2)
		mouthAreaW = int(face[2] + eyedist * 0.25)
		mouthAreaH = int(face[7] + dY / 2)

		mouthArea.append((mouthAreaX, mouthAreaY, mouthAreaW, mouthAreaH))

	#
	export_img_and_boxes(img, bboxes)
	saveMouth(img, mouthArea, imgpaths[i])
