import sys
import datetime
import xml.etree.ElementTree as ET

def parse_rmc(line):
    if not line.startswith('$') or 'RMC' not in line:
        print("return")
        return None

    parts = line.strip().split(',')
    if len(parts) < 12 or parts[2] != 'A':  # Check for Active fix
        return None

    try:
        # Latitude
        lat_deg = float(parts[3][:2])
        lat_min = float(parts[3][2:])
        lat = lat_deg + lat_min / 60.0
        if parts[4] == 'S':
            lat = -lat
        # Longitude
        lon_deg = float(parts[5][:3])
        lon_min = float(parts[5][3:])
        lon = lon_deg + lon_min / 60.0
        if parts[6] == 'W':
            lon = -lon
        # Date and time
        time_str = parts[1].split('.')[0]
        date_str = parts[9]
        dt = datetime.datetime.strptime(date_str + time_str, '%d%m%y%H%M%S')
        dt_iso = dt.isoformat() + 'Z'
        return {'lat': lat, 'lon': lon, 'time': dt_iso}
    except (ValueError, IndexError):
        return None

def create_gpx(points):
    gpx = ET.Element('gpx', version="1.1", creator="NMEA to GPX Converter",
                     xmlns="http://www.topografix.com/GPX/1/1")
    trk = ET.SubElement(gpx, 'trk')
    trkseg = ET.SubElement(trk, 'trkseg')

    for pt in points:
        trkpt = ET.SubElement(trkseg, 'trkpt', lat=str(pt['lat']), lon=str(pt['lon']))
        time_elem = ET.SubElement(trkpt, 'time')
        time_elem.text = pt['time']

    return ET.tostring(gpx, encoding='unicode', method='xml')

def main():
    if len(sys.argv) != 2:
        print("Usage: python nmea_to_gpx.py <input_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    try:
        with open(input_file, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        sys.exit(1)

    points = []
    for line in lines:
        point = parse_rmc(line)
        if point:
            points.append(point)

    if not points:
        print("No valid RMC data found.")
        sys.exit(1)

    gpx_output = create_gpx(points)
    print(gpx_output)

if __name__ == '__main__':
    main()
