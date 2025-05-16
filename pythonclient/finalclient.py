import sys
import zmq
import threading
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QPushButton,
    QLabel, QLineEdit, QTextEdit, QStackedWidget
)
from PyQt6.QtGui import QShortcut, QKeySequence
from PyQt6.QtCore import QTimer


class ZMQHandler:
    def __init__(self, message_callback):
        self.context = zmq.Context()
        self.pusher = self.context.socket(zmq.PUSH)
        self.pusher.connect("tcp://benternet.pxl-ea-ict.be:24041")

        self.subscriber = self.context.socket(zmq.SUB)
        self.subscriber.connect("tcp://benternet.pxl-ea-ict.be:24042")
        self.subscriber.setsockopt_string(zmq.SUBSCRIBE, "")

        self.message_callback = message_callback
        self.running = True
        self.thread = threading.Thread(target=self.receive_messages, daemon=True)
        self.thread.start()

    def send(self, message):
        self.pusher.send_string(message)

    def receive_messages(self):
        while self.running:
            if self.subscriber.poll(timeout=100):
                message = self.subscriber.recv_string()
                self.message_callback(message)

    def close(self):
        self.running = False
        self.thread.join(timeout=1)
        self.subscriber.close(0)
        self.pusher.close(0)
        self.context.term()


class TamagotchiApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Tamagotchiland Chatbot Client")
        self.resize(600, 500)

        self.pet_name = ""
        self.zmq = ZMQHandler(self.handle_message)

        self.stack = QStackedWidget()
        self.setCentralWidget(self.stack)

        self.message_log = QTextEdit()
        self.message_log.setReadOnly(True)
        self.message_log.setFixedHeight(120)

        self.login_screen()
        self.menu_screen()
        self.petpark_screen()
        self.petcare_screen()
        self.logs_screen()
        self.debug_screen()

        self.stack.setCurrentWidget(self.login_widget)

    def add_to_stack(self, widget):
        layout = QVBoxLayout()
        layout.addWidget(widget)
        layout.addWidget(QLabel("Service Messages:"))
        layout.addWidget(self.message_log)
        container = QWidget()
        container.setLayout(layout)
        self.stack.addWidget(container)
        return container

    def login_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        self.login_label = QLabel("Enter your pet name:")
        self.login_input = QLineEdit()
        self.login_input.setPlaceholderText("e.g., Larry")
        self.login_error = QLabel("")
        self.login_error.setStyleSheet("color: red")
        login_button = QPushButton("Login")
        login_button.clicked.connect(self.try_login)
        self.login_input.returnPressed.connect(self.try_login)
        layout.addWidget(self.login_label)
        layout.addWidget(self.login_input)
        layout.addWidget(login_button)
        layout.addWidget(self.login_error)
        widget.setLayout(layout)
        self.login_widget = self.add_to_stack(widget)

    def try_login(self):
        name = self.login_input.text().strip()
        if name:
            self.pet_name = name
            self.zmq.send(f"Tamagotchiland>CreatePet!>Login>{name}")
        else:
            self.login_error.setText("Please enter a valid pet name.")

    def menu_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        layout.addWidget(QLabel("Main Menu"))

        self.menu_buttons = []
        for text, handler in [
            ("PetPark", lambda: self.stack.setCurrentWidget(self.petpark_widget)),
            ("Petcare", lambda: self.stack.setCurrentWidget(self.petcare_widget)),
            ("Logs", lambda: self.stack.setCurrentWidget(self.logs_widget)),
            ("Debug", lambda: self.stack.setCurrentWidget(self.debug_widget)),
        ]:
            btn = QPushButton(text)
            btn.clicked.connect(handler)
            layout.addWidget(btn)
            self.menu_buttons.append(btn)

        self.current_menu_index = 0
        self.menu_buttons[self.current_menu_index].setFocus()

        QShortcut(QKeySequence("Up"), widget).activated.connect(self.menu_up)
        QShortcut(QKeySequence("Down"), widget).activated.connect(self.menu_down)
        QShortcut(QKeySequence("Return"), widget).activated.connect(self.menu_select)

        widget.setLayout(layout)
        self.menu_widget = self.add_to_stack(widget)

    def menu_up(self):
        self.current_menu_index = (self.current_menu_index - 1) % len(self.menu_buttons)
        self.menu_buttons[self.current_menu_index].setFocus()

    def menu_down(self):
        self.current_menu_index = (self.current_menu_index + 1) % len(self.menu_buttons)
        self.menu_buttons[self.current_menu_index].setFocus()

    def menu_select(self):
        self.menu_buttons[self.current_menu_index].click()

    def petpark_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        layout.addWidget(QLabel("Make a guess between 1 and 10:"))
        self.guess_input = QLineEdit()
        self.guess_input.setPlaceholderText("Enter a number...")
        self.guess_result = QLabel()
        guess_button = QPushButton("Submit Guess")
        guess_button.clicked.connect(self.send_guess)
        self.guess_input.returnPressed.connect(self.send_guess)
        back_button = QPushButton("Back")
        back_button.clicked.connect(lambda: self.stack.setCurrentWidget(self.menu_widget))
        layout.addWidget(self.guess_input)
        layout.addWidget(guess_button)
        layout.addWidget(self.guess_result)
        layout.addWidget(back_button)
        widget.setLayout(layout)
        self.petpark_widget = self.add_to_stack(widget)

    def petcare_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        self.feed_label = QLabel("Feeding: ?")
        self.clean_label = QLabel("Cleaning: ?")
        feed_btn = QPushButton("Feed")
        feed_btn.clicked.connect(lambda: self.zmq.send(f"Tamagotchiland>PetPark!>{self.pet_name}>Petcare>Feeding"))
        clean_btn = QPushButton("Clean")
        clean_btn.clicked.connect(lambda: self.zmq.send(f"Tamagotchiland>PetPark!>{self.pet_name}>Petcare>Cleaning"))
        stats_btn = QPushButton("Refresh Stats")
        stats_btn.clicked.connect(lambda: self.zmq.send(f"Tamagotchiland>PetPark!>{self.pet_name}>Stats"))
        back_btn = QPushButton("Back")
        back_btn.clicked.connect(lambda: self.stack.setCurrentWidget(self.menu_widget))
        layout.addWidget(self.feed_label)
        layout.addWidget(feed_btn)
        layout.addWidget(self.clean_label)
        layout.addWidget(clean_btn)
        layout.addWidget(stats_btn)
        layout.addWidget(back_btn)
        widget.setLayout(layout)
        self.petcare_widget = self.add_to_stack(widget)

    def logs_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        self.logs_output = QTextEdit()
        self.logs_output.setReadOnly(True)
        refresh_btn = QPushButton("Refresh Logs")
        refresh_btn.clicked.connect(lambda: self.zmq.send(f"Tamagotchiland>PetPark!>{self.pet_name}>Logs"))
        back_btn = QPushButton("Back")
        back_btn.clicked.connect(lambda: self.stack.setCurrentWidget(self.menu_widget))
        layout.addWidget(self.logs_output)
        layout.addWidget(refresh_btn)
        layout.addWidget(back_btn)
        widget.setLayout(layout)
        self.logs_widget = self.add_to_stack(widget)

    def debug_screen(self):
        widget = QWidget()
        layout = QVBoxLayout()
        self.debug_output = QTextEdit()
        self.debug_output.setReadOnly(True)
        back_btn = QPushButton("Back")
        back_btn.clicked.connect(lambda: self.stack.setCurrentWidget(self.menu_widget))
        layout.addWidget(self.debug_output)
        layout.addWidget(back_btn)
        widget.setLayout(layout)
        self.debug_widget = self.add_to_stack(widget)

    def send_guess(self):
        guess = self.guess_input.text().strip()
        if guess.isdigit():
            self.zmq.send(f"Tamagotchiland>PetPark!>{self.pet_name}>Play>{guess}")
            self.guess_result.setText("Waiting for response...")

    def handle_message(self, msg):
        self.debug_output.append(msg)

        # Server-style filtering
        if msg.startswith("Tamagotchiland>"):
            if f">{self.pet_name}>" in msg:
                readable = msg.split(f">{self.pet_name}>")[-1].strip()
                self.message_log.append(f"Server: {readable}")
            elif msg.startswith("Tamagotchiland>CreatePet?>"):
                readable = msg.split("?>")[-1].strip()
                self.message_log.append(f"Server: {readable}")
            else:
                self.message_log.append(f"Server: {msg}")

        # Login accepted or existing pet
        if msg.startswith(f"Tamagotchiland>CreatePet?>{self.pet_name}"):
            self.stack.setCurrentWidget(self.menu_widget)
        if msg.endswith("try again") or "invalid" in msg.lower():
            self.login_error.setText(msg.split(">")[-1])

        # Stats update
        if f">Stats>Hunger>" in msg:
            self.feed_label.setText(f"Feeding: {msg.split('>')[-1]}")
        elif f">Stats>Hygiene>" in msg:
            self.clean_label.setText(f"Cleaning: {msg.split('>')[-1]}")

        # Game result
        if f">Play>" in msg:
            content = msg.split(">")[-1].strip().lower()
            if "yay" in content or "guessed" in content:
                self.guess_result.setText("Congrats, you guessed it!")
                QTimer.singleShot(2000, lambda: self.stack.setCurrentWidget(self.menu_widget))
            elif content == "n":
                self.guess_result.setText("No silly, try again!")

        # Logs
        if f">Logs>" in msg:
            self.logs_output.append(msg.split(">")[-1])

    def closeEvent(self, event):
        self.zmq.close()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = TamagotchiApp()
    window.show()
    sys.exit(app.exec())
