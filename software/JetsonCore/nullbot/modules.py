from typing import Union
import serial
from time import sleep
from serial.serialposix import Serial
import yaml, os

global __coprocessor_access
__coprocessor_access = None

with open(os.path.join(os.path.dirname(__file__), "./config/instruction_set.yml"), "r") as file:
    instruction_set = yaml.load(file)

class __SerialController:
    def __init__(self, port:str="/dev/ttyUSB0", baud:int=115200, timeout:int=None) -> None:
        self.device = serial.Serial(port, baud, timeout=timeout)
    
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
    
    def display_error(self):
        self.write([instruction_set["display"]["error"]])

    def display_thread_failure(self):
        self.write([instruction_set["display"]["thread_failure"]])

    def display_all_running(self):
        self.write([instruction_set["display"]["all_running"]])

#load configuration
with open(os.path.join(os.path.dirname(__file__), "./config/coprocessor-config.yml"), "r") as file:
    __coprocessor_config = yaml.load(file)
if(__coprocessor_access == None):
    __coprocessor_access = __SerialController(__coprocessor_config["port"], __coprocessor_config["baudrate"])


def get_controller() -> __SerialController:
    return __coprocessor_access


### TEST ###
#esp = __SerialController()
#print("TESTING DISPLAY MODES")
#esp.display_thread_failure()
#sleep(1)
#esp.display_error()
#sleep(1)
#esp.display_all_running()
#print("TESTING WRITE")
#esp.write_address(0x31, [0x20])
#sleep(1)
#print(f"READ ADDRESS TEST (using echo address) {[hex(x) for x in esp.read_address(0x31, 1)]}")
#sleep(1)
#print(f"AVAILABLE ADDRESSES {[hex(x) for x in esp.read_scan()]}")