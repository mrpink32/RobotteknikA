# pip install websocket-client
# pip install websocket

from tkinter import *
from tkinter.font import Font
import app_logic



class UI(Frame):
    def __init__(self, root, logic):
        Frame.__init__(self, root, bg='white', bd=0, width=100, height=100)
        self.logic = logic
        self.helv28 = Font(family="Helvetica", size=27)
        self.helv24 = Font(family="Helvetica", size=24)
        self.helv16 = Font(family="Helvetica", size=14)
        self.helv16b = Font(family="Helvetica", size=12, weight='bold')

    def render_header(self):
        lbl_head = Label(self, text=' Robot Control Panel', anchor="w",
                         font=self.helv28, height=1, bd=16, bg="#009688", fg="white")
        lbl_action = Label(self, text='Action', bg='white', font=self.helv16b)
        lbl_status = Label(self, text='Status', bg='white', font=self.helv16b)
        lbl_head.grid(row=0, column=0, columnspan=2, sticky=N+S+E+W)
        # grid
        lbl_action.grid(row=1, column=0, sticky=W, padx=16)
        lbl_status.grid(row=1, column=1, sticky=W)

    def render_onoff(self):
        cvs_stat = Canvas(self, width=36, height=36, bg='white',
                          bd=0, highlightthickness=0, relief='ridge')
        oval = cvs_stat.create_oval(1, 1, 35, 35, fill="black", width=2)

        btn_onoff = Button(self, text='Turn on', width=12,
                           command=self.logic.onoff_pressed, bd=2, relief="ridge")
        
        def cb(value):
            if int(value) == 0:
                btn_onoff.configure(text='Turn on')
                cvs_stat.itemconfig(oval, fill="black")
            else:
                btn_onoff.configure(text='Turn off')
                cvs_stat.itemconfig(oval, fill="red")
        cb('0')
        self.logic.register_onoff_cb(cb)
        self.logic.register_led_state_cb(cb)

        # grid
        btn_onoff.grid(row=2, column=0, sticky=W, padx=16)
        cvs_stat.grid(row=2, column=1, sticky=W)

    def render_slider(self):
        ntr_path = Entry(self, width=20, bd=2, relief="ridge")
        btn_path = Button(
        #self.logic.set_path(x, ntr_path.get())
            self, text='Set path', width=12, command=lambda x='path': self.logic.set_path(x, ntr_path.get()), bd=2, relief="ridge")
        # sli_slider = Scale(self, from_=0, to=100, orient=HORIZONTAL, showvalue=0, highlightthickness=0, 
        #     troughcolor='light gray', cursor='arrow', bg='white', width=24)
        # sli_slider.bind("<ButtonRelease-1>",
        #                 lambda evt: self.logic.sli_change(sli_slider.get()))

        # lbl_slider_val = Label(self, bg='white')

        def cb(value):
            ntr_path.set(value)
            # lbl_slider_val.configure(text=value)

        self.logic.register_sli_cb(cb)
        # grid
        # Label(self, text='Intensity:', bg='White').grid(
            # row=3, column=0, sticky=W+S, padx=16)
        btn_path.grid(row=3, column=0, sticky=W, padx=16)
        ntr_path.grid(row=3, column=1, sticky=W, pady=8)

    def render_kx(self):
        # kx buttons
        ntr_kp = Entry(self, width=20, bd=2, relief="ridge")
        btn_kp = Button(
            self, text='Set kp', width=12, command=lambda x='kp': self.logic.set_kx(x, ntr_kp.get()), bd=2, relief="ridge")

        ntr_ki = Entry(self, width=20, bd=2, relief="ridge")
        btn_ki = Button(
            self, text='Set ki', width=12, command=lambda x='ki': self.logic.set_kx(x, ntr_ki.get()), bd=2, relief="ridge")

        ntr_kd = Entry(self, width=20, bd=2, relief="ridge")
        btn_kd = Button(
            self, text='Set kd', width=12, command=lambda x='kd': self.logic.set_kx(x, ntr_kd.get()), bd=2, relief="ridge")

        def cb(value, type):
            if type == 'kp':
                e = ntr_kp
            elif type == 'ki':
                e = ntr_ki
            elif type == 'kd':
                e = ntr_kd
            else:
                pass
            if e is not None:
                e.delete(0, END)
                e.insert(0, value)

        self.logic.register_kx_cb(cb)

        # grid
        btn_kp.grid(row=5, column=0, sticky=W, padx=16)
        ntr_kp.grid(row=5, column=1, sticky=W, pady=8)
        btn_ki.grid(row=6, column=0, sticky=W, padx=16)
        ntr_ki.grid(row=6, column=1, sticky=W, pady=8)
        btn_kd.grid(row=7, column=0, sticky=W, padx=16)
        ntr_kd.grid(row=7, column=1, sticky=W, pady=8)

    def render_footer(self):
        lbl_footer = Label(
            self, text='Robot teknik er:\n\nawesome...', anchor="w", font=self.helv16, height=1, bd=32, bg="#009688", fg="white")
        # grid
        lbl_footer.grid(row=8, column=0, columnspan=2, sticky=N+S+E+W)

    def render(self):
        self.grid(sticky=N+S+E+W)
        # row and column configure
        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=1)
        for i in range(1, 7):
            self.rowconfigure(i, weight=1)
        # render gui
        self.render_header()
        self.render_onoff()
        self.render_slider()
        self.render_kx()
        self.render_footer()

        def fu():
            self.logic.request_led_state()
            self.logic.request_slider()
            self.logic.request_kx('kp')
            self.logic.request_kx('ki')
            self.logic.request_kx('kd')

        self.after(300, fu)


def main():
    window = Tk()
    window.title('Robot Control')
    window.rowconfigure(0, weight=1)
    window.columnconfigure(0, weight=1)
    window.geometry('560x460')
    ui = UI(window, app_logic.app_logic())
    ui.render()
    window.mainloop()


if __name__ == "__main__":
    main()
