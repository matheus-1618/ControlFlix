import pyautogui
import serial
import argparse
import time
import logging
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
from comtypes import CLSCTX_ALL
from ctypes import cast, POINTER
import numpy as np
from random_movie import movie
from threading import Thread
import unicodedata
import sys


#-----------------------------------------------------------#
#---------------------Task----------------------------------#
#-----------------------------------------------------------#
def task(name):
    time.sleep(1)
    movie(name)


#-----------------------------------------------------------#
#---------------------ControllerMap-------------------------#
#-----------------------------------------------------------#
class MyControllerMap:
    def __init__(self):
        self.button = {'Voltar 10s':'left','Pause/Play':'space',
        'Avançar 10s':'right'} # Fast forward (10 seg) pro Youtube

#-----------------------------------------------------------#
#---------------------BytesReceivedMap----------------------#
#-----------------------------------------------------------#
class BytesReceivedMap:
    def __init__(self) -> None:
        self.convert = {'Voltar 10s':b'R','Pause/Play':b'P',
        'Avançar 10s':b'A', 'Volume':b'V','Filme Aleatório':b'O' ,
        'EOP':b'X', 'START':b'W','ON/OFF':b'B','Handshake':b'Z', 'Sacrifice':b'K'}

#-----------------------------------------------------------#
#----------------Serial Interface Control-------------------#
#-----------------------------------------------------------#
class SerialControllerInterface:
    def __init__(self, port, baudrate):
        pyautogui.PAUSE = 0  ## remove delay
        #Serial port
        self.ser = serial.Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        #Bytes associated
        self.bytes = BytesReceivedMap()
        self.incoming = '0'
        self.data = '0'  
        self.bytes_to_read = 0
        self.low_8bits = b'0'
        self.hight_8bits = b'0'
        #volume control
        self.devices = AudioUtilities.GetSpeakers()
        self.interface = self.devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
        self.volume = cast(self.interface, POINTER(IAudioEndpointVolume))
        self.volMin,self.volMax = self.volume.GetVolumeRange()[:2]
        #thread associated
        self.movie_name = []
        self.thread = Thread(target=task, args=(self.movie_name, ))
        self.handshake = False
        self.movie_started = False
        self.device_is_on = True
    
    def disable(self):
        """Turn off the connection with the COM Port"""
        self.ser.close()
    
    def handshake_received(self):
        """Handshake between Uc and Device to be controlled"""
        while not self.handshake:
            if self.ser.read() !=  self.bytes.convert['Handshake']:
                self.ser.write(self.bytes.convert['Handshake'])
            else:
                logging.info("\nHandshake has been sent and received\n")
                self.handshake = True
    def reconnect(self,start):
        if start ==  self.bytes.convert['Sacrifice']:
            self.handshake = False
            self.handshake_received()

    def send_movie_name(self):
        """Send the name of the random movie sorted by the software"""
        if len(self.movie_name) != 0:
            if len(self.movie_name) >= 40:
                self.movie_name = self.movie_name[:39]
            for letra in self.movie_name:
                self.ser.write( unicodedata.normalize('NFKD', letra).encode('ascii', 'ignore'))
                time.sleep(.1)
            self.ser.write('#'.encode())            
            self.movie_name = []  

    def read_head(self):
        """Read the bytes of the head, ensuring reliability of the package 
        and the number of bytes of the payload to be read"""
        start = self.ser.read()
        while start != self.bytes.convert['START']:
            self.reconnect(start)
            start = self.ser.read()
            logging.info("Received START Wrong: {}".format(start)) 
        logging.info("Received START Correct") 
        byte = self.ser.read()
        self.bytes_to_read = int.from_bytes(byte  , "little")
        logging.info("Payload size: {} bytes".format(self.bytes_to_read)) 
        time.sleep(0.1)

    def read_payload(self):
        """Read the data of the payload and direct it to the right action
        to be taken after"""
        self.data = self.ser.read()
        if self.bytes_to_read == 3:
            logging.info("Received Volume DATA: {}".format(self.data))
            self.low_8bits = self.ser.read()
            self.hight_8bits = self.ser.read()
        else:
            logging.info("Received Button DATA: {}".format(self.data))
        self.control_actions()

    def read_eop(self):
        """Read the end of package, ensuring reliability of the package"""
        self.incoming = self.ser.read()
        while self.incoming != self.bytes.convert['EOP']:
            self.reconnect(self.incoming)
            self.incoming = self.ser.read()
            logging.info("Received EOP Wrong: {}".format(self.incoming)) 
        logging.info("Received EOP Correct") 
        logging.info("\n") 

    def control_volume(self, low_8bits, hight_8bits):
        """If received data related to volume, control the volume of the device"""
        length = (hight_8bits << 8) | low_8bits
        self.volume.SetMasterVolumeLevel(np.interp(length,[7500,8720],[self.volMin,self.volMax]) , None)
    
    def control_actions(self):
        """If received data related to another action, control it"""
        if self.data ==self.bytes.convert['Volume']:
            low_8bits = int.from_bytes(self.low_8bits  , "little")
            hight_8bits = int.from_bytes(self.hight_8bits, "little")
            self.control_volume(low_8bits,hight_8bits)
        if self.data ==self.bytes.convert['Filme Aleatório'] and not self.movie_started:
            logging.info("Escolhendo seu filme...")
            self.thread.start()
            self.movie_started = True
        elif self.data ==self.bytes.convert['Voltar 10s']:
            logging.info("Voltar 10s")
            pyautogui.keyDown('left')
        elif self.data == self.bytes.convert['Pause/Play']:
            logging.info("Pause/Stop")
            pyautogui.keyDown('space')
        elif self.data == self.bytes.convert['Avançar 10s']:
            logging.info("Avançar 10s")
            pyautogui.keyDown('right')
        elif self.data == self.bytes.convert['ON/OFF']:
            if self.device_is_on:
                logging.info("Dispositivo Desligado")
                self.device_is_on = False
            else:
                logging.info("Dispositivo Ligado")
                self.device_is_on = True  
        else:
            pyautogui.keyUp('right')
            pyautogui.keyUp('left')
            pyautogui.keyUp('space')

    def update(self):
        """Update the controller according the action decided"""
        self.send_movie_name()
        self.read_head()
        self.read_payload()
        self.read_eop()

if __name__ == '__main__':
    interfaces = ['dummy', 'serial']
    argparse = argparse.ArgumentParser()
    argparse.add_argument('serial_port', type=str)
    argparse.add_argument('-b', '--baudrate', type=int, default=115200)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    print("Connection to {} using {} interface ({})".format(args.serial_port, args.controller_interface, args.baudrate))
    controller = SerialControllerInterface(port=args.serial_port, baudrate=args.baudrate)

    while True:
        try:
            controller.update()
        except KeyboardInterrupt:
            print("\n---------------->Interrupção Forçada")
            controller.disable()
            break
