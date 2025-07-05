from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
from PyQt5.QtCore import QTimer
import subprocess
import sys

app = QApplication(sys.argv)

window = QWidget()
window.setWindowTitle('keyHearing')
window.resize(100, 100)

label = QLabel()
label.setStyleSheet("color: white; font-size: 24px;")  
layout = QVBoxLayout()
layout.addWidget(label)
window.setLayout(layout)
window.setStyleSheet("background-color: black;")  
window.show()

process = subprocess.Popen(
    ["./build/Debug/keyHearing.exe"],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True
)

def read_output():
    line = process.stdout.readline()
    if not line:
        return

    line = line.strip()
    if line == "NO_SIGNAL":
        label.setText("Play something!")
        window.setStyleSheet("background-color: black;")
    else:
        try:
            note, freq, cents = line.split()
            label.setText(f"Note: {note}\nFrequency: {freq} Hz\nCents off: {cents}")
            if abs(float(cents)) < 30:
                window.setStyleSheet("background-color: green;")
            else:
                window.setStyleSheet("background-color: red;")
        except ValueError:
            label.setText(f"Malformed output:\n{line}")
            window.setStyleSheet("background-color: orange;")

timer = QTimer()
timer.timeout.connect(read_output)
timer.start(50)

sys.exit(app.exec_())
