import socketio

def ack_received(data1, data2=""):
    print("  Status: ack received", flush=True)
    return

class sioClient:
    def __init__(self):
        self.sio = socketio.Client(reconnection=False, request_timeout=100)

        @self.sio.event
        def connect():
            print("   SocketIO connected", flush=True)
        @self.sio.event
        def connect_error(erro=None):
            print("   SocketIO Connection failed!",erro, flush=True)
            self.sio.wait()
        @self.sio.event
        def disconnect():
            print("   SocketIO  disconnected", flush=True)
        @self.sio.on('*')
        def catch_all(event, data):
            print("   Received:", event, data, flush=True)

            pass
    def doConnect(self, address, headers):
        self.sio.connect(address, headers)
    def doMessage(self, command, data, cb_func=None):
        self.sio.emit(command, data, callback=cb_func)
    def doDisconnect(self):
        self.sio.disconnect()
    def finish(self):
        self.sio.disconnect()
        self.sio.wait()