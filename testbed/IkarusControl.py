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
        """Packt das Message-Objekt zu einem binären C-kompatiblen Bytearray."""
        header = struct.pack("<BBB", self.start, self.msg_type, self.payload_length)
        payload_padded = self.payload.ljust(PAYLOAD_LENGTH, b"\x00")
        crc_value = (sum(header + payload_padded) & 0xFF)
        self.crc = crc_value
        return header + payload_padded + struct.pack("<B", crc_value)

    @staticmethod
    def unpack(data: bytes) -> "Message":
        start, msg_type, payload_length = struct.unpack_from("<BBB", data, 0)
        payload = data[3:3 + PAYLOAD_LENGTH]
        crc = struct.unpack_from("<B", data, 3 + PAYLOAD_LENGTH)[0]
        return Message(start, msg_type, payload_length, payload[:payload_length], crc)


# === Payload-Struktur (ctypes für Byte-Kompatibilität) ===
class MotorThrust(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ('motor1', ctypes.c_float),
        ('motor2', ctypes.c_float),
        ('motor3', ctypes.c_float),
        ('motor4', ctypes.c_float),
    ]


# === Nachrichten-IDs (müssen mit deinem STM32-Code übereinstimmen) ===
IKARUS_MSG_THRUST = 1
IKARUS_MSG_ARMING = 0


# === UART Setup ===
ser = serial.Serial('/dev/tty.usbserial-A5069RR4', baudrate=9600, timeout=1)

def lese_thread():
    """Separater Thread zum Lesen von UART."""
    while True:
        try:
            line = ser.readline()
            if line:
                print("Empfangen (roh):", line)
        except serial.SerialException:
            print("UART geschlossen.")
            break
        time.sleep(0.1)

# Lese-Thread starten
threading.Thread(target=lese_thread, daemon=True).start()

print("Gib eine Nachricht ein und drücke Enter (Strg+C zum Beenden):")
print("Beispiele:")
print("  thrust 10 20 30 40")
print("  arming 1   (zum Aktivieren)")
print("  arming 0   (zum Deaktivieren)")

try:
    while True:
        nachricht = input().strip()

        # ====== THRUST COMMAND ======
        if nachricht.startswith("thrust"):
            teile = nachricht.split()
            if len(teile) == 5:
                try:
                    # Eingaben in float umwandeln
                    t1, t2, t3, t4 = map(float, teile[1:])

                    # Payload erzeugen (C-kompatibel)
                    thrust_cmd = MotorThrust(t1, t2, t3, t4)
                    payload = bytes(thrust_cmd)

                    # Message erzeugen
                    msg = Message(msg_type=IKARUS_MSG_THRUST, payload_length=len(payload), payload=payload)
                    packet = msg.pack()

                    # Senden
                    ser.write(packet)
                    print(f"→ Gesendet: Thrust = {t1}, {t2}, {t3}, {t4} (Paketgröße {len(packet)} Bytes)")
                    continue

                except ValueError:
                    print("❌ Ungültige Werte. Beispiel: thrust 10 20 30 40")
            else:
                print("❌ Falsche Anzahl an Werten. Beispiel: thrust 10 20 30 40")
                continue


        # ====== ARMING COMMAND ======
        elif nachricht.startswith("arming"):
            teile = nachricht.split()
            if len(teile) == 2 and teile[1] in ["0", "1"]:
                armed = int(teile[1])
                payload = bytes([armed])

                msg = Message(msg_type=IKARUS_MSG_ARMING, payload_length=1, payload=payload)
                packet = msg.pack()

                ser.write(packet)
                print(f"→ Gesendet: {'ARM' if armed else 'DISARM'} (Paketgröße {len(packet)} Bytes)")
                continue
            else:
                print("❌ Falscher Befehl. Beispiel: arming 1 oder arming 0")
                continue


        # ====== SONST NORMALEN TEXT SENDEN ======
        if nachricht:
            ser.write((nachricht + '\n').encode('utf-8'))

except KeyboardInterrupt:
    print("\nProgramm beendet.")
    ser.close()