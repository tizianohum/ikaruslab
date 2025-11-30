import serial
import time
import threading
import dataclasses
import struct
import ctypes

PAYLOAD_LENGTH = 100

# === Special Command IDs ===
MOTOR1_BEEP = 1
MOTOR2_BEEP = 2
MOTOR3_BEEP = 3
MOTOR4_BEEP = 4
MOTOR1_REVERSE_SPIN = 5
MOTOR2_REVERSE_SPIN = 6
MOTOR3_REVERSE_SPIN = 7
MOTOR4_REVERSE_SPIN = 8

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
class ikarus_control_external_input_t(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("roll", ctypes.c_float),
        ("pitch", ctypes.c_float),
        ("yaw", ctypes.c_float),
    ]
class ikarus_estimation_state_t(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("roll", ctypes.c_float),
        ("pitch", ctypes.c_float),
        ("yaw", ctypes.c_float),
        ("roll_dot", ctypes.c_float),
        ("pitch_dot", ctypes.c_float),
        ("yaw_dot", ctypes.c_float),
    ]
class ikarus_control_outputs_t(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("thrust1", ctypes.c_uint16),
        ("thrust2", ctypes.c_uint16),
        ("thrust3", ctypes.c_uint16),
        ("thrust4", ctypes.c_uint16),
    ]
class bmi160_acc(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
    ]

class bmi160_gyro(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
    ]

class gy271_mag(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
    ]
class ikarus_sensors_data_t(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("acc", bmi160_acc),
        ("gyr", bmi160_gyro),
        ("mag", gy271_mag),
        ("ultrasonic", ctypes.c_float),
    ]

class ikarus_log_data_t(ctypes.Structure):
    _pack_ = 1
    _fields_ = [
        ("sensors_data", ikarus_sensors_data_t),
        ("estimation_state", ikarus_estimation_state_t),
        ("control_outputs", ikarus_control_outputs_t),
        ("control_inputs", ikarus_control_external_input_t),
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

IKARUS_MAG_CALIBRATE = 50

IKARUS_SPECIAL_COMMAND = 100


# ============================================================
#                     COMMUNICATION-CLASS
# ============================================================
class Communication:
    roll = 0.0
    pitch = 0.0
    yaw = 0.0
    ultrasonic = 0.0

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
        """Thread zum Lesen kompletter UART-Pakete oder Textzeilen."""
        PAYLOAD_MAX = 100  # wie in IKARUS_MSG_MAX_PAYLOAD
        MESSAGE_LENGTH = 1 + 1 + 1 + PAYLOAD_MAX + 1  # Start + Type + Length + Payload + CRC
        TIMEOUT = 0.05

        while True:
            try:
                byte = self.ser.read(1)
                if not byte:
                    continue

                # --- Startbyte eines Binärpakets ---
                if byte[0] == 0xAA:
                    buffer = bytearray(byte)
                    start_time = time.time()

                    while len(buffer) < MESSAGE_LENGTH:
                        chunk = self.ser.read(MESSAGE_LENGTH - len(buffer))
                        if chunk:
                            buffer.extend(chunk)
                            start_time = time.time()
                        elif time.time() - start_time > TIMEOUT:
                            print("Timeout beim Lesen des Pakets")
                            break

                    if len(buffer) == MESSAGE_LENGTH:
                        # Header auslesen
                        start, msg_type, payload_length = buffer[:3]
                        if payload_length > PAYLOAD_MAX:
                            print("Ungültige Payload-Länge:", payload_length)
                            continue

                        # Payload extrahieren
                        payload = buffer[3:3 + payload_length]
                        crc_received = buffer[3 + PAYLOAD_MAX]  # CRC ist immer am Ende des fixed Payload

                        # # CRC prüfen
                        # crc_calc = (sum(buffer[:3 + PAYLOAD_MAX]) & 0xFF)
                        # if crc_calc != crc_received:
                        #     print("CRC Fehler")
                        #     continue

                        # Struktur dekodieren (nur die tatsächliche Strukturgröße)
                        log = ikarus_log_data_t.from_buffer_copy(payload[:ctypes.sizeof(ikarus_log_data_t)])

                        # Beispiel-Ausgabe
                        print(
                            f"→ ACC = ({log.sensors_data.acc.x:.3f}, "
                            f"{log.sensors_data.acc.y:.3f}, {log.sensors_data.acc.z:.3f})"
                        )
                        print(
                            f"→ Gyro = ({log.sensors_data.gyr.x:.3f}, "
                            f"{log.sensors_data.gyr.y:.3f}, {log.sensors_data.gyr.z:.3f})"
                        )
                        print(
                            f"→ Mag = ({log.sensors_data.mag.x:.3f}, "
                            f"{log.sensors_data.mag.y:.3f}, {log.sensors_data.mag.z:.3f})"
                        )
                        print(
                            f"→ Estimation roll={log.estimation_state.roll:.3f}, "
                            f"pitch={log.estimation_state.pitch:.3f}, "
                            f"yaw={log.estimation_state.yaw:.3f}"
                        )
                        print(
                            f"→ Control thrusts: "
                            f"{log.control_outputs.thrust1}, "
                            f"{log.control_outputs.thrust2}, "
                            f"{log.control_outputs.thrust3}, "
                            f"{log.control_outputs.thrust4}"
                        )

                        # interne Variablen updaten
                        self.pitch = log.estimation_state.pitch
                        self.roll = log.estimation_state.roll
                        self.yaw = log.estimation_state.yaw
                        self.ultrasonic = log.sensors_data.ultrasonic

                    continue

                # --- ASCII Text ---
                if 32 <= byte[0] <= 126 or byte in (b'\n', b'\r'):
                    line = byte + self.ser.readline()
                    print("Text:", line.decode(errors='replace').rstrip())

                # sonst: binäre Noise → ignorieren
                else:
                    pass

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

    def send_mag_calibration(self):
        payload = struct.pack("<f", 0)
        pkt = self._send_message(IKARUS_MAG_CALIBRATE, payload)
        print(f"→ Gesendet: Mag Calibrate (Paketgröße {len(pkt)} Bytes)")



    def send_special_command(self, command_id: int):
        payload = struct.pack("<I", command_id)
        pkt = self._send_message(IKARUS_SPECIAL_COMMAND, payload)
        print(f"→ Gesendet: Special Command ID = {command_id} (Paketgröße {len(pkt)} Bytes)")
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