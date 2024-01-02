# -*- coding: utf-8 -*-
"""
Created on Sun Dec 31 10:24:45 2023

@author: reach
"""

import tkinter as tk
from tkinter import ttk
import webbrowser
import folium
import os

from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.chrome.service import Service as ChromeService
from selenium import webdriver

def show_map(latitude, longitude):
    # Create a folium map centered at the specified location
    map_center = [latitude, longitude]
    my_map = folium.Map(location=map_center, zoom_start=15)

    # create a browser object    	
    service = ChromeService(executable_path=ChromeDriverManager().install())
    browser = webdriver.Chrome(service=service)

    # Add a marker at the specified location
    folium.Marker(location=map_center, popup="Location").add_to(my_map)

    # Save the map to an HTML file
    map_filename = "map.html"
    my_map.save(map_filename)

    # Open the map in the default web browser
    #webbrowser.open('file://' + os.path.realpath(map_filename))
    browser.get('file://' + os.path.realpath(map_filename))

def on_submit():
    try:
        # Get latitude and longitude from user input
        latitude = float(latitude_entry.get())
        longitude = float(longitude_entry.get())

        # Display the map
        show_map(latitude, longitude)
    except ValueError:
        result_label.config(text="Invalid input. Please enter valid numbers.")

# Create the main window
main_window = tk.Tk()
main_window.title("Maps API GUI")

# Create and place widgets
latitude_label = ttk.Label(main_window, text="Latitude:")
latitude_label.grid(row=0, column=0, padx=10, pady=10, sticky='e')

latitude_entry = ttk.Entry(main_window)
latitude_entry.grid(row=0, column=1, padx=10, pady=10)

longitude_label = ttk.Label(main_window, text="Longitude:")
longitude_label.grid(row=0, column=2, padx=10, pady=10, sticky='e')

longitude_entry = ttk.Entry(main_window)
longitude_entry.grid(row=0, column=3, padx=10, pady=10)

submit_button = ttk.Button(main_window, text="Show Map", command=on_submit)
submit_button.grid(row=0, column=4, padx=10, pady=10)

result_label = ttk.Label(main_window, text="")
result_label.grid(row=2, column=0, columnspan=3)

# Run the main loop
main_window.mainloop()
