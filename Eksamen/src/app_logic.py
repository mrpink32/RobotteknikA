import app_data

class app_logic(app_data.websocket_data):

    def __init__(self):
        self.register_cb(self.callback)
        self.connect('ws://192.168.4.1:1337')

    def callback(self, cmd, value):
        if value == '?':
            return
        if cmd=='1':
            self.onoff_cb(1)
        elif cmd=='0':
            self.onoff_cb(0)
        elif cmd=='led_state':
            self.led_state_cb(value)  
        elif cmd=='sli':
            self.sli_cb(value)        
        elif cmd=='pid_kp':
            self.kx_cb(value, 'kp')    
        elif cmd=='pid_ki':
            self.kx_cb(value, 'ki')    
        elif cmd=='pid_kd':
            self.kx_cb(value, 'kd')    
        else:
            print(cmd, value)

    def onoff_pressed(self):
        self.toggle()
    
    def register_led_state_cb(self, cb_func):
        self.led_state_cb = cb_func

    def register_onoff_cb(self, cb_func):
        self.onoff_cb = cb_func

    def sli_change(self, value):
        self.set_slider(value)

    def register_sli_cb(self, cb_func):
        self.sli_cb = cb_func
    
    def register_kx_cb(self, cb_func):
        self.kx_cb = cb_func
    
