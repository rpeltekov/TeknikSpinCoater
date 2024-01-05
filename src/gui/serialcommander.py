import platform
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLineEdit, QTextEdit, QLabel
import serial

class EmittingStream(object):
    def __init__(self, text_widget):
        self.text_widget = text_widget

    def write(self, text):
        self.text_widget.append(text)


class SerialCommander(QWidget):
    def __init__(self):
        super().__init__()

        self.initUI()
        self.initSerial()

    def initUI(self):
        # Create buttons
        self.button1 = QPushButton('Send "E0"', self)
        self.button1.clicked.connect(self.sendCommandEM0)

        self.button2 = QPushButton('Send "V0 2000"', self)
        self.button2.clicked.connect(self.sendCommandMV100)
        
        # Create a text field for command input
        self.commandInput = QLineEdit(self)

        # Create a button to send command
        self.sendButton = QPushButton('Send Command', self)
        self.sendButton.clicked.connect(self.sendCommand)

        # Log Console
        self.logLabel = QLabel('Log:', self)
        self.logConsole = QTextEdit(self)
        self.logConsole.setReadOnly(True)

        # Redirect stdout and stderr
        sys.stdout = EmittingStream(self.logConsole)
        sys.stderr = EmittingStream(self.logConsole)

        # Layout
        layout = QVBoxLayout()
        layout.addWidget(self.button1)
        layout.addWidget(self.button2)
        layout.addWidget(self.commandInput)
        layout.addWidget(self.sendButton)
        layout.addWidget(self.logLabel)
        layout.addWidget(self.logConsole)
        self.setLayout(layout)

        # Window settings
        self.setWindowTitle('Serial Commander')
        self.show()

    def initSerial(self):
        # Initialize serial connection
        self.ser = serial.Serial()
        self.ser.baudrate = 9600
        
        os_name = platform.system()
        if os_name == 'Linux':
            self.ser.port = '/dev/ttyACM0'  # Change this to your Linux serial port
        elif os_name == 'Windows':
            ports = serial.tools.list_ports.comports()
            for port, desc, hwid in sorted(ports):
                # Assuming the first available port is the one we want to use
                self.ser.port = port
                break
            else:
                print("No active COM ports found on Windows.")
                return
        else:
            print(f"Unsupported OS: {os_name}")
            return

        try:
            self.ser.open()
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")

    def sendCommandEM0(self):
        # Send command "E M0" via serial
        try:
            self.ser.write(b'E M0\n')
            print("Command 'E M0' sent!")
        except serial.SerialException as e:
            print(f"Error sending command 'E M0': {e}")

    def sendCommandMV100(self):
        # Send command "mv 100" via serial
        try:
            self.ser.write(b'v0 2000\n')
            print("Command 'v0 2000' sent!")
        except serial.SerialException as e:
            print(f"Error sending command 'v0 2000': {e}")

    def sendCommand(self):
        # Read command from text field
        command = self.commandInput.text()
        if command:
            # Send command via serial
            try:
                self.ser.write(command.encode() + b'\n')
                print(f"Command '{command}' sent!")
            except serial.SerialException as e:
                print(f"Error sending command '{command}': {e}")


def main():
    app = QApplication(sys.argv)
    ex = SerialCommander()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()

