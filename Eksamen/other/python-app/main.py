import socket
import calcs
import sys
import websocket

def main():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        print("Connecting...")
        client.connect(('192.168.4.1', 1337))
        print("Connected!")
        path = sys.argv[1]
        svg_handler = calcs.SvgHandler(path)
        positions = svg_handler.interpret()
        print(positions)
        # client.send(f"points:?".encode('utf-8'))
        client.send(f"points:?".encode('ascii'))
        client.send(b"points:?")
        print("Done!")
    except Exception as e:
        print("Error:", e)
    client.close()

if __name__ == "__main__":
    main()
