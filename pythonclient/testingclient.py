import sys
import zmq
import threading
import time
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QTextEdit, QLineEdit, QVBoxLayout, QWidget
)
from PyQt6.QtCore import Qt


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
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, "")  # Subscribe to all topics

        self.message_history = []
        self.history_index = -1

        # GUI components
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

        # Threading
        self.running = True
        self.receiver_thread = threading.Thread(target=self.receive_messages, daemon=True)
        self.receiver_thread.start()

    def send_message(self):
        text = self.input.text().strip()
        if not text:
            return

        try:
            self.pusher.send_string(text)
            self.output.append(f"[SENT] {text}")
            self.message_history.append(text)
            self.history_index = len(self.message_history)
        except zmq.ZMQError as e:
            self.output.append(f"[ERROR] Failed to send: {e}")
        finally:
            self.input.clear()

    def receive_messages(self):
        while self.running:
            try:
                if self.subscriber.poll(timeout=100):  # safer polling
                    message = self.subscriber.recv_string()
                    self.output.append(f"[RECEIVED] {message}")
            except zmq.ZMQError:
                break  # Likely caused by shutdown

    def closeEvent(self, event):
        # Proper shutdown
        self.running = False
        self.receiver_thread.join(timeout=1)
        try:
            self.subscriber.close(0)
            self.pusher.close(0)
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
