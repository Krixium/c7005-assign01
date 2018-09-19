import sys
import socket

serverhost = 'localhost'
serverport = 7005

message = b'\x02Gbig.txt'

remaining = 262 - len(message)
padding = b'\00' * remaining

message += padding

print(message)
print(len(message))

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((serverhost, serverport))

s.send(message)

s.close()

ss = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
ss.bind((serverhost, 7006))
ss.listen(5)
conn, addr = ss.accept()
print("connected address: ", addr)

data = conn.recv(45)
print(data)

ss.close()