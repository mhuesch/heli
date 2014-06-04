import time, sys, serial

ser = serial.Serial('/dev/cu.usbmodemfa131')
ser.setBaudrate(9600)
print "found serial port: " + ser.name
time.sleep(4)
print ser.write('1\n') # Sync heli
print "syncing heli"
time.sleep(11)
print "done syncing"

x = 1
while True:
    try:
        time.sleep(.2)
        instr = '03%03i\n' % ((x % 50) + 10)
        print repr(instr)
        ser.write(instr)
        x += 1
    except KeyboardInterrupt:
        print "Bye"
        ser.close()
        sys.exit()
