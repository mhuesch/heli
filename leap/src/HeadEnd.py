
import sys, serial, time
sys.path.insert(0, "../lib")
import Leap
from Leap import CircleGesture, KeyTapGesture, ScreenTapGesture, SwipeGesture

class SampleListener(Leap.Listener):
    ser = None

    def on_init(self, controller):
        self.ser = serial.Serial('/dev/cu.usbmodemfd121')
        print "found serial port: " + self.ser.name
        time.sleep(4)
        print self.ser.write('1\n') # Sync heli
        print "syncing heli"
        time.sleep(11)
        print "done syncing"
        print "begin throttle"
        print self.ser.write('03100\n')
        time.sleep(3)
        print self.ser.write('03050\n')
        time.sleep(3)
        print self.ser.write('03127\n')
        time.sleep(3)



    def on_connect(self, controller):
        print "Connected"

        # Enable gestures
        controller.enable_gesture(Leap.Gesture.TYPE_CIRCLE);
        controller.enable_gesture(Leap.Gesture.TYPE_KEY_TAP);
        controller.enable_gesture(Leap.Gesture.TYPE_SCREEN_TAP);
        controller.enable_gesture(Leap.Gesture.TYPE_SWIPE);

    def on_disconnect(self, controller):
        # Note: not dispatched when running in a debugger.
        print "Disconnected"

    def on_exit(self, controller):
        print "closing serial connection"
        self.ser.close()
        print "Exited"

    def on_frame(self, controller):
        # Get the most recent frame and report some basic information
        frame = controller.frame()

        print "Frame id: %d, timestamp: %d, hands: %d, fingers: %d, tools: %d, gestures: %d" % (
              frame.id, frame.timestamp, len(frame.hands), len(frame.fingers), len(frame.tools), len(frame.gestures()))

        if not frame.hands.is_empty:
            # Get the first hand
            hand = frame.hands[0]

            # Get palm position y componenet
            palm_y = hand.palm_position.y
            max_height = 400
            min_height = 50
            max_throttle = 127
            min_throttle = 0
            throttle = None
            print "Palm position: %f" % (palm_y)
            if palm_y > max_height:
                throttle = max_throttle
            elif palm_y < min_height:
                throttle = min_throttle
            else:
                throttle = ((palm_y - min_height) / (max_height - min_height) * max_throttle)

            time.sleep(0.2)
            instr = '03%03i' % int(throttle)
            print "Arduino says: "
            print self.ser.readline()
            print instr
            print "sent this many bytes to arduino: "
            print self.ser.write(instr)

            # Get the hand's normal vector and direction
            normal = hand.palm_normal
            direction = hand.direction

            # Calculate the hand's pitch, roll, and yaw angles
            print "Hand pitch: %f degrees, roll: %f degrees, yaw: %f degrees" % (
                direction.pitch * Leap.RAD_TO_DEG,
                normal.roll * Leap.RAD_TO_DEG,
                direction.yaw * Leap.RAD_TO_DEG)

        if not (frame.hands.is_empty and frame.gestures().is_empty):
            print ""

    def state_string(self, state):
        if state == Leap.Gesture.STATE_START:
            return "STATE_START"

        if state == Leap.Gesture.STATE_UPDATE:
            return "STATE_UPDATE"

        if state == Leap.Gesture.STATE_STOP:
            return "STATE_STOP"

        if state == Leap.Gesture.STATE_INVALID:
            return "STATE_INVALID"


def main():
    # Create a sample listener and controller
    listener = SampleListener()
    controller = Leap.Controller()

    # Have the sample listener receive events from the controller
    controller.add_listener(listener)

    # Keep this process running until Enter is pressed
    print "Press Enter to quit..."
    sys.stdin.readline()

    # Remove the sample listener when done
    controller.remove_listener(listener)

if __name__ == "__main__":
    main()
