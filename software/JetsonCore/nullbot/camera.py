#NOTE The old gstCamera is not deleted, but commented to leave the option accessible in case cv2 breaks
#(It was extra compiled for gstStreamer support)

#from jetson.utils import gstCamera
from PIL import Image
import cv2
import numpy as np

from null_core import watchdog

__activated = False
__callback_function = None
cam = None
pipeline = "nvarguscamerasrc ! \
video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)NV12, framerate=(fraction)30/1 ! \
nvvidconv   flip-method=0 ! video/x-raw, width=(int)1280, height=(int)720, format=(string)BGRx ! \
videoconvert ! video/x-raw, format=BGR ! appsink drop=1"

def cam_condition(wrap):
    global cam
    if( cam == None ):
        cam = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)
        cam.set(3, 1280)
        cam.set(4, 720)
    def __inner(*args):
        wrap(*args)
    return __inner

@cam_condition
def read_image() -> None:
    #img, _, _ = cam.CaptureRGBA(zeroCopy=1)
    #img = Image.fromarray(cv2.cvtColor(np.array(img).astype(np.uint8), cv2.COLOR_RGBA2RGB))
    #__callback_function(img)
    success, image = cam.read()
    if( success ):
        image = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
        __callback_function(image)

@cam_condition
def camera_with_callback(callback) -> None:
    global __callback_function, __activated
    if( not  __activated ):
        __callback_function = callback
        #cam = gstCamera(1280,720,"0")
        thread = watchdog.LoopThread(target=read_image, daemon=True, name="Camera_interrupt")
        thread.start()
        __activated = True
    else:
        raise Exception("You can only use one Camera callback")