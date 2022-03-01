#!/usr/bin/env python3

#### This is an example of how the drive model can interact with the camera

import sys
from PIL import Image
from program1 import fill_led, rainbow
from nullbot import ai, camera, modules

with open("coco_labels.txt", "r") as file:
    labels = file.readlines()
    for idx, label in enumerate(labels):
        labels[idx] = label.strip().split()[1]
print(labels)
print("NullBot: This is only a test. Very unstable")
input(">>> Continue? ")
fill_led(0,0,120)

pred_x = 0
model = ai.AvoidanceNet()

def callback(img: Image) -> None:
    global pred_x
    predictions = model.predict(img)
    #data = [(det.Center, det.Width, det.Height) for det in detections]
    if( len(predictions)  > 0):
        print(predictions[0])
        pred_x = predictions[0].Center[0]
    else:
        pred_x = 0

print("Initializing done.")
camera.camera_with_callback(callback)
bot = modules.DriveModule()

while( True ):
    print(f"X value >> {pred_x}")
    if( pred_x != 0 ):
        fill_led(250,0,0)
        bot.reverse(0.3)
        if( pred_x < 640 ):
            bot.right(0.8)
        else:
            bot.left(0.8)
    else:
        fill_led(0,250,50)
        bot.forward(0.8)
