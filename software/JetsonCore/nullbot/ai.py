#TODO do something with the other locks and finish avoidancenet

import jetson.inference
import jetson.utils
from numpy.lib.type_check import imag
import torch
import tensorflow as tf
import tensorflow.keras as keras
import numpy as np
import cv2

from PIL import Image
from neural import ObstacleNet as obs_net
import watchdog, modules

__stack_lock = False

class StackNetwork:
    image = False
    #class types that can go into the AI stack

#AI Stack
__auto_image_stack = list()
__auto_stack = list()

def add_to_stack(network:object) -> None:
    if( isinstance(network, StackNetwork )):
        if( network.image ):
            __auto_image_stack.append(network)
        else:
            __auto_stack.append(network)

def run_stack(*args) -> None:
    global __stack_lock
    # wait for lock to clean
    while( not __stack_lock ):
        pass
    __stack_lock = True

    if( isinstance(args[0], Image) ):
        for net in __auto_image_stack:
            net.ai_stack(args[0])
    else:
        for net in __auto_stack:
            net.ai_stack(*args)
    
    __stack_lock = False


class ObstacleNet(StackNetwork):
    image = True
    def __init__(self, pretrained: bool = True) -> None:
        self.labels = labels = ["free", "blocked", "obstacle"]
        obs_net.get_model(pretrained)
    
    def predict(self, img:Image) -> str:
        label = obs_net.run_inference(img)
        return self.labels[label]
    
    def ai_stack(self, img:Image) -> None:
        print(self.labels)
        #dummy function

class AvoidanceNet(StackNetwork):
    image = True
    def __init__(self) -> None:
        self.model = jetson.inference.detectNet("ssd-mobilenet-v2")
    
    def predict(self, img:Image) -> list:
        detections = self.model.Detect(jetson.utils.cudaFromNumpy(cv2.cvtColor(np.array(img), cv2.COLOR_BGR2RGBA)), 1280, 720)
        data = [(det.Center, det.Width, det.Height) for det in detections]
        return data
    
    def predict_raw(self, img:Image) -> list:
        detections = self.model.Detect(jetson.utils.cudaFromNumpy(cv2.cvtColor(np.array(img), cv2.COLOR_BGR2RGBA)), 1280, 720)
        return detections

    def ai_stack(self, img:Image) -> None:
        pass
        #ai_stack function to get triggered and calculate where to go based on results