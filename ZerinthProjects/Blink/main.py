###############################################################################
# Led Blink
#
# Created by Zerynth Team 2015 CC
# Authors: G. Baldi, D. Mazzei
###############################################################################

# D0 to D127 represent the names of digital pins
# On most Arduino-like boards Pin D13 has an on-board LED connected.
# However Zerynth abstracts the board layout allowing to use LED0, LED1, etc as led names.
# In this example LED0 is used.

import streams
import gc
import threading

# create a lock
lock = threading.Lock()

# define a function to be launched as a thread
def threadfun(msg):
    while True:
        # if it's unlocked, lock it and continue printing. Else wait.
        lock.acquire()
        print(msg)
        sleep(50)
        # unlock and allow another thread to call the print
        lock.release()



# creates a serial port and name it "s"
s=streams.serial()
pinMode(D8,OUTPUT)

counter = 0

# launch thread 1
thread(threadfun,"Hello")

# launch thread 2
thread(threadfun,"World")

# loop forever
while True:
        digitalWrite(D8, HIGH)  # turn the LED ON by setting the voltage HIGH
        sleep(1000)               # wait for a second
        digitalWrite(D8, LOW)   # turn the LED OFF by setting the voltage LOW
        sleep(1000)               # wait for a second
        lock.acquire()
        print(gc.info())
        print(counter)
        lock.release()
        counter = counter + 1


