from imp import find_module
from typing import Union
import serial
from time import sleep
from serial.serialposix import Serial
import yaml, os

global __coprocessor_access
_coprocessor = None

global found_modulpasses
found_modules = None

with open(os.path.join(os.path.dirname(__file__), "./config/instruction_set.yml"), "r") as file:
    instruction_set = yaml.load(file)

class __SerialController:
    def __init__(self, port:str="/dev/ttyUSB0", baud:int=115200, timeout:int=None) -> None:
        self.device = serial.Serial(port, baud, timeout=timeout)
        self.locked = True
    
    def write(self, command:list) -> None:
        command.append(instruction_set["stop"])
        self.device.write(bytearray(command))

    def read(self) -> bytes:
        return self.device.read_until(bytearray([instruction_set["stop"]]))[:-1]

    def read_scan(self) -> Union[list, None]:
        self.write([instruction_set["read_scan"]])
        response = self.read()
        num_bytes = response[0]
        if( len(response) > 0 ):
            return list(response[1:])
        else:
            return None
    
    def write_address(self, address:int, payload:list) -> None:
        self.write([instruction_set["write_address"]] + [address] + payload)

    def read_address(self, address:int, num_bytes:int) -> Union[list, None]:
        self.write([instruction_set["read_address"], address, num_bytes])
        result = self.read()
        if( len(result) <= 0):
            return None
        else:
            return list(result)
    
    def clear_i2c(self, address:int):
        self.write([instruction_set["clear_i2c"]])
    
    def led_set(self, led_num:int, r:int, g:int, b:int) -> None:
        if( ((r+g+b) - 765) <= 0 and ((r+g+b) - 765) >= -765 ): # if all values are between 0 and 255
            self.write([instruction_set["led_set"], led_num, r, g, b])

    def led_fill(self, r:int, g:int, b:int) -> None:
        if( ((r+g+b) - 765) <= 0 and ((r+g+b) - 765) >= -765 ): # if all values are between 0 and 255
            self.write([instruction_set["led_fill"], r, g, b])
    
    def display_error(self):
        self.write([instruction_set["display"]["error"]])

    def display_thread_failure(self):
        self.write([instruction_set["display"]["thread_failure"]])

    def display_all_running(self):
        self.write([instruction_set["display"]["all_running"]])

#load configuration
with open(os.path.join(os.path.dirname(__file__), "./config/coprocessor-config.yml"), "r") as file:
    __coprocessor_config = yaml.load(file)
if(_coprocessor == None):
    _coprocessor = __SerialController(__coprocessor_config["port"], __coprocessor_config["baudrate"])
    while( found_modules == None ):
        found_modules = _coprocessor.read_scan()
    _coprocessor.locked = False

def wait_lock(func):
    def __inner(*args):
        while( _coprocessor.locked ):
            pass
        _coprocessor.locked = True
        func(*args)
        _coprocessor.locked = False
    return __inner

class Base:
    def __init__(self, address: int) -> None:
        self.address = address
        if( self.address not in found_modules ):
            _coprocessor.display_error()
            raise Exception("Module not found")

    @wait_lock
    def send(self, *payload):
        _coprocessor.write_address(self.address, list(payload))
    
    @wait_lock
    def receive(self, num_bytes: int):
        ret = _coprocessor.read_address(self.address, num_bytes)
        return ret

# Prebuilt nullbot modules
class DriveModule(Base):
    """"Module for basic driving around - H style voltage regulators"""
    def __init__(self, address: int = 0x51) -> None:
        super().__init__(address)
        self.v_calibration: int

    def stop(self, delay:int = 0) -> None:
        self.send(0x01)

    def set_level(self, level:int) -> None:
        if(level >= 0 and level <= 255):
            self.send(0x08, level)
        else:
            raise Exception(f"Level cannot be {level}. Range is 0-255 (0x00 - 0xff)")

    def forward(self, duration:int = 0) -> None:
        self.send(0x02)
        if( duration ):
            sleep(duration)
            self.stop()
        
    def reverse(self, duration:int = 0) -> None:
        self.send(0x05)
        if( duration ):
            sleep(duration)
            self.stop()

    def right(self, duration:int = 0) -> None:
        self.send(0x03)
        if( duration ):
            sleep(duration)
            self.stop()

    def left(self, duration:int = 0) -> None:
        self.send(0x04)
        if( duration ):
            sleep(duration)
            self.stop()

    def simple_right(self, duration:int = 0) -> None:
        self.send(0x06)
        if( duration ):
            sleep(duration)
            self.stop()

    def simple_left(self, duration:int = 0) -> None:
        self.send(0x07)
        if( duration ):
            sleep(duration)
            self.stop()