import serial
import csv
import time

# הגדרת החיבור הסדרתי
ser = serial.Serial('COM6', 9600, timeout=1)
time.sleep(2)  # המתנה ליצירת החיבור

# שם הקובץ לכתיבה
csv_filename = 'sensor_data.csv'

# כתיבת כותרות לקובץ אם הוא חדש
with open(csv_filename, 'w', newline='') as csvfile:
    fieldnames = ['Date', 'Time', 'Temperature (C)', 'Humidity (%)', 'Soil Moisture (%)', 'Light Intensity (lux)']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()

# פתיחת הקובץ להמשך כתיבה
while True:
    line = ser.readline().decode('utf-8').rstrip()
    if line:
        try:
            # פיצול לפי סימן פסיק בין הנתונים
            parts = line.split(', ')
            temperature = float(parts[0].split(': ')[1].replace(' C', '').strip())
            humidity = float(parts[1].split(': ')[1].replace(' %', '').strip())
            soil_moisture = float(parts[2].split(': ')[1].replace(' %', '').strip())
            light_intensity = float(parts[3].split(': ')[1].replace(' lux', '').strip())

            # הדפסת הערכים לקונסול
            print(f"Temperature: {temperature} C")
            print(f"Humidity: {humidity} %")
            print(f"Soil Moisture: {soil_moisture} %")
            print(f"Light Intensity: {light_intensity} lux")
            print("-" * 40)

            # כתיבת הנתונים ל-CSV
            timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
            date, time_part = timestamp.split(' ')
            with open(csv_filename, 'a', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow([date, time_part, temperature, humidity, soil_moisture, light_intensity])

        except (IndexError, ValueError) as e:
            print(f"Error processing line: {line} | Error: {e}")
            
  #time.sleep(120) 