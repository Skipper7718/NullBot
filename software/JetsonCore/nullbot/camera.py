#NOTE The old gstCamera is not deleted, but commented to leave the option accessible in case cv2 breaks
#(It was extra compiled for gstStreamer support)

#from jetson.utils import gstCamera
from PIL import Image
import cv2
import numpy as np

#TODO comment test scripts and rewrite to use pipeline

#import sys
#sys.path.append("..")
from null_core import watchdog

__activated = False
__callback_function = None
cam = None
pipeline = "nvarguscamerasrc ! \
video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)NV12, framerate=(fraction)30/1 ! \
nvvidconv   flip-method=0 ! video/x-raw, width=(int)1280, height=(int)720, format=(string)BGRx ! \
videoconvert ! video/x-raw, format=BGR ! appsink drop=1"


def read_image() -> None:
    #img, _, _ = cam.CaptureRGBA(zeroCopy=1)
    #img = Image.fromarray(cv2.cvtColor(np.array(img).astype(np.uint8), cv2.COLOR_RGBA2RGB))
    #__callback_function(img)
    success, image = cam.read()
    if( success ):
        image = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
        __callback_function(image)

def camera_with_callback(callback) -> None:
    global __callback_function, cam, __activated
    if( not  __activated ):
        __callback_function = callback
        #cam = gstCamera(1280,720,"0")
        cam = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)
        cam.set(3, 1280)
        cam.set(4, 720)
        thread = watchdog.LoopThread(target=read_image, daemon=True, name="Camera_interrupt")
        thread.start()
        __activated = True
    else:
        raise Exception("You can only use one Camera allback")


#cam = cv2.VideoCapture(pipeline)
#import matplotlib.pyplot as plt
#cam = cv2.VideoCapture(__pipeline, cv2.CAP_GSTREAMER)
#print(cam.isOpened())
#succ, img = cam.read()
#cam.release()
#print(succ, img)
#plt.imshow(img)
#plt.show()
