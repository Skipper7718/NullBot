#!/usr/bin/env python3

from time import sleep
import sys

from nullbot import modules

bot = modules.DriveModule(0x56)

time = 1

while( True ):
    dir = input(">> ").strip().lower()
    if( dir == "1" ):
        bot.forward(time)
    elif( dir == "2" ):
        bot.left(time)
    elif( dir == "3" ):
        bot.right(time)
    elif( dir == "4" ):
        bot.reverse(time)
    elif( dir == "5" ):
        bot.simple_left(time)
    elif( dir == "6" ):
        bot.simple_right(time)

    elif( dir == "+" ):
        time += 0.2
        print(f"Increased sleep time to {time}")
    elif( dir == "-" ):
        time -= 0.2
        print(f"Decreased sleep time to {time}")
    
    elif( dir == "9" ):
        bot.set_level(int(input("Set new value: ")))

    elif( dir == "0" ):
        break
