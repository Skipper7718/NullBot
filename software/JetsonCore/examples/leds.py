#!/usr/bin/env python3

from time import sleep
import sys

from nullbot import modules

def set_led( led:int, r:int, g:int, b:int ) -> None:
    modules._coprocessor.led_set(led, r, g, b,)

def fill_led( r:int, g:int, b:int ) -> None:
    modules._coprocessor.led_fill(r, g, b)

def rainbow() -> None:
    modules._coprocessor.led_fill(0,0,0)
    modules._coprocessor.led_set(0,250,0,0)
    modules._coprocessor.led_set(1,250,50,0)
    modules._coprocessor.led_set(2,250,180,0)
    modules._coprocessor.led_set(3,0,250,0)
    modules._coprocessor.led_set(4,0,250,250)
    modules._coprocessor.led_set(5,0,0,250)
    modules._coprocessor.led_set(6,250,0,250)
    modules._coprocessor.led_set(7,250,0,250)
    modules._coprocessor.led_set(8,0,0,250)
    modules._coprocessor.led_set(9,0,250,250)
    modules._coprocessor.led_set(10,0,250,0)
    modules._coprocessor.led_set(11,250,180,0)
    modules._coprocessor.led_set(12,250,50,0)
    modules._coprocessor.led_set(13,250,0,0)

if( __name__ == "__main__" ):
    if( len(sys.argv) < 2 ):
        print("./program1.py fill / single / rainbow")
        exit()
    if( sys.argv[1].lower() == "single" ):
        led = int(input("LED:\t"))
        red = int(input("Red:\t"))
        green = int(input("Green:\t"))
        blue = int(input("Blue:\t"))
        set_led( led, red, green, blue )
    elif( sys.argv[1].lower() == "fill" ):
        red = int(input("Red:\t"))
        green = int(input("Green:\t"))
        blue = int(input("Blue:\t"))
        fill_led( red, green, blue )
    elif( sys.argv[1].lower() == "rainbow" ):
        rainbow()
