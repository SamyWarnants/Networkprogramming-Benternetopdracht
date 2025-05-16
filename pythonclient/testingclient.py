import sys
import zmq
import threading
import time
from datetime import datetime
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QLineEdit, QVBoxLayout, QWidget
)
from PyQt6.QtCore import Qt, pyqtSignal, QObject


POLL_INTERVAL_MS = 100


class SignalHandler(QObject):
    log_signal = pyqtSignal(str)


class BenternetClient(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Benternet GUI Client")
        self.resize(600, 400)

        # ZMQ setup
        self.context = zmq.Context()
        self.pusher = self.context.socket(zmq.PUSH)
        self.pusher.connect("tcp://benternet.pxl-ea-ict.be:24041")

        self.subscriber = self.context.socket(zmq.SUB)
        self.subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042")
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, "")

        self.message_history = []
        self.history_index = -1

        # GUI setup
        self.output = QTextEdit()
        self.output.setReadOnly(True)

        self.input = QLineEdit()
        self.input.returnPressed.connect(self.send_message)
        self.input.installEventFilter(self)

        layout = QVBoxLayout()
        layout.addWidget(self.output)
        layout.addWidget(self.input)

        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)

        # Signal handler (thread-safe logger)
        self.signals = SignalHandler()
        self.signals.log_signal.connect(self.append_log)

        # Threading
        self.running = True
        self.receiver_thread = threading.Thread(target=self.receive_messages, daemon=True)
        self.receiver_thread.start()

    def append_log(self, msg):
        self.output.append(msg)

    def log(self, label, msg):
        timestamp = datetime.now().strftime("[%H:%M:%S]")
        full_msg = f"{timestamp} [{label}] {msg}"
        self.signals.log_signal.emit(full_msg)

    def send_message(self):
        text = self.input.text().strip()
        if not text:
            return

        try:
            self.pusher.send_string(text)
            self.log("SENT", text)
            self.message_history.append(text)
            self.history_index = len(self.message_history)
        except zmq.ZMQError as e:
            self.log("ERROR", f"Failed to send: {e}")
        finally:
            self.input.clear()

    def receive_messages(self):
        poller = zmq.Poller()
        poller.register(self.subscriber, zmq.POLLIN)

        while self.running:
            try:
                socks = dict(poller.poll(POLL_INTERVAL_MS))
                if self.subscriber in socks:
                    message = self.subscriber.recv_string()
                    self.log("RECEIVED", message)
            except zmq.ZMQError:
                break

    def closeEvent(self, event):
        self.running = False
        self.receiver_thread.join(timeout=1)
        try:
            self.subscriber.close()
            self.pusher.close()
            self.context.term()
        except zmq.ZMQError:
            pass
        event.accept()

    def eventFilter(self, source, event):
        if source == self.input and event.type() == event.Type.KeyPress:
            if event.key() == Qt.Key.Key_Up:
                if self.message_history and self.history_index > 0:
                    self.history_index -= 1
                    self.input.setText(self.message_history[self.history_index])
                return True
            elif event.key() == Qt.Key.Key_Down:
                if self.message_history and self.history_index < len(self.message_history) - 1:
                    self.history_index += 1
                    self.input.setText(self.message_history[self.history_index])
                else:
                    self.history_index = len(self.message_history)
                    self.input.clear()
                return True
        return super().eventFilter(source, event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = BenternetClient()
    window.show()
    sys.exit(app.exec())
