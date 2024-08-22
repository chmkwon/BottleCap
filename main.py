import socket
import struct
import cv2
import numpy as np
from tensorflow.keras.models import load_model

class Protocol:
    DISCONNECT = 0x03
    SUCCESS = 0x01
    FAIL = 0x02
    CHECK_RESULT = 0x01
    SHOW_CHART = 0x02

DELIMITER = ";"

class PyServer:
    def __init__(self, host='localhost', port=9935):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((self.host, self.port))
        self.model = load_model('bootleCap.h5')

    def start(self):
        self.sock.listen(5)
        print(f"서버 시작 {self.host}:{self.port}")
        while True:
            try:
                client_socket, address = self.sock.accept()
                print(f"새로운 클라이언트 연결: {address}")
                self.handle_client(client_socket)
            except Exception as e:
                print(f"클라이언트 연결 수락 중 오류 발생: {e}")

    def handle_client(self, client_socket):
        try:
            while True:
                protocol = self.recv_protocol(client_socket)
                if protocol == Protocol.CHECK_RESULT:
                    print("클라이언트가 뚜껑 분류 요청")
                    image = self.recv_image(client_socket)
                    predict, confidence = self.predict_image(image)
                    print(f"예측: {predict}, 신뢰도: {confidence:.2f}%")
                    self.send_message(client_socket, predict + DELIMITER + f"{confidence:.2f}")
                elif protocol == Protocol.DISCONNECT:
                    print("클라이언트가 연결 종료 요청")
                    break
                else:
                    print(f"알 수 없는 프로토콜: {protocol}")
        except ConnectionResetError:
            print("클라이언트 연결이 강제로 종료됨")
        except Exception as e:
            print(f"클라이언트 처리 중 오류 발생: {e}")
        finally:
            client_socket.close()
            print("클라이언트 연결 종료")

    def recv_protocol(self, client_socket):
        data = client_socket.recv(1)
        if not data:
            raise ConnectionResetError("클라이언트가 연결을 종료")
        return struct.unpack('B', data)[0]

    def recv_message(self, client_socket):
        length_bytes = client_socket.recv(4)
        length = struct.unpack('i', length_bytes)[0]
        message = client_socket.recv(length).decode('utf-8')
        print(f"받은 메시지: {message}")
        return message

    def send_message(self, client_socket, message):
        client_socket.send(message.encode('utf-8') + b'\0')
        print("전송완")

    def recv_image(self, client_socket):
        size_data = client_socket.recv(4)
        size = struct.unpack('i', size_data)[0]
        print(f"총 이미지 사이즈: {size}")
        data = b''
        while len(data) < size:
            packet = client_socket.recv(size - len(data))
            if not packet:
                return None
            data += packet
        nparr = np.frombuffer(data, np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        return img

    def predict_image(self, image):
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        resized = cv2.resize(gray, (224, 224))
        img_array = np.expand_dims(resized, axis=[0, -1]) / 255.0

        prediction = self.model.predict(img_array)
        class_labels = ['YesCap', 'YesCap', 'NoCap']

        predicte = class_labels[np.argmax(prediction)]
        confidence = np.max(prediction) * 100

        return predicte, confidence

server = PyServer()
server.start()