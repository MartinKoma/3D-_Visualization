import sys
from configparser import ConfigParser
import database
import subprocess
import diagrams
from PyQt5.QtWidgets import QWidget, QApplication, QLineEdit, QLabel, QFormLayout, QComboBox, QPushButton, \
    QGridLayout, QMessageBox
from PyQt5 import QtGui, QtCore, QtSvg
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from visualization import main


class App(QWidget):
    """
    This Window serves as a game menu, from which the
    game itself can be started, as well as the
    visualization of the sensor.
    :input:
        Username: The name which gets written
        to the database along with the achieved score.
        Difficulty: The user can choose up to four
        different difficulties for the game.
    :buttons:
        Options: New usernames can be entered, and the
        comport is set here, for more information, see
        docstring in Options class.
        Show Highscore: The highscore of the username
        chosen in the combobox above will be displayed
        in the button, when clicked.
        Show visu: the visualization of the sensor
        opens up in the browser.
        Save config: values from the inputs (including options)
        will be saved into config.ini
        Start Game: The game will start
    """

    def __init__(self):
        super().__init__()
        # General layout
        self.file = 'config.ini'
        self.config = ConfigParser()
        self.config.read(self.file)
        self.setWindowTitle('Game Menu')
        self.setWindowIcon(QIcon("icons\\spaceship.png"))
        self.setGeometry(450, 150, 300, 250)
        self.layout = QFormLayout(self)

        # Initialization of buttons and comboboxes
        self.combousername = QComboBox()
        self.combousername.addItems([name[0] for name in database.get_usernames()])
        self.combousername.setCurrentText(self.config["user"]["name"])
        self.combodifficulty = QComboBox()
        self.combodifficulty.addItems(["easy", "medium", "hard", "very_hard"])
        self.combodifficulty.setCurrentText(self.config["user"]["difficulty"])
        self.start_button = QPushButton("Start Game")
        self.start_button.clicked.connect(self.start_game)
        self.start_button.resize(20, 40)
        self.save_button = QPushButton("Save Config")
        self.save_button.clicked.connect(self.save_config)
        self.score_button = QPushButton("Show Highscore")
        self.score_button.clicked.connect(self.show_score)
        self.visu_button = QPushButton("Show visu")
        self.visu_button.pressed.connect(self.show_visu)
        self.option_button = QPushButton()
        self.option_button.pressed.connect(self.show_options)
        self.option_button.setIcon(QIcon("icons\\options.png"))
        self.dia_button = QPushButton("Show Diagram")
        self.dia_button.pressed.connect(self.show_diagram)

        # Initialization and design of labels
        myFont = QtGui.QFont()
        myFont.setBold(True)
        self.label_options = QLabel("Options")
        self.label_options.setFont(myFont)
        self.label_options.setFixedWidth(70)
        self.label_difficulty = QLabel("Difficulty")
        self.label_difficulty.setFont(myFont)
        self.label_difficulty.setFixedWidth(70)
        self.label_options.setFont(myFont)
        self.label_username = QLabel("Username")
        self.label_username.setFont(myFont)
        self.label_username.setFixedWidth(70)
        self.label_highscore = QLabel("Highscore")
        self.label_highscore.setFont(myFont)
        self.label_highscore.setFixedWidth(70)
        self.label_visu = QLabel("Visualization")
        self.label_visu.setFont(myFont)
        self.label_visu.setFixedWidth(70)
        self.label_diagram = QLabel("Diagram")
        self.label_diagram.setFont(myFont)
        self.label_diagram.setFixedWidth(70)
        # Adding above initialized Widgets (labels, buttons, comboboxes..) to layout
        self.layout.addRow(self.label_options, self.option_button)
        self.layout.addRow(self.label_username, self.combousername)
        self.layout.addRow(self.label_difficulty, self.combodifficulty)
        self.layout.addRow(self.label_highscore, self.score_button)
        self.layout.addRow(self.label_visu, self.visu_button)
        self.layout.addRow(self.label_diagram, self.dia_button)
        self.layout.addRow(self.save_button)
        self.layout.addRow(self.start_button)
        self.show()

    def show_score(self):
        # Grabs input from the combobox, calls method from database, which returns the highscore
        # Parses it into a string and sets it as text for the button
        self.score_button.setText(str(database.get_highscore(self.combousername.currentText())))

    def start_game(self):
        try:
            #self.close()
            subprocess.call("python main.py", shell=True)
        except:
            pass

    def show_visu(self):
        try:
            self.close()
            main()
        except:
            pass

    def show_diagram(self):
        try:
            subprocess.call("python diagrams.py", shell=True)
        except:
            pass

    def save_config(self):
        # Grabs input from comboboxes and saves it into config.ini to be later used by the game
        self.config.set("user", "name", f"{self.combousername.currentText()}")
        self.config.set("user", "highscore", f"{database.get_highscore(self.combousername.currentText())}")
        self.config.set("user", "difficulty", f"{self.combodifficulty.currentText()}")
        with open(self.file, "w") as configfile:
            self.config.write(configfile)

    def show_options(self):
        # Opens Config window, data is transmitted via a pyQtSignal
        self.window = Options()
        self.window.submitted.connect(self.transmit)
        self.window.show()

    def transmit(self, names):
        # Puts transmitted names from Option window into the combobox
        if names != "":
            self.combousername.addItems(name for name in names.split(","))


class Options(QWidget):
    """
    Options provides the additional window which gets
    opened, when the user wants to configure the game.
    When the Window opens, the User can pass in the
    parameters, which will be sent to the Main window
    with a pyqtSignal.
    :input:
    name: user can set name
    Comport: user can set comport
    :buttons:
    Save name:
    text from input field name above will be saved into the database
    Save comport and close:
    text from input field comport will be saved into config.ini
    """
    submitted = QtCore.pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.new_names = []
        self.file = 'config.ini'
        self.config = ConfigParser()
        self.config.read(self.file)
        self.setWindowTitle('Config')
        self.setWindowIcon(QIcon("icons\\options.png"))
        self.setGeometry(500, 200, 400, 200)
        self.layout = QGridLayout(self)
        self.setLayout(self.layout)

        self.button_name = QPushButton("Save name")
        self.button_save = QPushButton("Save Comport and close")
        self.edit_comport = QLineEdit()
        self.edit_comport.setText(self.config["comport"]["port"])
        self.edit_username = QLineEdit()
        self.label_comport = QLabel("Comport")
        self.label_username = QLabel("Enter new name")
        self.button_name.pressed.connect(self.save_name)
        self.button_save.pressed.connect(self.save_comport)

        self.layout.addWidget(self.label_comport, self.layout.rowCount() - 1, self.layout.columnCount())
        self.layout.addWidget(self.edit_comport, self.layout.rowCount() - 1, self.layout.columnCount())
        self.layout.addWidget(self.label_username, self.layout.rowCount(), self.layout.columnCount() - 2)
        self.layout.addWidget(self.edit_username, self.layout.rowCount() - 1, self.layout.columnCount() - 1)
        self.layout.addWidget(self.button_save, self.layout.rowCount() + 2, self.layout.columnCount() - 2, 2, 2)
        self.layout.addWidget(self.button_name, self.layout.rowCount() - 3, self.layout.columnCount() - 2, 2, 2)

    def save_name(self):
        """
        Saves name to the database
        :var name
        Grabs the input from the username textfield, it serves no purpose other than storing that value
        temporarily, so it doesn't have the be Grabbed three times from the QLineEdit
        The user is not allowed to enter an empty string as a name, if he does it anyways,
        a Messagebox will show up, to remind the user of our name conventions
        """
        name = str(self.edit_username.text())
        if name != "":
            database.insert_user(name)
            self.new_names.append(name)
            msg = QMessageBox()
            msg.setText(f"Username {name} is part of the database")
            msg.setWindowTitle("Name saved")
            #msg.setIcon(QMessageBox.QIcon("icons\\options.png"))
            msg.exec()
        else:
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText(f"Enter a Username, at least one character is required")
            msg.setWindowTitle("Name could not be saved")
            msg.exec()

    def save_comport(self):
        # Grabs comport from input field and saves it into config.ini
        # all names that have been entered during the time the window
        # was open, will be transmitted back to the main window
        # lists can not be sent via the signal, at least I didn't succeed
        # so the names will be transmitted as a string
        self.config.set("comport", "port", f"{self.edit_comport.text()}")
        with open(self.file, "w") as configfile:
            self.config.write(configfile)
        self.submitted.emit(",".join(self.new_names))
        self.close()

   #def closeEvent(self, event):
   #    # In case the user doesn't use the designated button to close the form
   #    self.submitted.emit(",".join(self.new_names))
   #    self.close()


if __name__ == '__main__':
    # I am by no means a graphic designer, neither am I
    # experienced in beautifying pyqt applications,
    # but I tried my best to make the form differentiate
    # from others and to expand my knowledge about designing
    qp = QPalette()
    qp.setColor(QPalette.Window, Qt.black)
    app = QApplication(sys.argv)
    app.setStyleSheet("QLineEdit { background-color: yellow }")
    app.setStyleSheet("QLabel{font-size: 14pt;}")
    app.setStyleSheet("QLabel{background-color: red;}")
    myFont = QtGui.QFont()
    myFont.setBold(True)
    app.setFont(myFont)
    app.setPalette(qp)
    ex = App()
    sys.exit(app.exec_())