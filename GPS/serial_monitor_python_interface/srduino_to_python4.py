import serial
import time
import schedule

def main_func():
    arduino = serial.Serial('com8', 9600)
    print('Established serial connection to Arduino')

    while True:
        arduino_data = arduino.readline()

        # Decode with error handling to replace invalid byte sequences
        decoded_values = str(arduino_data[0:len(arduino_data)].decode("utf-8", errors='replace'))
        list_values = decoded_values.split('xxxx')

        for item in list_values:
            # Strip 'x' from both ends
            cleaned_item = item.strip('x')

            print(f'Value: {cleaned_item}')

        time.sleep(1)  # Add a delay to control the rate of reading from Arduino

    arduino.close()
    print('Connection closed')
    print('<----------------------------->')

# ----------------------------------------Main Code------------------------------------
# Declare variables to be used
list_values = []

print('Program started')

# Setting up the Arduino
schedule.every(10).seconds.do(main_func)

while True:
    schedule.run_pending()
    time.sleep(1)