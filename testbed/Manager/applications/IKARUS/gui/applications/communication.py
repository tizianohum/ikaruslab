import serial
import time
import threading
import dataclasses
import struct
import ctypes

PAYLOAD_LENGTH = 100

# === Nachricht-Struktur (C-kompatibel) ===
@dataclasses.dataclass
class Message:
    start: int = 0xAA
    msg_type: int = 0
    payload_length: int = 0
    payload: bytes = b""
    crc: int = 0

    def pack(self) -> bytes:
        """Packt das Message-Objekt zu einem C-kompatiblen Bytearray."""
        header = struct.pack("<BBB", self.start, self.msg_type, self.payload_length)
        payload_padded = self.payload.ljust(PAYLOAD_LENGTH, b"\x00")
        crc_value = (sum(header + payload_padded) & 0xFF)
        self.crc = crc_value
        return header + payload_padded + struct.pack("<B", crc_value)

    @staticmethod
    def unpack(data: bytes) -> "Message":
        start, msg_type, payload_length = struct.unpack_from("<BBB", data, 0)
        payload = data[3:3 + PAYLOAD_LENGTH]
        #crc = struct.unpack_from("<B", data, 3 + PAYLOAD_LENGTH)[0]
        return Message(start, msg_type, payload_length, payload[:payload_length], 0)


# === Payload-Struktur für Thrust (ctypes für Byte-Kompatibilität) ===
class MotorThrust(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('motor1', ctypes.c_float),
        ('motor2', ctypes.c_float),
        ('motor3', ctypes.c_float),
        ('motor4', ctypes.c_float),
    ]
class Sample(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("roll", ctypes.c_float),
        ("pitch", ctypes.c_float),
        ("yaw", ctypes.c_float),
    ]

# === Nachrichten-IDs (müssen zum STM32 passen) ===
IKARUS_MSG_THRUST = 1
IKARUS_MSG_ARMING = 0
IKARUS_MSG_PITCH  = 2
IKARUS_MSG_ROLL   = 3
IKARUS_MSG_YAW    = 4

IKARUS_MSG_MOTOR1 = 5
IKARUS_MSG_MOTOR2 = 6
IKARUS_MSG_MOTOR3 = 7
IKARUS_MSG_MOTOR4 = 8

IKARUS_MSG_SAMPLE_UPDATE = 10


# ============================================================
#                     COMMUNICATION-CLASS
# ============================================================
class Communication:
    roll = 0.0
    pitch = 0.0
    yaw = 0.0

    def __init__(self, port="/dev/tty.usbserial-A5069RR4", baud=9600):
        self.ser = serial.Serial(port, baudrate=baud, timeout=1)

        # RX-Thread starten
        self.rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
        self.rx_thread.start()

        print("UART Communication gestartet.")

    # ===== RX LOOP =====
    import struct
    from collections import namedtuple
    import time


    def _rx_loop(self):
        """Thread zum Lesen kompletter UART-Pakete (104 Bytes) ab Startbyte 0xAA."""
        PACKET_LENGTH = 104
        TIMEOUT = 0.05  # Timeout für fragmentierte Pakete

        buffer = bytearray()

        while True:
            try:
                # 1. Lese Byte für Byte, bis Startbyte 0xAA gefunden ist
                while True:
                    byte = self.ser.read(1)
                    if not byte:
                        continue
                    if byte[0] == 0xAA:
                        buffer = bytearray(byte)  # Startbyte ins Paket
                        break

                # 2. Lese die restlichen Bytes bis zur vollen Paketlänge
                start_time = time.time()
                while len(buffer) < PACKET_LENGTH:
                    chunk = self.ser.read(PACKET_LENGTH - len(buffer))
                    if chunk:
                        buffer.extend(chunk)
                        start_time = time.time()
                    elif (time.time() - start_time) > TIMEOUT:
                        print("Timeout beim Lesen des Pakets")
                        buffer = bytearray()
                        break

                if len(buffer) == PACKET_LENGTH:
                    #print(f"Empfangenes Paket (Hex): {buffer.hex()}")

                    # 3. Payload extrahieren (angenommen Header 3 Bytes: Start, Type, Length)
                    payload_len = buffer[2]
                    if payload_len >= 12:
                        payload_bytes = buffer[3:3 + 12]  # erste 3 floats
                        self.pitch, self.roll, self.yaw = struct.unpack('<fff', payload_bytes)
                        print("→ Empfangen: Pitch =", self.pitch, "Roll =", self.roll, "Yaw =", self.yaw)

            except serial.SerialException:
                print("UART geschlossen.")
                break

    # ----------------------------------------------------------
    #               MESSAGE SEND HELPER
    # ----------------------------------------------------------
    def _send_message(self, msg_type: int, payload: bytes):
        msg = Message(msg_type=msg_type, payload_length=len(payload), payload=payload)
        packet = msg.pack()
        self.ser.write(packet)
        return packet

    # ----------------------------------------------------------
    #                    ARM / DISARM
    # ----------------------------------------------------------
    def send_arming(self, state: bool):
        payload = bytes([1 if state else 0])
        pkt = self._send_message(IKARUS_MSG_ARMING, payload)
        print("→ Gesendet:", "ARM" if state else "DISARM", f"(Paketgröße {len(pkt)} Bytes)")

    # ----------------------------------------------------------
    #                 MOTORTHRUST (alle 4)
    # ----------------------------------------------------------
    def send_motor_thrust(self, m1: float, m2: float, m3: float, m4: float):
        thrust_struct = MotorThrust(m1, m2, m3, m4)
        payload = bytes(thrust_struct)
        pkt = self._send_message(IKARUS_MSG_THRUST, payload)
        print(f"→ Gesendet: Thrust = {m1}, {m2}, {m3}, {m4} (Paketgröße {len(pkt)} Bytes)")

    # ----------------------------------------------------------
    #       EINZELNE MOTORWERTE SCHICKEN (als Thrust Msg)
    # ----------------------------------------------------------
    def send_motor1(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_MOTOR1, payload)
        print(f"→ Gesendet: Motor1 = {value} (Paketgröße {len(pkt)} Bytes)")

    def send_motor2(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_MOTOR2, payload)
        print(f"→ Gesendet: Motor2 = {value} (Paketgröße {len(pkt)} Bytes)")

    def send_motor3(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_MOTOR3, payload)
        print(f"→ Gesendet: Motor3 = {value} (Paketgröße {len(pkt)} Bytes)")

    def send_motor4(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_MOTOR4, payload)
        print(f"→ Gesendet: Motor4 = {value} (Paketgröße {len(pkt)} Bytes)")

    # ----------------------------------------------------------
    #                PITCH / ROLL / YAW SENDEN
    # ----------------------------------------------------------
    def send_pitch(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_PITCH, payload)
        print(f"→ Gesendet: Pitch = {value} (Paketgröße {len(pkt)} Bytes)")

    def send_roll(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_ROLL, payload)
        print(f"→ Gesendet: Roll = {value} (Paketgröße {len(pkt)} Bytes)")

    def send_yaw(self, value: float):
        payload = struct.pack("<f", value)
        pkt = self._send_message(IKARUS_MSG_YAW, payload)
        print(f"→ Gesendet: Yaw = {value} (Paketgröße {len(pkt)} Bytes)")


# ============================================================
#                 Beispiel-Nutzung
# ============================================================
if __name__ == "__main__":
    com = Communication()

    print("Beispiel:")
    print("  com.send_motor_thrust(10,20,30,40)")
    print("  com.send_arming(True)")
    print("  com.send_pitch(0.3)")

    try:
        while True:
            com.send_motor_thrust(10,20,30,40)
            time.sleep(5)
    except KeyboardInterrupt:
        print("Beendet.")