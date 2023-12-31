# -*- coding: utf-8 -*-
"""
Created on Sun Dec 31 10:03:08 2023

@author: reach
"""

import serial




def read_serial_data(port, baud_rate, num_samples):
    # Open serial connection
    ser = serial.Serial(port, baud_rate)

    # Initialize an empty list to store data
    data_list = []

    try:
        # Read data from serial port
        for _ in range(num_samples):
            # Read a line from the serial port
            serial_data = ser.readline().decode('utf-8').strip()

            # Print the received data (optional)
            #print(serial_data)

            # Append the data to the list
            data_list.append(serial_data)

    except KeyboardInterrupt:
        print("Serial reading interrupted by user.")

    finally:
        # Close the serial connection
        ser.close()

    return data_list





if __name__ == "__main__":
    # Specify the serial port, baud rate, and number of samples
    serial_port = 'COMx'  # Change this to your Arduino's serial port
    baud_rate = 9600
    num_samples = 1

    # Read data from serial port
    data_list = read_serial_data(serial_port, baud_rate, num_samples)

    # Print the collected data
    print("\nCollected Data:")
    for data in data_list:
        print(data)
