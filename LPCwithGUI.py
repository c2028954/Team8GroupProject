import serial
import time
import threading
import queue
import tkinter as tk
from tkinter import messagebox
from collections import deque
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.animation as animation

# === CONFIG ===
SERIAL_PORT        = 'COM11'
BAUD_RATE          = 9600
BUFFER_LEN         = 100
UPDATE_INTERVAL_MS = 500

# === THREAD-SAFE QUEUE FOR PARSED DATA ===
data_q = queue.Queue()

# === DEQUES FOR PLOTTING ===
temp_data    = deque(maxlen=BUFFER_LEN)
rssi_data    = deque(maxlen=BUFFER_LEN)
sample_index = deque(maxlen=BUFFER_LEN)
counter      = 0

# === TOOLTIP HELPER ===
class ToolTip:
    def __init__(self, widget, text):
        self.widget = widget
        self.text   = text
        self.tipwin = None
        widget.bind("<Enter>", self.show)
        widget.bind("<Leave>", self.hide)
    def show(self, _):
        if self.tipwin or not self.text: return
        x, y, _, cy = self.widget.bbox("insert")
        x += self.widget.winfo_rootx() + 25
        y += self.widget.winfo_rooty() + cy + 25
        tw = tk.Toplevel(self.widget)
        tw.wm_overrideredirect(True)
        tw.wm_geometry(f"+{x}+{y}")
        lbl = tk.Label(tw, text=self.text, justify='left',
                       background="#ffffe0", relief='solid', borderwidth=1,
                       font=("tahoma", "8", "normal"))
        lbl.pack(ipadx=1)
        self.tipwin = tw
    def hide(self, _):
        if self.tipwin:
            self.tipwin.destroy()
            self.tipwin = None

# === TK INTERFACE SETUP ===
root = tk.Tk()
root.title("Formula Student Telemetry")
root.geometry("1000x800")

# — Control bar —
ctrl = tk.Frame(root)
ctrl.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)

# Connection Status
status_label = tk.Label(ctrl, text="Disconnected", width=12, bg="red", fg="white")
status_label.pack(side=tk.LEFT, padx=5)
ToolTip(status_label, "Shows serial connection status")

# Numeric Readouts
temp_label = tk.Label(ctrl, text="Temp: --.- °C", width=16)
temp_label.pack(side=tk.LEFT, padx=5)
ToolTip(temp_label, "Latest temperature reading")

rssi_label = tk.Label(ctrl, text="RSSI: ---- dBm", width=16)
rssi_label.pack(side=tk.LEFT, padx=5)
ToolTip(rssi_label, "Latest signal strength reading")

# Pause / Resume / Reset buttons
def on_pause():
    anim.event_source.stop()
    pause_btn.config(state=tk.DISABLED)
    resume_btn.config(state=tk.NORMAL)
pause_btn = tk.Button(ctrl, text="⏸ Pause", command=on_pause)
pause_btn.pack(side=tk.LEFT, padx=5)
ToolTip(pause_btn, "Pause live plotting")

def on_resume():
    anim.event_source.start()
    resume_btn.config(state=tk.DISABLED)
    pause_btn.config(state=tk.NORMAL)
resume_btn = tk.Button(ctrl, text="▶ Resume", command=on_resume, state=tk.DISABLED)
resume_btn.pack(side=tk.LEFT, padx=5)
ToolTip(resume_btn, "Resume live plotting")

def on_reset():
    global counter
    temp_data.clear(); rssi_data.clear(); sample_index.clear()
    counter = 0
    _setup_axes()
    temp_line.set_data([], [])
    rssi_line.set_data([], [])
    canvas.draw()
reset_btn = tk.Button(ctrl, text="🔄 Reset", command=on_reset)
reset_btn.pack(side=tk.LEFT, padx=5)
ToolTip(reset_btn, "Clear data and reset sample count")

# Dark / Light theme toggle
dark_mode = False
def toggle_theme():
    global dark_mode
    dark_mode = not dark_mode
    _apply_theme()
theme_btn = tk.Button(ctrl, text="🌙 Dark Mode", command=toggle_theme)
theme_btn.pack(side=tk.LEFT, padx=5)
ToolTip(theme_btn, "Switch between Dark and Light mode")

# Help dialog
def show_help():
    messagebox.showinfo(
        "Help",
        "• Connects to Arduino over serial (COM11 @ 9600)\n"
        "• Plots Temperature & RSSI vs. sample number\n"
        "• Red flash square on UVD event\n"
        "• Pause / Resume / Reset data\n"
        "• Toggle Dark / Light theme\n"
        "• Hover over controls for tips"
    )
help_btn = tk.Button(ctrl, text="❔ Help", command=show_help)
help_btn.pack(side=tk.RIGHT, padx=5)
ToolTip(help_btn, "Show this help dialog")

# — UVD Indicator Square (upper-right) —
indicator_frame = tk.Frame(root, bg='lightgrey', width=200, height=100, bd=2, relief='ridge')
indicator_frame.place(relx=0.75, rely=0.05)
indicator_label = tk.Label(indicator_frame, text="", font=("Helvetica", 12, "bold"))
indicator_label.place(relx=0.5, rely=0.5, anchor='center')
indicator_frame.lower()  # hide beneath everything

# === MATPLOTLIB CANVAS ===
fig, (ax1, ax2) = plt.subplots(2,1, figsize=(8,6))
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

def _setup_axes():
    ax1.clear(); ax2.clear()
    ax1.set_title("Temperature");  ax1.set_ylabel("°C");  ax1.set_xlabel("Sample #")
    ax2.set_title("RSSI");         ax2.set_ylabel("dBm"); ax2.set_xlabel("Sample #")
    ax1.set_xlim(0, BUFFER_LEN);   ax2.set_xlim(0, BUFFER_LEN)
    ax1.set_ylim(0, 100);          ax2.set_ylim(-120, 0)
_setup_axes()

# create line objects
temp_line, = ax1.plot([], [], color='blue')
rssi_line, = ax2.plot([], [], color='orange')

# theme styling
def _apply_theme():
    bg = '#333333' if dark_mode else 'white'
    fg = 'white'       if dark_mode else 'black'
    root.configure(bg=bg); ctrl.configure(bg=bg)
    for w in ctrl.winfo_children():
        if isinstance(w, (tk.Label, tk.Button)):
            w.configure(bg=bg, fg=fg, activebackground=fg, activeforeground=bg)
    fig.patch.set_facecolor(bg)
    for ax in (ax1, ax2):
        ax.set_facecolor(bg)
        for spine in ax.spines.values(): spine.set_color(fg)
        ax.title.set_color(fg)
        ax.xaxis.label.set_color(fg); ax.yaxis.label.set_color(fg)
        ax.tick_params(colors=fg)
    canvas.draw()
    theme_btn.config(text="☀️ Light Mode" if dark_mode else "🌙 Dark Mode")
_apply_theme()

# === UVD FLASH HANDLER ===
def flash_indicator():
    indicator_frame.lift()
    indicator_frame.configure(bg='red')
    indicator_label.config(text="Unusual\nVibrations\nDetected")
    root.after(500, hide_indicator)  # show for 0.5 s

def hide_indicator():
    indicator_frame.lower()
    indicator_frame.configure(bg='lightgrey')
    indicator_label.config(text="")

# === SERIAL THREAD ===
def set_status(txt, col):
    root.after(0, lambda: status_label.config(text=txt, bg=col))

def read_serial():
    global counter
    set_status("Connecting", "orange")
    # try to open port
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            set_status("Connected", "green")
            break
        except Exception:
            set_status("Disconnected", "red")
            time.sleep(2)
    # read loop
    while True:
        try:
            raw = ser.readline().decode(errors='ignore').strip()
            if raw.startswith("Received '") and "with RSSI" in raw:
                start = raw.find("'")+1
                end   = raw.find("'", start)
                data  = raw[start:end]
                rssi  = int(raw.split("with RSSI")[-1])
                if "UVD" in data:
                    root.after(0, flash_indicator)
                tmp = data.split(',')[0]
                try:
                    t = float(tmp)
                    data_q.put((t, rssi))
                except ValueError:
                    pass
        except serial.SerialException:
            set_status("Disconnected", "red")
            break
        except Exception:
            break
    ser.close()
    set_status("Disconnected", "red")

threading.Thread(target=read_serial, daemon=True).start()

# === PLOTTING UPDATE ===
def update(frame):
    global counter
    while not data_q.empty():
        t, r = data_q.get_nowait()
        temp_data.append(t)
        rssi_data.append(r)
        sample_index.append(counter)
        counter += 1

    if sample_index:
        xs  = list(sample_index)
        ys1 = list(temp_data)
        ys2 = list(rssi_data)
        temp_line.set_data(xs, ys1)
        ax1.set_xlim(0, max(BUFFER_LEN, xs[-1]+1))
        ax1.set_ylim(min(ys1)-2, max(ys1)+2)
        rssi_line.set_data(xs, ys2)
        ax2.set_xlim(0, max(BUFFER_LEN, xs[-1]+1))
        ax2.set_ylim(min(ys2)-5, max(ys2)+5)
        temp_label.config(text=f"Temp: {ys1[-1]:.2f} °C")
        rssi_label.config(text=f"RSSI: {ys2[-1]} dBm")

    canvas.draw()

anim = animation.FuncAnimation(
    fig, update,
    interval=UPDATE_INTERVAL_MS,
    cache_frame_data=False
)

root.protocol("WM_DELETE_WINDOW", root.quit)
root.mainloop()
