import socket
import time

# Replace this with the actual IP address of your ESP32
server_ip = "192.168.10.2"  # Replace with your ESP32's IP address
server_port = 8080  # Port number used in the ESP32 code

def connect_to_server():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.settimeout(10)  # Set timeout for the connection
    connected = False

    for attempt in range(5):  # Try to connect 5 times
        try:
            client_socket.connect((server_ip, server_port))
            print("Connected to server at {}:{}".format(server_ip, server_port))
            connected = True
            break
        except (socket.timeout, socket.error) as e:
            print(f"Connection attempt {attempt + 1} failed: {e}")
            time.sleep(2)

    if not connected:
        print("Failed to connect after multiple attempts.")
        return None 

    return client_socket

def main():
    while True:
        client_socket = connect_to_server()
        if not client_socket:
            time.sleep(5)  # Wait before retrying to connect
            continue

        try:
            while True:
                data = client_socket.recv(1024)
                if not data:
                    print("Server disconnected. Reconnecting...")
                    break  # Exit to reconnect loop
                print("Received data:", data.decode('utf-8').strip())
        except socket.error as e:
            print("Communication error:", e)
        finally:
            client_socket.close()
            print("Socket closed. Reconnecting...")
            time.sleep(2)  # Optional delay before reconnecting

if __name__ == "__main__":
    main()
