import streams
import threading

# open a serial port: our resource
s = streams.serial("ser1")
# create a lock
lock = threading.Lock()

# define a function to be launched as a thread
def threadfun(msg):
    while True:
        # if it's unlocked, lock it and continue printing. Else wait.
        lock.acquire()
        print(msg)
        sleep(500)
        # unlock and allow another thread to call the print
        lock.release()

# launch thread 1
thread(threadfun,"Hello")

# launch thread 2
thread(threadfun,"World")