#!/usr/bin/env python

from nullbot import modules, camera
from time import sleep

class MyModule(modules.Base):
    def __init__(self) -> None:
        super().__init__(0x54)

    def on(self):
        self.send(0x01)
    
    def off(self):
        self.send(0x02)

if __name__ == "__main__":
    led = MyModule()
    while(True):
        led.on()
        sleep(1)
        led.off()
        sleep(1)