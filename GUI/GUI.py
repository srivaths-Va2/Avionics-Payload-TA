import folium
import time
import serial
#import schedule

def refresh_html_page(html_file_path):
    with open(html_file_path, 'r') as file:
        html_content = file.read()

    # Add JavaScript script for page reload
    script = '<script>setTimeout(function(){location.reload();}, 5000);</script>'
    html_content = html_content.replace('</body>', script + '</body>')

    # Write the modified content back to the HTML file
    with open(html_file_path, 'w') as file:
        file.write(html_content)

def update_map(gmap, latitude, longitude, popup_text):
    marker = folium.Marker(location=[latitude, longitude], popup=popup_text)
    marker.add_to(gmap)
    gmap.center = [latitude, longitude]
    return gmap

def GPS_to_Serial():
    arduino = serial.Serial('com8', 9600)
    print('Established serial connection to Arduino')

    while True:
        arduino_data = arduino.readline()
        # Decode with error handling to replace invalid byte sequences
        decoded_values = str(arduino_data[0:len(arduino_data)].decode("utf-8", errors='replace'))
        if('xxxx' in decoded_values):
            list_values = decoded_values.split('xxxx')

            '''
            for item in list_values:
                # Strip 'x' from both ends
                cleaned_item = item.strip('x')

                return cleaned_item

                #print(f'Value: {cleaned_item}')

            '''

            time.sleep(1)  # Add a delay to control the rate of reading from Arduino
        
            return list_values
        else:
            continue

    #arduino.close()
    #print('Connection closed')
    #print('<----------------------------->')



def main():

    # Initializing lists to dynamically store latitude and longitude
    latitude_L = []
    longitude_L = []
    # Initial location
    pre_latitude = float(input("Enter the intial latitude : "))
    pre_longitude = float(input("Enter the intial longitude : "))

    latitude_L.append(pre_latitude)
    longitude_L.append(pre_longitude)
    
    #pre_latitude =  latitude_L[len(latitude_L) - 1]     # Replace with your starting latitude
    #pre_longitude = longitude_L[len(longitude_L) - 1]   # Replace with your starting longitude

    # Create an interactive map centered at the initial location
    gmap = folium.Map(location=[pre_latitude, pre_longitude], zoom_start=20)

    # Display the map
    gmap.save("live_location_tracking.html")  # Save to an HTML file
    print("Map opened in default web browser. Press Ctrl+C to stop.")

    try:
        while True:
            # Update the map with the current location
            current_latitude = GPS_to_Serial()[0]  # Replace with your method of getting latitude
            current_longitude = GPS_to_Serial()[1]  # Replace with your method of getting longitude

            popup_text = f"Latitude: {current_latitude}, Longitude: {current_longitude}"
            gmap = update_map(gmap, current_latitude, current_longitude, popup_text)

            # Add the latitude and longitude to the lists
            latitude_L.append(current_latitude)
            longitude_L.append(current_longitude)

            # Save the map to an HTML file and open it in the default web browser
            gmap.save("live_location_tracking.html")
            time.sleep(5)  # Update every 5 seconds (adjust as needed)

            # Refresh the HTML page
            refresh_html_page("live_location_tracking.html")

    except KeyboardInterrupt:
        print("Live location tracking stopped.")

if __name__ == "__main__":
    main()
