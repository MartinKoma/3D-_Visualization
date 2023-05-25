import csv
import math
import numpy as np


# global variables
toRad = 2 * np.pi / 360
toDeg = 1 / toRad


# def readDataFromTXT():
#     with open('bewegung.txt', 'r') as csv_file:
#         csv_reader = csv.reader(csv_file)
#
#         next(csv_reader)
#         next(csv_reader)
#
#         array = list(csv_file)
#         return array


def computePitchAngle(accx, accz, gyroy, thetaOld, dt):
    """
    This function computes the Pitch Angle
    :param accx: value of accel. in x-direction
    :param accz: value of accel. in z-direction
    :param gyroy: rotational velocity of y-axis
    :param thetaOld: pitch angle of last measurement
    :param dt: temporal distance btw. two measurements
    :return: pitch angle in degrees
    """
    accx = float(accx)
    accz = float(accz)
    gyroy = float(gyroy)
    thetaMeasured = -math.atan2(accx / 9.8, accz / 9.8) / 2 / np.pi * 360
    theta = (thetaOld + gyroy * dt) * 0.95 + thetaMeasured * 0.05
    return theta


def computeRollAngle(accy, accz, gyrox, phiOld, dt):
    """
    This function computes the Roll Angle
    :param accy: value of accel. in y-direction
    :param accz: value of accel. in z-direction
    :param gyrox: rotational velocity of x-axis
    :param phiOld: roll angle of last measurement
    :param dt: temporal distance btw. two measurements
    :return: roll angle in degrees
    """
    accy = float(accy)
    accz = float(accz)
    gyrox = float(gyrox)
    phiMeasured = math.atan2(accy / 9.8, accz / 9.8) / 2 / np.pi * 360
    phi = (phiOld - gyrox * dt) * 0.95 + phiMeasured * 0.05
    return phi


def computeYawAngle(theta, phi, magx, magy, magz):
    """
    This function computes the yaw angle
    :param theta: computed pitch angle
    :param phi: computed roll angle
    :param magx: value of magnetometer in x-direction
    :param magy: value of magnetometer in y-direction
    :param magz: value of magnetometer in z-direction
    :return: yaw angle in degrees
    """
    magx = float(magx)
    magy = float(magy)
    magz = float(magz)
    phiRad = phi
    thetaRad = theta

    xMagnetometer = magx * math.cos(thetaRad) - magy * math.sin(phiRad) * math.sin(thetaRad) + magz * math.cos(
        phiRad) * math.sin(thetaRad)
    yMagnetometer = magy * math.cos(phiRad) + magz * math.sin(phiRad)

    yaw = math.atan2(xMagnetometer, yMagnetometer) * toDeg
    return -yaw


def transformQuatEuler(q0, q1, q2, q3):
    """
    This functions converts quaternions into euler angles
    :param q0: first value of quaternion
    :param q1: second value of quaternion
    :param q2: third value of quaternion
    :param q3: fourth value of quaternion
    :return: euler angels in radians
    """
    roll = -math.atan2(2 * (q0 * q1 + q2 * q3), 1 - 2 * (q1 * q1 + q2 * q2))
    pitch = math.asin(2 * (q0 * q2 - q3 * q1))
    yaw = -math.atan2(2 * (q0 * q3 + q1 * q2), 1 - 2 * (q2 * q2 + q3 * q3))  # - np.pi/2

    return roll, pitch, yaw


