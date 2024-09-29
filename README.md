# Smart Garden Project

This project is an advanced embedded system designed for automatic control and monitoring of a home garden. The system utilizes various sensors, such as temperature, humidity, soil moisture, and light intensity, to optimize plant growth conditions with minimal user intervention. The system is powered by an Arduino microcontroller and includes automated irrigation based on real-time sensor readings.

## Project Overview

The Smart Garden project aims to provide a solution for managing garden environments by monitoring essential environmental conditions and controlling a water pump for irrigation. The data collected from the sensors is analyzed and stored, allowing the system to adapt to changing conditions. The project involves both hardware and software development.

### Key Features:
- **Automatic Irrigation**: Based on soil moisture levels and preset thresholds.
- **Real-Time Monitoring**: Live display of temperature, humidity, soil moisture, and light intensity on an LCD.
- **Data Logging**: Sensor data is recorded in a CSV file for further analysis.
- **User Interface**: A simple menu system using buttons allows users to set watering times, plant types, and sensor thresholds.
- **Custom Driver**: A custom driver is built to handle sensor readings efficiently, ensuring accurate and reliable data collection from the DHT11 sensor, soil moisture sensor, and LDR.

## Custom Driver Functionality

The custom driver developed for the Smart Garden system handles data collection from the sensors and ensures reliable communication between the Arduino and the sensors. Below is a summary of the key functionalities of the driver:

1. **Sensor Initialization**: 
   - The driver initializes communication with the sensors, setting up the appropriate pins and configurations for reading data.

2. **DHT11 Sensor Handling**:
   - The driver starts by lowering the communication line, waits for a response, and then reads 40 bits of data from the sensor.
   - It processes and verifies the data, calculating the temperature and humidity values. A checksum is used to ensure data integrity. If the checksum fails, an error is reported&#8203;:contentReference[oaicite:0]{index=0}.

3. **Soil Moisture Sensor Handling**:
   - The soil moisture sensor reads analog values from the soil. The driver converts these values to percentage moisture content, determining if the soil is dry or moist.
   - The driver provides error handling in case of sensor failure&#8203;:contentReference[oaicite:1]{index=1}.

4. **Light Intensity (LDR) Sensor**:
   - The driver reads the light intensity by measuring the resistance of the LDR. This value is then converted to lux using a mathematical formula based on the sensor's characteristics.
   - The system can determine if it is day or night based on the light intensity level, which is crucial for scheduling irrigation and monitoring plant growth&#8203;:contentReference[oaicite:2]{index=2}.

5. **Real-Time Communication**:
   - The sensor data is sent to both the LCD for live display and to the computer via serial communication for logging purposes.

This driver ensures that the Smart Garden system operates reliably and efficiently, providing accurate real-time data to optimize garden conditions.

## Components

- **Arduino Microcontroller**: Controls the sensors, pump, and display.
- **DHT11 Sensor**: Measures temperature and humidity.
- **Soil Moisture Sensor**: Monitors the moisture level of the soil.
- **LDR (Light Dependent Resistor)**: Measures light intensity.
- **Water Pump**: Activates irrigation based on sensor readings.
- **LCD Display**: Provides real-time feedback to the user.
- **Buttons**: Allow the user to interact with the system and set configurations.
- **EEPROM**: Stores user settings such as watering schedules and sensor thresholds.

## Files and Directories

### `src/`
Contains the source code for the project:
- `python_code.py`: Python script for capturing and logging sensor data via serial communication&#8203;:contentReference[oaicite:3]{index=3}.
- `cpp_code.cpp`: Arduino C++ code for managing the sensors, pump, and user interface, including the custom driver&#8203;:contentReference[oaicite:4]{index=4}.

### `data/`
Contains the logged sensor data:
- `sensor_data.csv`: CSV file where the sensor readings are stored for later analysis.

### `docs/`
Contains the project documentation:
- `project_summary.pdf`: A comprehensive report that summarizes the project, including the system design, results, and conclusions&#8203;:contentReference[oaicite:5]{index=5}.
- `system_diagram.png`: Diagram showing the system architecture and connections between components.

### `media/`
Contains media related to the project:
- `system_video.mp4`: A video demonstrating the functionality of the Smart Garden system.
- `system_photos.png`: Images showing the system setup and components.

## How to Run the Project

### Hardware Setup:
1. Connect the sensors (DHT11, Soil Moisture, LDR) and the water pump to the Arduino as per the system diagram.
2. Upload the C++ code (`cpp_code.cpp`) to the Arduino using the Arduino IDE.
3. Ensure the sensors and LCD display are functioning correctly.

### Software Setup:
1. Run the Python script (`python_code.py`) to capture real-time sensor data and store it in a CSV file for future analysis.
2. Use a serial connection to send the sensor data from the Arduino to your computer.

### Video and Images:
Watch the video in the `media/system_video.mp4` file to see the Smart Garden in action. Images of the system are available in `media/system_photos.png`.

## Conclusion

The Smart Garden project successfully automates garden maintenance, providing an efficient and low-effort solution for plant care. The system's flexibility allows it to be adapted to various plants and environmental conditions, making it a valuable tool for home gardeners.

---

© 2024 Daniel Ram and Team. Licensed under the MIT License.
