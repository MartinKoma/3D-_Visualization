import sys
import time
from configparser import ConfigParser

from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QWidget, QApplication, QLineEdit, QLabel, QFormLayout, QComboBox, QPushButton, \
    QGridLayout, QMessageBox, QMainWindow, QVBoxLayout, QDesktopWidget
import numpy as np
from PyQt5 import QtGui, QtCore
import pyqtgraph as pg
# import pyqtgraph.exporters
import serial
from computation import *


class AngelsPlot(QMainWindow):
    def __init__(self, parent=None):
        super(AngelsPlot, self).__init__(parent)

        # Set up GUI configuration
        self.setGeometry(0, 0, 1200, 500)
        self.centerOnScreen()
        self.setWindowTitle('Diagrams')
        self.setWindowIcon(QIcon("icons\\spaceship.png"))
        self.mainbox = QWidget()
        self.setCentralWidget(self.mainbox)
        self.mainbox.setLayout(QVBoxLayout())

        self.canvas = pg.GraphicsLayoutWidget()  # create GrpahicsLayoutWidget obejct
        self.mainbox.layout().addWidget(self.canvas)

        # Set up plot
        self.eulerPlot = self.canvas.addPlot(title="Euler Angles")
        self.eulerPlot.setYRange(-180, 180)
        # self.eulerPlot.setXRange()
        self.eulerPlot.showGrid(x=True, y=True, alpha=0.5)
        x_axis = self.eulerPlot.getAxis('bottom')
        y_axis = self.eulerPlot.getAxis('left')

        x_axis.setLabel(text='Time in s')  # set axis labels
        y_axis.setLabel(text='Angels in Degree')

        self.quatPlot = self.canvas.addPlot(title="Quaternions")
        self.quatPlot.setYRange(-180, 180)
        # self.eulerPlot.setXRange()
        self.quatPlot.showGrid(x=True, y=True, alpha=0.5)
        x_axis1 = self.quatPlot.getAxis('bottom')
        y_axis2 = self.quatPlot.getAxis('left')

        x_axis1.setLabel(text='Time in s')  # set axis labels
        y_axis2.setLabel(text='Angels in Degree')

        # initialize sensor data variables
        self.numPoints = 20

        self.pitch_values_euler = np.array([], dtype=float)
        self.roll_values_euler = np.array([], dtype=float)
        self.yaw_values_euler = np.array([], dtype=float)
        self.time_euler = np.array([], dtype=float)

        self.pitch_values_quat = np.array([], dtype=float)
        self.roll_values_quat = np.array([], dtype=float)
        self.yaw_values_quat = np.array([], dtype=float)
        self.time_quat = np.array([], dtype=float)


        # Variables for Time Stamp
        self.oldTimeEuler = time.time()
        self.plotTimeEuler = 0

        self.oldTimeQuat = time.time()
        self.plotTimeQuat = 0

        self.counterEuler = 0
        self.counterQuat = 0

        # Initialization of pitch and roll
        self.phi = 0
        self.theta = 0

        # # self._update()
        self.timer = QtCore.QTimer()
        self.timer.setInterval(5)
        self.timer.timeout.connect(self._update)
        self.timer.start()

        self.pitch_curve_euler = self.eulerPlot.plot(pen='r', name="Pitch")
        self.roll_curve_euler = self.eulerPlot.plot(pen='g', name="Roll")
        self.yaw_curve_euler = self.eulerPlot.plot(pen='y', name="Yaw")

        self.pitch_curve_quat = self.quatPlot.plot(pen='r', name="Pitch")
        self.roll_curve_quat = self.quatPlot.plot(pen='g', name="Roll")
        self.yaw_curve_quat = self.quatPlot.plot(pen='y', name="Yaw")

    def centerOnScreen(self):
        resolution = QDesktopWidget().screenGeometry()
        self.move(int((resolution.width() / 2) - (self.frameSize().width() / 2)),
                  int((resolution.height() / 2) - (self.frameSize().height() / 2)))

    def _update(self):
        while (Data.inWaiting() == 0):
            # time.sleep(0.001)
            pass
        dataPacket = Data.readline()
        dataPacket = str(dataPacket, 'utf-8')
        dataPacket = dataPacket.strip('\r\n')
        splitPacket = dataPacket.split(",")
        print(splitPacket)

        if int(splitPacket[0]) == 0:
            self.counterEuler += 1
            newTimeEuler = time.time()
            deltaTimeEuler = newTimeEuler - self.oldTimeEuler
            self.oldTimeEuler = newTimeEuler
            splitPacket.append(str(deltaTimeEuler))
            self.plotTimeEuler += deltaTimeEuler
            row = splitPacket

            try:
                roll = -computeRollAngle(accy=row[2], accz=row[3], gyrox=row[4], phiOld=self.phi, dt=float(row[11]))
                pitch = -computePitchAngle(accx=row[1], accz=row[3], gyroy=row[5], thetaOld=self.theta,
                                          dt=float(row[11]))
                yaw = computeYawAngle(theta=pitch, phi=roll, magx=row[7], magy=row[8], magz=row[9])
                self.phi = roll
                self.theta = pitch

                self.pitch_values_euler = np.append(self.pitch_values_euler, pitch)
                self.roll_values_euler = np.append(self.roll_values_euler, roll)
                self.yaw_values_euler = np.append(self.yaw_values_euler, yaw)
                self.time_euler = np.append(self.time_euler, self.plotTimeEuler)
                if self.counterEuler > 40:
                    self.pitch_values_euler = np.append(self.pitch_values_euler[1:20], pitch)
                    self.roll_values_euler = np.append(self.roll_values_euler[1:20], roll)
                    self.yaw_values_euler = np.append(self.yaw_values_euler[1:20], yaw)
                    self.time_euler = np.append(self.time_euler[1:20], self.plotTimeEuler)

                self.pitch_curve_euler.setData(self.time_euler, self.pitch_values_euler)
                self.roll_curve_euler.setData(self.time_euler, self.roll_values_euler)
                self.yaw_curve_euler.setData(self.time_euler, self.yaw_values_euler)

                app.processEvents()
            except:
                print("??????????")
                pass

        if int(splitPacket[0]) == 1:
            self.counterQuat += 1
            newTimeQuat = time.time()
            deltaTimeQuat = newTimeQuat - self.oldTimeQuat
            self.oldTimeQuat = newTimeQuat
            splitPacket.append(str(deltaTimeQuat))
            self.plotTimeQuat += deltaTimeQuat
            row = splitPacket

            try:
                q0 = float(row[1])
                q1 = float(row[2])
                q2 = float(row[3])
                q3 = float(row[4])

                roll, pitch, yaw = transformQuatEuler(q0, q1, q2, q3)
                roll = roll * toDeg
                pitch = pitch * toDeg
                yaw = yaw * toDeg

                self.pitch_values_quat = np.append(self.pitch_values_quat, pitch)
                self.roll_values_quat = np.append(self.roll_values_quat, roll)
                self.yaw_values_quat = np.append(self.yaw_values_quat, yaw)
                self.time_quat = np.append(self.time_quat, self.plotTimeQuat)
                #print(self.pitch_values_quat)
                if self.counterQuat > 40:
                    self.pitch_values_quat = np.append(self.pitch_values_quat[1:20], pitch)
                    self.roll_values_quat = np.append(self.roll_values_quat[1:20], roll)
                    self.yaw_values_quat = np.append(self.yaw_values_quat[1:20], yaw)
                    self.time_quat = np.append(self.time_quat[1:20], self.plotTimeQuat)
                #print(self.pitch_values_quat)

                self.pitch_curve_quat.setData(self.time_quat, self.pitch_values_quat)
                self.roll_curve_quat.setData(self.time_quat, self.roll_values_quat)
                self.yaw_curve_quat.setData(self.time_quat, self.yaw_values_quat)

                app.processEvents()
            except:
                print('!!!!!!!!!!!!!!!!!!')
                pass


if __name__ == '__main__':
    file = 'config.ini'
    config = ConfigParser()
    config.read(file)
    com = config["comport"]["port"]
    Data = serial.Serial(com, 38400)
    app = QApplication(sys.argv)
    plot = AngelsPlot()



    plot.show()

    sys.exit(app.exec_())