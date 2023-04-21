import websocket
from datetime import datetime
import time
import _thread as thread
from calcs import *

class websocket_data:
    def __init__(self):
        pass

    def onMessage(self, msg):
        m = msg.split(':')
        if len(m) == 2:
            self.cb(m[0], m[1])
        else:
            self.cb(msg, None)

    def onError(self, msg, test):
        print(f"Websocket error: {msg}") # {test}")

    def onOpen(self):
        print(f"Websocket open")

    def onClose(self):
        print("Websocket closed.")

    def connect(self, url):
        self.url = url
        #websocket.enableTrace(True)
        self.ws = websocket.WebSocketApp(self.url,
                                         on_message=self.onMessage,
                                         on_error=self.onError,
                                         on_open=self.onOpen,
                                         on_close=self.onClose)
           
        thread.start_new_thread(lambda *args: self.ws.run_forever(), ())

    def disconnect(self):
        self.ws.close()

    def register_cb(self, func):
        self.cb = func

    def toggle(self):
        self.ws.send("toggle")

    def set_slider(self, value):
        svg = SvgHandler(value)
        self.ws.send(f"sli:{svg.interpret()}")

    def set_kx(self, x, value):
        self.ws.send(f"pid_{x}:{value}")

    def request_led_state(self):
        self.ws.send(f"led_state:?") 

    # def request_slider(self):
    #     self.ws.send(f"sli:?") 
    
    def request_kx(self, x):
        self.ws.send(f"pid_{x}:?")

def my_cb(cmd, parm):
    print(f'command: {cmd} parameter: {parm}')

def test_class():
    wsd = websocket_data()
    wsd.register_cb(my_cb)
    wsd.connect('ws://192.168.10.147:1337')

    kp = 0
    while True:
        time.sleep(1.0)
        wsd.set_kx('kp', kp)
        kp += 0.1
        
if __name__ == "__main__":
    test_class()
