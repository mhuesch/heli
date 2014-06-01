import glob

port_options = glob.glob('/dev/cu.usbmodem*')
selected_port = "-1"
for port in port_options:
    print "1: " + port

selected_port = port_options[int(raw_input("Please select the USB port on which the Arduino is connected to your computer: "))-1]
print selected_port

    