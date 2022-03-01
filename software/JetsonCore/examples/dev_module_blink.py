#!/usr/bin/env python3

import sys
sys.path.append("./NullBot/software/JetsonCore")
sys.path.append("./NullBot/software/JetsonCore/nullbot")

from nullbot import modules
from time import sleep

class MyModule(modules.Base):
    def __init__(self) -> None:
        super().__init__(0x1a)

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
