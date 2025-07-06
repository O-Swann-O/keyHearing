from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QVBoxLayout
from PyQt5.QtCore import QTimer, QProcess, QDateTime
import sys

app = QApplication(sys.argv)

window = QWidget()
window.setWindowTitle('keyHearing')
window.resize(220, 140)

label = QLabel()
label.setStyleSheet("color: white; font-size: 20px;")
layout = QVBoxLayout()
layout.addWidget(label)
window.setLayout(layout)
window.setStyleSheet("background-color: black;")
window.show()

process = QProcess()
process.setProcessChannelMode(QProcess.MergedChannels)

last_note_time = QDateTime.currentDateTime()

def read_output():
    global last_note_time

    while process.canReadLine():
        line = process.readLine().data().decode().strip()
        if not line:
            return

        current_time = QDateTime.currentDateTime()

        if line == "NO_SIGNAL":
            if last_note_time.msecsTo(current_time) > 400:
                label.setText("Play something!")
                window.setStyleSheet("background-color: black;")
        else:
            try:
                note, freq, cents = line.split()
                cents = float(cents)
                last_note_time = current_time  

                if abs(cents) < 20:
                    status = "In Tune"
                    color = "green"
                elif cents > 0:
                    status = "Tune Down"
                    color = "red"
                else:
                    status = "Tune Up"
                    color = "red"

                label.setText(f"Note: {note}\nCents off: {cents:.1f}\n{status}")
                window.setStyleSheet(f"background-color: {color};")

            except ValueError:
                label.setText(f"Malformed output:\n{line}")
                window.setStyleSheet("background-color: orange;")

process.readyReadStandardOutput.connect(read_output)
process.start("./build/Debug/keyHearing.exe")

sys.exit(app.exec_())
