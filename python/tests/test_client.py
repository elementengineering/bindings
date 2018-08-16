import uWebSockets

import threading
import time
import sys
import uuid
import json

"""
    A Test Client which sends random messages to wss://echo.websocket.org and verifies the replies
"""
class TestClient(uWebSockets.WebSocketClient):

    def __init__(self, *args, **kwargs):
        """
            Implement pre-connection initialisation here
        """
        super(TestClient, self).__init__(*args, **kwargs)
        self.expected = None
        self.sent_received = {}
        
    def on_message(self, message):
        """
            Handle messages here. Message is a string.
            If we get an exception, the websocket will disconnect automatically.
        """
        print("Recv %s" % repr(message))
        assert(message == self.expected)
        assert(message in self.sent_received)
        assert(self.sent_received[message] == False)
        self.sent_received[message] = True
        self.expected = str(uuid.uuid4().hex).upper()[0:6]
        self.send(self.expected)
        
    def test(self):
        print("What")
        
    def send(self, message):
        """
            Overridable method to send messages
        """
        print("Send %s" % repr(message))
        self.sent_received[message] = False
        return super(TestClient, self).send(message)
    
    def on_open(self):
        """
            Implement post-connection initialisation here
        """
        print("Open %s" % repr(self))
        self.expected = str(uuid.uuid4().hex).upper()[0:6]
        self.send(self.expected)
        
    def on_close(self, code, message):
        print("Closed %s %s" % (repr(code), repr(message)))
        
        
    def close(self):
        """
            Overridable method for closing
        """
        print("Overrode close here")
        return super(TestClient, self).close()
        
num_messages = 10
timeout = 20

ws = TestClient("wss://echo.websocket.org") # Initialise with URI
ws.run(True) # Argument is True to run in a seperate (C++ not python) thread, False to run within current thread.
start_time = time.time()
time.sleep(3)

# ws.valid() becomes True after first ever connection and remains True forever
# ws.connected() is actually connection state.
#  We need to check both so we don't break out of the loop before we even connected.
while (not ws.valid() or ws.connected()) \
  and (num_messages is None or len(ws.sent_received) < num_messages) \
  and (timeout is None or time.time() - start_time < timeout):
    """
    Do other application tasks here.
    """
    time.sleep(1)

print(json.dumps(ws.sent_received, indent=4))
ws.close()
assert(len(ws.sent_received) >= num_messages) # Check test passed
## Required on Windoze
sys.exit(0)
