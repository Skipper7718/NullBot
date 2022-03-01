#!/usr/bin/env python3

import sys

from PIL import Image
from nullbot import ai, camera

with open("coco_labels.txt", "r") as file:
    labels = file.readlines()
    for idx, label in enumerate(labels):
        labels[idx] = label.strip().split()[1]

print(labels)
input(">>> 0x0B >>> Continue? ")

model = ai.AvoidanceNet()

def callback(img: Image) -> None:
    data = model.predict(img)
    for i in data:
        try:
            print(f"Detected class: {labels[i.ClassID-1]}")
        except:
            print("Error")

camera.camera_with_callback(callback)

while( True ):
    pass
