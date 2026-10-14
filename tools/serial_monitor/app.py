# -*- coding: utf-8 -*-
"""
GT2001 Dual-UART Phased-Array Antenna Calibration & Register Monitor Server
Author: Antigravity Agent
"""

import os
import sys
import time
import json
import re
import threading
import math
import urllib.parse
import webbrowser
import traceback
from http.server import HTTPServer, BaseHTTPRequestHandler
from socketserver import ThreadingMixIn

# Keep mutable data outside the installation directory. Program Files is not
# writable for a standard user and WebView2 cannot initialize its user-data
# directory there.
if getattr(sys, 'frozen', False):
    _local_app_data = os.environ.get('LOCALAPPDATA') or os.path.join(
        os.path.expanduser('~'), 'AppData', 'Local'
    )
    APP_STORAGE_DIR = os.path.join(_local_app_data, 'GT2001_Monitor')
else:
    APP_STORAGE_DIR = os.path.dirname(os.path.abspath(__file__))

try:
    os.makedirs(APP_STORAGE_DIR, exist_ok=True)
except OSError:
    # Last-resort fallback for unusual locked-down user profiles.
    import tempfile
    APP_STORAGE_DIR = os.path.join(tempfile.gettempdir(), 'GT2001_Monitor')
    os.makedirs(APP_STORAGE_DIR, exist_ok=True)

LOG_FILE = os.path.join(APP_STORAGE_DIR, 'GT2001_Monitor.log')

# Guard against NoneType stdout/stderr, preserve diagnostics for windowed
# builds, and prevent Windows GBK UnicodeEncodeError.
_windowed_log_stream = None
if getattr(sys, 'stdout', None) is None or getattr(sys, 'stderr', None) is None:
    try:
        _windowed_log_stream = open(
            LOG_FILE, 'a', encoding='utf-8', errors='replace', buffering=1
        )
    except OSError:
        _windowed_log_stream = open(os.devnull, 'w', encoding='utf-8', errors='replace')

for _stream_name in ('stdout', 'stderr'):
    _stream = getattr(sys, _stream_name, None)
    if _stream is None:
        setattr(sys, _stream_name, _windowed_log_stream)
    else:
        try:
            _stream.reconfigure(encoding='utf-8', errors='replace')
        except Exception:
            pass

def _log_uncaught_exception(exc_type, exc_value, exc_traceback):
    try:
        traceback.print_exception(exc_type, exc_value, exc_traceback, file=sys.stderr)
    except Exception:
        pass

sys.excepthook = _log_uncaught_exception

try:
    import webview
    HAS_WEBVIEW = True
except ImportError:
    HAS_WEBVIEW = False

try:
    import serial
    import serial.tools.list_ports
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False

# Physical and Geometric Constants for KU-Band Phased-Array
CV = 299792458.0
DELTAX_TX = 0.0105
DELTAY_TX = 0.0105
DELTAX_RX = 0.0120
DELTAY_RX = 0.0120

beam_map_id_tx = [
    [ 9, 9, 7, 7, 5, 5,11,11],
    [ 9, 9, 7, 7, 5, 5,11,11],
    [ 8, 8, 6, 6,14,14, 1, 1],
    [ 8, 8, 6, 6,14,14, 1, 1],
    [13,13, 2, 2,12,12, 4, 4],
    [13,13, 2, 2,12,12, 4, 4],
    [10,10,15,15, 0, 0, 3, 3],
    [10,10,15,15, 0, 0, 3, 3]
]

beam_map_id_rx = [
    [ 3, 3,11,11, 7, 7,15,15],
    [ 3, 3,11,11, 7, 7,15,15],
    [13,13, 5, 5, 9, 9, 1, 1],
    [13,13, 5, 5, 9, 9, 1, 1],
    [ 2, 2,10,10, 6, 6,14,14],
    [ 2, 2,10,10, 6, 6,14,14],
    [12,12, 4, 4, 8, 8, 0, 0],
    [12,12, 4, 4, 8, 8, 0, 0]
]

beam_map_chl_H = [
    [7,6,7,6,7,6,7,6],
    [1,0,1,0,1,0,1,0],
    [0,1,0,1,0,1,0,1],
    [6,7,6,7,6,7,6,7],
    [7,6,7,6,7,6,7,6],
    [1,0,1,0,1,0,1,0],
    [0,1,0,1,0,1,0,1],
    [6,7,6,7,6,7,6,7]
]

beam_map_chl_V = [
    [5,4,5,4,5,4,5,4],
    [3,2,3,2,3,2,3,2],
    [2,3,2,3,2,3,2,3],
    [4,5,4,5,4,5,4,5],
    [5,4,5,4,5,4,5,4],
    [3,2,3,2,3,2,3,2],
    [2,3,2,3,2,3,2,3],
    [4,5,4,5,4,5,4,5]
]

def phase_calmat_beam_after(tr_dir, chip_id, ch, in_phase, pol):
    phase_t = 0.0
    if tr_dir == "TX":
        if chip_id in [10, 15, 0, 3, 8, 6, 14, 1]:
            table = {
                0: in_phase,
                1: in_phase + 270.0 + 2.0 * pol,
                2: in_phase + 2.0 * pol,
                3: in_phase + 90.0,
                4: in_phase + 270.0,
                5: in_phase + 180.0 + 2.0 * pol,
                6: in_phase + 90.0 + 2.0 * pol,
                7: in_phase + 180.0
            }
            phase_t = table.get(ch, 0.0)
        else:
            table = {
                0: in_phase + 180.0,
                1: in_phase + 90.0 + 2.0 * pol,
                2: in_phase + 180.0 + 2.0 * pol,
                3: in_phase + 270.0,
                4: in_phase + 90.0,
                5: in_phase + 2.0 * pol,
                6: in_phase + 270.0 + 2.0 * pol,
                7: in_phase
            }
            phase_t = table.get(ch, 0.0)
    else: # RX
        if chip_id in [12, 4, 8, 0, 13, 9, 5, 1]:
            table = {
                0: in_phase,
                1: in_phase + 270.0 + 2.0 * pol,
                2: in_phase + 2.0 * pol,
                3: in_phase + 90.0,
                4: in_phase + 270.0,
                5: in_phase + 180.0 + 2.0 * pol,
                6: in_phase + 90.0 + 2.0 * pol,
                7: in_phase + 180.0
            }
            phase_t = table.get(ch, 0.0)
        else:
            table = {
                0: in_phase + 180.0,
                1: in_phase + 90.0 + 2.0 * pol,
                2: in_phase + 180.0 + 2.0 * pol,
                3: in_phase + 270.0,
                4: in_phase + 90.0,
                5: in_phase + 2.0 * pol,
                6: in_phase + 270.0 + 2.0 * pol,
                7: in_phase
            }
            phase_t = table.get(ch, 0.0)
    return phase_t % 360.0

def port_bias(tr_dir, chip_id, ch):
    """Returns static port compensation bias (deg) and whether channel has +2*pol factor."""
    bias_zero = phase_calmat_beam_after(tr_dir, chip_id, ch, 0.0, 0.0)
    bias_test = phase_calmat_beam_after(tr_dir, chip_id, ch, 0.0, 45.0)
    diff = (bias_test - bias_zero) % 360.0
    has_pol = abs(diff - 90.0) < 1.0 or abs(diff - 450.0) < 1.0
    return bias_zero, has_pol

# Precompute static port compensation table for instant O(1) solver performance
PORT_BIAS_TABLE = {"TX": {}, "RX": {}}
for _d in ["TX", "RX"]:
    for _chip in range(16):
        PORT_BIAS_TABLE[_d][_chip] = {}
        for _ch in range(8):
            PORT_BIAS_TABLE[_d][_chip][_ch] = port_bias(_d, _chip, _ch)

try:
    from beam_solver import (
        solve_beam_angles_dual,
        solve_beam_angles_pos_pol,
        solve_beam_angles_neg_pol,
        extract_channel_code
    )
except ImportError:
    import sys
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from beam_solver import (
        solve_beam_angles_dual,
        solve_beam_angles_pos_pol,
        solve_beam_angles_neg_pol,
        extract_channel_code
    )

def solve_beam_angles(registers, tr_dir="TX", freq_hz=14.25e9, pol_mode=None):
    """
    反推离轴角 (pitch θ)、方位角 (yaw φ)、极化角 (pol ψ) 及阵面 X/Y 空间相位梯度。
    支持双版本算法：
    - '+2*pol': 基于当前固件修复公式 phase = in_phase + bias + 2.0 * pol
    - '-2*pol': 基于旧版固件公式 phase = in_phase + bias - 2.0 * pol
    """
    mode = pol_mode or getattr(state, "pol_algorithm_mode", "+2*pol")
    return solve_beam_angles_dual(registers, tr_dir, freq_hz, default_mode=mode)


def forward_simulation(tr_dir, pitch, yaw, freq_hz, pol):
    dx = DELTAX_TX if tr_dir == "TX" else DELTAX_RX
    dy = DELTAY_TX if tr_dir == "TX" else DELTAY_RX
    k = 2 * math.pi * freq_hz / CV
    th = math.radians(pitch)
    ph = math.radians(yaw)
    dx_rad = (-k * dx * math.cos(ph) * math.sin(th)) % (-2 * math.pi)
    dy_rad = (-k * dy * math.sin(ph) * math.sin(th)) % (-2 * math.pi)
    x_inc = math.degrees(dx_rad) % 360.0
    y_inc = math.degrees(dy_rad) % 360.0

    beam_map_id = beam_map_id_tx if tr_dir == "TX" else beam_map_id_rx
    registers = {chip: [0]*8 for chip in range(16)}

    for pol_idx, beam_map_chl in enumerate([beam_map_chl_H, beam_map_chl_V]):
        y_phase = 0.0
        for y in range(8):
            x_phase = 0.0
            for x in range(8):
                chip_id = beam_map_id[y][x]
                ch = beam_map_chl[y][x]
                phase_t = (x_phase + y_phase) % 360.0
                phase_actual = phase_calmat_beam_after(tr_dir, chip_id, ch, phase_t, pol)
                code = int(round(phase_actual / 5.625)) & 0x3F
                registers[chip_id][ch] = code
                x_phase = (x_phase + x_inc) % 360.0
            y_phase = (y_phase + y_inc) % 360.0
    return registers

PORT = 8088
if getattr(sys, 'frozen', False):
    base_candidates = [
        getattr(sys, '_MEIPASS', ''),
        os.path.join(getattr(sys, '_MEIPASS', ''), '_internal'),
        os.path.dirname(sys.executable),
        os.path.join(os.path.dirname(sys.executable), '_internal')
    ]
    STATIC_DIR = os.path.join(os.path.dirname(sys.executable), "static")
    for b in base_candidates:
        if b and os.path.isdir(os.path.join(b, "static")):
            STATIC_DIR = os.path.join(b, "static")
            break
else:
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))
    STATIC_DIR = os.path.join(BASE_DIR, "static")

# Persistent Config File for Custom Presets (A, B, C, D)
PRESETS_FILE = os.path.join(APP_STORAGE_DIR, "config_presets.json")

DEFAULT_CUSTOM_PRESETS = {
    "custom1": {"reg1": "48", "reg2": "16", "reg3": "32", "reg4": "32"},
    "custom2": {"reg1": "48", "reg2": "24", "reg3": "36", "reg4": "32"},
    "custom3": {"reg1": "20", "reg2": "28", "reg3": "36", "reg4": "44"},
    "custom4": {"reg1": "16", "reg2": "20", "reg3": "24", "reg4": "28"},
    "active_preset_key": "group3"
}

def load_custom_presets():
    if os.path.exists(PRESETS_FILE):
        try:
            with open(PRESETS_FILE, 'r', encoding='utf-8') as f:
                data = json.load(f)
                res = {k: dict(v) for k, v in DEFAULT_CUSTOM_PRESETS.items() if k.startswith("custom")}
                for k in ["custom1", "custom2", "custom3", "custom4"]:
                    if k in data and isinstance(data[k], dict):
                        res[k] = {
                            "reg1": str(data[k].get("reg1", res[k]["reg1"])),
                            "reg2": str(data[k].get("reg2", res[k]["reg2"])),
                            "reg3": str(data[k].get("reg3", res[k]["reg3"])),
                            "reg4": str(data[k].get("reg4", res[k]["reg4"])),
                        }
                res["active_preset_key"] = str(data.get("active_preset_key", "group3"))
                return res
        except Exception as e:
            print(f"[WARN] 读取预设配置文件 config_presets.json 异常: {e}")
    return {k: dict(v) if isinstance(v, dict) else v for k, v in DEFAULT_CUSTOM_PRESETS.items()}

def save_custom_presets(data):
    current = load_custom_presets()
    for k in ["custom1", "custom2", "custom3", "custom4"]:
        if k in data and isinstance(data[k], dict):
            current[k] = {
                "reg1": str(data[k].get("reg1", current[k]["reg1"])),
                "reg2": str(data[k].get("reg2", current[k]["reg2"])),
                "reg3": str(data[k].get("reg3", current[k]["reg3"])),
                "reg4": str(data[k].get("reg4", current[k]["reg4"])),
            }
    if "active_preset_key" in data:
        current["active_preset_key"] = str(data["active_preset_key"])
    
    try:
        tmp_file = PRESETS_FILE + ".tmp"
        with open(tmp_file, 'w', encoding='utf-8') as f:
            json.dump(current, f, indent=2, ensure_ascii=False)
        if os.path.exists(PRESETS_FILE):
            os.replace(tmp_file, PRESETS_FILE)
        else:
            os.rename(tmp_file, PRESETS_FILE)
        print(f"[CONFIG] 自定义预设已成功持久化写入本地磁盘文件: {PRESETS_FILE}")
    except Exception as e:
        print(f"[ERROR] 写入预设配置文件失败: {e}")
    return current

# Antenna physical chip layout from Image 2
TX_CHIP_GRID = [
    [3, 0, 15, 10],
    [4, 12, 2, 13],
    [1, 14, 6, 8],
    [11, 5, 7, 9]
]

RX_CHIP_GRID = [
    [0, 8, 4, 12],
    [14, 6, 10, 2],
    [1, 9, 5, 13],
    [15, 7, 11, 3]
]

# Channel order inside each chip card from Image 1 & 3:
# Rows 0 & 2 (1st & 3rd row, yellow): left [7, 5, 3, 1], right [6, 4, 2, 0]
# Rows 1 & 3 (2nd & 4th row, green):  left [0, 2, 4, 6], right [1, 3, 5, 7]

class DeviceState:
    def __init__(self):
        self.lock = threading.RLock()
        
        # Dual UART connection handles
        self.uart0_conn = None
        self.uart0_port = ""
        self.uart0_baud = 115200
        self.uart0_status = "disconnected" # "connected" or "disconnected"
        
        self.uart3_conn = None
        self.uart3_port = ""
        self.uart3_baud = 921600
        self.uart3_status = "disconnected"
        
        self.simulation_mode = False
        
        # Logs buffer
        self.uart0_logs = []
        self.uart3_logs = []
        
        # Register data: 16 chips each for TX and RX, 8 channels each
        # chip_id: 0..15 -> channels: 0..7 -> {amp: int|None, phase: int|None, phase_deg: float|None, has_readback: bool, error: bool, updated_at: float}
        self.registers = {
            "TX": {chip: {ch: {"amp": None, "phase": None, "phase_deg": None, "has_readback": False, "is_open": False, "error": False, "updated_at": 0} for ch in range(8)} for chip in range(16)},
            "RX": {chip: {ch: {"amp": None, "phase": None, "phase_deg": None, "has_readback": False, "is_open": False, "error": False, "updated_at": 0} for ch in range(8)} for chip in range(16)}
        }
        
        self.last_solver_time = {"TX": 0.0, "RX": 0.0}
        self.pol_algorithm_mode = "+2*pol"  # "+2*pol" (当前固件修复) 或 "-2*pol" (旧版算法)

        self.system_status = {
            "pointing_mode": "MANUAL", # "MANUAL" or "AUTO"
            "array_power": "UNKNOWN",  # "ON", "OFF", or "UNKNOWN"
            "tx_channels": "UNKNOWN",  # "ALL_ON", "ALL_OFF", or "UNKNOWN"
            "rx_channels": "UNKNOWN",  # "ALL_ON", "ALL_OFF", or "UNKNOWN"
            "pol_algorithm_mode": self.pol_algorithm_mode,
            "last_beam_update": None,
            "last_reg_reply": None,
            "beam_inverse": {
                "TX": None,
                "RX": None
            }
        }

state = DeviceState()

# CRC-16/CCITT-FALSE Calculation (Polynomial 0x1021, Init 0xFFFF)
def calc_crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8) & 0xFFFF
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

# NMEA Checksum Calculation
def calc_nmea_checksum(body_str):
    body = body_str.lstrip('$').split('*')[0]
    c = 0
    for ch in body:
        c ^= ord(ch)
    return f"{c:02X}"

# Regex patterns
beam_update_pattern = re.compile(
    r"BEAM_UPDATE,(TX|RX),PITCH=([0-9.-]+),YAW=([0-9.-]+),POL=([0-9.-]+),FREQ_MHZ=([0-9.-]+),AMP_CMD=(\d+)"
)
pts_pattern = re.compile(r"PTS:\s*切换为(自动|手动)对星模式")

def dispatch_parsed_line(line_str, hex_str=None, source="uart3"):
    now = time.time()
    with state.lock:
        if not hex_str and line_str:
            hex_str = line_str.encode('utf-8', errors='replace').hex(' ').upper()

        log_entry = {
            "time": time.strftime("%H:%M:%S"),
            "text": line_str,
            "hex": hex_str or ""
        }
        if source == "uart0":
            state.uart0_logs.append(log_entry)
            if len(state.uart0_logs) > 400:
                state.uart0_logs.pop(0)
        else:
            state.uart3_logs.append(log_entry)
            if len(state.uart3_logs) > 400:
                state.uart3_logs.pop(0)

        # 1. Parse BEAM_REG AMP or PHASE strictly
        # Format from beam.c:
        # BEAM_REG,TX,ID=03,AMP=63,63,63,63,63,63,63,63
        # BEAM_REG,TX,ID=03,PHASE=30,17,14,01,33,46,17,30
        m_reg = re.search(r"BEAM_REG\s*,\s*(TX|RX)\s*,\s*ID\s*=\s*(\d+)\s*,\s*(AMP|PHASE)\s*=\s*([0-9\s,]+)", line_str)
        if m_reg:
            direction = m_reg.group(1).upper()
            chip_id = int(m_reg.group(2))
            reg_type = m_reg.group(3).upper()
            val_tokens = [v.strip() for v in m_reg.group(4).split(',') if v.strip().isdigit()]
            if chip_id < 16 and len(val_tokens) == 8:
                for ch in range(8):
                    val = int(val_tokens[ch])
                    reg_entry = state.registers[direction][chip_id][ch]
                    if reg_type == "AMP":
                        reg_entry["amp"] = val
                        if 0 < val < 255:
                            reg_entry["is_open"] = True
                        elif val == 0:
                            reg_entry["is_open"] = False
                    else: # PHASE
                        reg_entry["phase"] = val
                        if val == 255:
                            reg_entry["phase_deg"] = None
                        else:
                            reg_entry["phase_deg"] = round(val * 5.625, 3)
                    
                    # 校验错误状态：当且仅当幅值或相位回读为 255 时标记为超时错误
                    p_val = reg_entry["phase"]
                    a_val = reg_entry["amp"]
                    reg_entry["error"] = (p_val == 255 or a_val == 255)
                    reg_entry["has_readback"] = True
                    reg_entry["updated_at"] = now

                # Perform beam angle inverse estimation when phase registers are updated
                if reg_type == "PHASE":
                    now_t = time.time()
                    # Trigger solver immediately on chip 15 (full sweep ready) or debounced at >=40ms
                    if chip_id == 15 or (now_t - state.last_solver_time.get(direction, 0) >= 0.04):
                        state.last_solver_time[direction] = now_t
                        f_hz = 14.25e9 if direction == "TX" else 12.25e9
                        if state.system_status.get("last_beam_update") and state.system_status["last_beam_update"].get("dir") == direction:
                            f_hz = state.system_status["last_beam_update"].get("freq_mhz", 14250.0 if direction == "TX" else 12250.0) * 1e6
                        state.system_status["beam_inverse"][direction] = solve_beam_angles(state.registers[direction], direction, f_hz)
            return

        # 1.05 Parse SINGLE_CH_SET (e.g. SINGLE_CH_SET,TX,ID=03,CH=2,AMP=63,PHASE=15)
        m_single_set = re.search(r"SINGLE_CH_SET\s*,\s*(TX|RX)\s*,\s*ID\s*=\s*(\d+)\s*,\s*CH\s*=\s*(\d+)\s*,\s*AMP\s*=\s*(\d+)\s*,\s*PHASE\s*=\s*(\d+)", line_str)
        if m_single_set:
            direction = m_single_set.group(1).upper()
            chip_id = int(m_single_set.group(2))
            ch = int(m_single_set.group(3))
            amp = int(m_single_set.group(4))
            phase = int(m_single_set.group(5))
            if chip_id < 16 and ch < 8:
                reg_entry = state.registers[direction][chip_id][ch]
                reg_entry["amp"] = amp
                reg_entry["phase"] = phase
                reg_entry["phase_deg"] = round(phase * 5.625, 3) if phase != 255 else None
                reg_entry["error"] = (amp == 255 or phase == 255)
                reg_entry["is_open"] = (0 < amp < 255)
                reg_entry["has_readback"] = True
                reg_entry["updated_at"] = now
            return

        # 1.1 Complete Phase Dump Notification
        if "REGISTER DUMP" in line_str and "COMPLETE" in line_str:
            m_comp = re.search(r"REGISTER DUMP\s*(TX|RX)\s*COMPLETE", line_str)
            if m_comp:
                direction = m_comp.group(1).upper()
                f_hz = 14.25e9 if direction == "TX" else 12.25e9
                if state.system_status.get("last_beam_update") and state.system_status["last_beam_update"].get("dir") == direction:
                    f_hz = state.system_status["last_beam_update"].get("freq_mhz", 14250.0 if direction == "TX" else 12250.0) * 1e6
                state.system_status["beam_inverse"][direction] = solve_beam_angles(state.registers[direction], direction, f_hz)

        # 2. Check REG / BIP reply (e.g. $xxREG,32,32,32,32,1,OK*44, $xxREG,5,48,16,32,32,0,OK*72 or $xxBIP,1,OK*56)
        m_reg_reply = re.search(r"(\$?[a-zA-Z0-9]{2}(?:REG|BIP),[^\r\n*]+)", line_str)
        if m_reg_reply:
            state.system_status["last_reg_reply"] = {
                "text": m_reg_reply.group(1),
                "time": time.strftime("%H:%M:%S")
            }

        # 3. Check BEAM_UPDATE
        m_update = beam_update_pattern.search(line_str)
        if m_update:
            direction = m_update.group(1).upper()
            state.system_status["last_beam_update"] = {
                "dir": direction,
                "pitch": float(m_update.group(2)),
                "yaw": float(m_update.group(3)),
                "pol": float(m_update.group(4)),
                "freq_mhz": float(m_update.group(5)),
                "amp_cmd": int(m_update.group(6)),
                "time": time.strftime("%H:%M:%S")
            }
            # 波束更新时，默认该阵面所有通道都是打开的，回读后直接全亮
            for cid in range(16):
                for ch in range(8):
                    state.registers[direction][cid][ch]["is_open"] = True
            return

        # 4. Check PTS mode
        m_pts = pts_pattern.search(line_str)
        if m_pts:
            state.system_status["pointing_mode"] = "AUTO" if m_pts.group(1) == "自动" else "MANUAL"
        elif "PTS,1" in line_str:
            state.system_status["pointing_mode"] = "AUTO"
        elif "PTS,0" in line_str:
            state.system_status["pointing_mode"] = "MANUAL"

        # 5. Check Single Channel Status (CH_STATUS,TX,ID=03,CH=2,OPEN=1,MASK=0x04)
        m_ch_stat = re.search(r"CH_STATUS\s*,\s*(TX|RX)\s*,\s*ID\s*=\s*(\d+)\s*,\s*CH\s*=\s*(\d+)\s*,\s*OPEN\s*=\s*(\d+)", line_str)
        if m_ch_stat:
            direction = m_ch_stat.group(1).upper()
            chip_id = int(m_ch_stat.group(2))
            ch = int(m_ch_stat.group(3))
            is_open = (int(m_ch_stat.group(4)) == 1)
            if chip_id < 16 and ch < 8:
                state.registers[direction][chip_id][ch]["is_open"] = is_open
                state.registers[direction][chip_id][ch]["updated_at"] = now
            return

        # 6. Check Array Power status
        if ("阵面上电" in line_str) or ("POWER_ON" in line_str) or ("onf_flg=0xFF" in line_str):
            state.system_status["array_power"] = "ON"
        elif ("阵面下电" in line_str) or ("POWER_OFF" in line_str) or ("onf_flg=0x00" in line_str) or ("rst" in line_str):
            state.system_status["array_power"] = "OFF"
            for d in ["TX", "RX"]:
                for cid in range(16):
                    for ch in range(8):
                        state.registers[d][cid][ch]["is_open"] = False
        elif "xxPWR," in line_str:
            parts = line_str.split(",")
            if len(parts) >= 3:
                state.system_status["array_power"] = "ON" if parts[2].startswith("1") else "OFF"

        # 7. Check Channel switch status (全开/全关)
        if ("发射通道全开" in line_str) or ("OPE_TX_ALL_ON" in line_str) or ("rxtx_sft=0xFF" in line_str):
            state.system_status["tx_channels"] = "ALL_ON"
            for cid in range(16):
                for ch in range(8):
                    state.registers["TX"][cid][ch]["is_open"] = True
        elif ("发射通道全关" in line_str) or ("OPE_TX_ALL_OFF" in line_str) or ("rxtx_sft=0x1F" in line_str):
            state.system_status["tx_channels"] = "ALL_OFF"
            for cid in range(16):
                for ch in range(8):
                    state.registers["TX"][cid][ch]["is_open"] = False

        if ("接收通道全开" in line_str) or ("OPE_RX_ALL_ON" in line_str) or ("rxtx_sft=0x00" in line_str):
            state.system_status["rx_channels"] = "ALL_ON"
            for cid in range(16):
                for ch in range(8):
                    state.registers["RX"][cid][ch]["is_open"] = True
        elif ("接收通道全关" in line_str) or ("OPE_RX_ALL_OFF" in line_str) or ("rxtx_sft=0x10" in line_str):
            state.system_status["rx_channels"] = "ALL_OFF"
            for cid in range(16):
                for ch in range(8):
                    state.registers["RX"][cid][ch]["is_open"] = False

        if ("OPE_RXTX_ALL_ON" in line_str) or ("rxtx_sft=0x0F" in line_str):
            state.system_status["tx_channels"] = "ALL_ON"
            state.system_status["rx_channels"] = "ALL_ON"
            for d in ["TX", "RX"]:
                for cid in range(16):
                    for ch in range(8):
                        state.registers[d][cid][ch]["is_open"] = True
        elif ("OPE_RXTX_ALL_OFF" in line_str) or ("rxtx_sft=0xF0" in line_str):
            state.system_status["tx_channels"] = "ALL_OFF"
            state.system_status["rx_channels"] = "ALL_OFF"
            for d in ["TX", "RX"]:
                for cid in range(16):
                    for ch in range(8):
                        state.registers[d][cid][ch]["is_open"] = False

def process_uart3_line(line_str, hex_str=None):
    dispatch_parsed_line(line_str, hex_str, source="uart3")

def process_uart0_line(line_str, hex_str=None):
    dispatch_parsed_line(line_str, hex_str, source="uart0")

def process_incoming_stream(rx_buf: bytearray, source: str) -> bytearray:
    """
    高吞吐混合流式分帧器 (High-Throughput Hybrid Stream Parser)
    自动解构 ACU 二进制帧 (0xAA 0xAA 0x55 0x55)、ATCL 二进制帧 (0xAA 0x99)
    以及 NMEA / ASCII 换行文本行，彻底杜绝无换行符二进制数据导致的超时死等。
    """
    while len(rx_buf) > 0:
        # 1. 识别 ACU 二进制遥测/指令帧 (0xAA 0xAA 0x55 0x55)
        if rx_buf.startswith(b'\xaa\xaa\x55\x55'):
            if len(rx_buf) < 6:
                break
            all_len = rx_buf[4] | (rx_buf[5] << 8)
            if all_len < 8 or all_len > 2048:
                rx_buf = rx_buf[1:]
                continue
            if len(rx_buf) < all_len:
                break
            frame_bytes = bytes(rx_buf[:all_len])
            rx_buf = rx_buf[all_len:]
            tarcmd = frame_bytes[7] if all_len > 7 else 0
            cmd_name = {
                0xAA: "DATA/遥测",
                0xBB: "STATUS/状态",
                0xCC: "CMD/控制"
            }.get(tarcmd, f"0x{tarcmd:02X}")
            line_str = f"[ACU BIN] 遥测回传 tarcmd={cmd_name}, len={all_len}"
            hex_str = frame_bytes.hex(' ').upper()
            dispatch_parsed_line(line_str, hex_str, source=source)
            continue

        # 2. 识别 ATCL 二进制指令帧 (0xAA 0x99，如 Flash 标校响应)
        if rx_buf.startswith(b'\xaa\x99'):
            if len(rx_buf) < 4:
                break
            all_len = (rx_buf[2] << 8) | rx_buf[3]
            if all_len < 6 or all_len > 2048:
                rx_buf = rx_buf[1:]
                continue
            if len(rx_buf) < all_len:
                break
            frame_bytes = bytes(rx_buf[:all_len])
            rx_buf = rx_buf[all_len:]
            fcode = frame_bytes[4] if all_len > 4 else 0
            fcode_desc = {
                0x01: "芯片复位响应 (RESET)",
                0x02: "单通道幅相配置响应 (SINGLE_CH)",
                0x03: "阵面上/下电响应 (POWER)",
                0x05: "通道全开/全关响应 (CHANNEL_ALL)",
                0x08: "标校误差表清零/下发响应 (ERT)"
            }.get(fcode, f"FCODE=0x{fcode:02X}")
            line_str = f"[ATCL BIN] {fcode_desc}, len={all_len}"
            hex_str = frame_bytes.hex(' ').upper()
            dispatch_parsed_line(line_str, hex_str, source=source)
            continue

        # 3. 识别 ASCII 文本行（以 \n 或 \r 结尾）
        idx_n = rx_buf.find(b'\n')
        idx_r = rx_buf.find(b'\r')
        delim_idx = -1
        if idx_n != -1 and idx_r != -1:
            delim_idx = min(idx_n, idx_r)
        elif idx_n != -1:
            delim_idx = idx_n
        elif idx_r != -1:
            delim_idx = idx_r

        if delim_idx != -1:
            line_bytes = bytes(rx_buf[:delim_idx])
            delim_end = delim_idx + 1
            if delim_idx == idx_r and delim_end < len(rx_buf) and rx_buf[delim_end] == ord(b'\n'):
                delim_end += 1
            rx_buf = rx_buf[delim_end:]

            if line_bytes:
                try:
                    decoded = line_bytes.decode('utf-8').strip()
                except UnicodeDecodeError:
                    try:
                        decoded = line_bytes.decode('gbk').strip()
                    except UnicodeDecodeError:
                        decoded = line_bytes.decode('utf-8', errors='ignore').strip()
                hex_dump = line_bytes.hex(' ').upper()
                if decoded or hex_dump:
                    dispatch_parsed_line(decoded, hex_dump, source=source)
            continue

        # 4. 环形防溢出防护：若缓冲区过大且头部无法识别，滑动至下一个潜在合法帧头
        if len(rx_buf) > 2048:
            candidates = []
            for pat in (b'\xaa\xaa\x55\x55', b'\xaa\x99', b'$', b'\n', b'\r'):
                p = rx_buf.find(pat, 1)
                if p != -1:
                    candidates.append(p)
            if candidates:
                rx_buf = rx_buf[min(candidates):]
            else:
                rx_buf = rx_buf[-512:]
        break

    return rx_buf

# UART Background Readers (使用 in_waiting 批量块读取，极大降低系统调用开销)
def uart0_worker():
    rx_buf = bytearray()
    while True:
        ser = None
        with state.lock:
            if state.uart0_status == "connected":
                ser = state.uart0_conn
        if ser and ser.is_open:
            try:
                waiting = ser.in_waiting if hasattr(ser, 'in_waiting') else 0
                n_read = max(1, min(waiting, 4096)) if waiting > 0 else 1
                chunk = ser.read(n_read)
                if chunk:
                    rx_buf.extend(chunk)
                    rx_buf = process_incoming_stream(rx_buf, source="uart0")
            except Exception:
                with state.lock:
                    state.uart0_status = "disconnected"
                time.sleep(0.5)
        else:
            time.sleep(0.05)

def uart3_worker():
    rx_buf = bytearray()
    while True:
        ser = None
        with state.lock:
            if state.uart3_status == "connected":
                ser = state.uart3_conn
        if ser and ser.is_open:
            try:
                waiting = ser.in_waiting if hasattr(ser, 'in_waiting') else 0
                n_read = max(1, min(waiting, 4096)) if waiting > 0 else 1
                chunk = ser.read(n_read)
                if chunk:
                    rx_buf.extend(chunk)
                    rx_buf = process_incoming_stream(rx_buf, source="uart3")
            except Exception:
                with state.lock:
                    state.uart3_status = "disconnected"
                time.sleep(0.5)
        else:
            time.sleep(0.05)

# Sample phase codes matching Image 1 for Chip 3:
SAMPLE_PHASE_MAP = {
    3:  [17, 46, 17, 14, 1, 30, 33, 30],
    0:  [54, 19, 54, 51, 38, 3, 6, 3],
    15: [27, 56, 27, 24, 11, 40, 43, 40],
    10: [0, 29, 0, 61, 48, 13, 16, 13],
    4:  [30, 33, 30, 1, 14, 17, 46, 17],
    12: [3, 6, 3, 38, 51, 54, 19, 54],
    2:  [40, 43, 40, 11, 24, 27, 56, 27],
    13: [13, 16, 13, 48, 61, 0, 29, 0],
    1:  [17, 46, 17, 14, 1, 30, 33, 30],
    14: [54, 19, 54, 51, 38, 3, 6, 3],
    6:  [27, 56, 27, 24, 11, 40, 43, 40],
    8:  [0, 29, 0, 61, 48, 13, 16, 13],
    11: [30, 33, 30, 1, 14, 17, 46, 17],
    5:  [3, 6, 3, 38, 51, 54, 19, 54],
    7:  [40, 43, 40, 11, 24, 27, 56, 27],
    9:  [13, 16, 13, 48, 61, 0, 29, 0]
}

def emit_simulated_chip_dump(tr_dir="TX"):
    """Emits a complete 16-chip BEAM_REG readback stream matching firmware format."""
    now = time.time()
    process_uart3_line(f"========== BEAM AMP/PHASE REGISTER DUMP {tr_dir} START ==========")
    process_uart3_line("FORMAT: physical_id, AMP[ch0..ch7], PHASE[ch0..ch7]; raw decimal code=0..63, phase_step=5.625deg")

    beam_up = state.system_status.get("last_beam_update")
    if beam_up and beam_up.get("dir") == tr_dir:
        pitch = beam_up.get("pitch", 0.0)
        yaw = beam_up.get("yaw", 0.0)
        pol = beam_up.get("pol", 0.0)
        freq = beam_up.get("freq_mhz", 14250.0 if tr_dir == "TX" else 12250.0) * 1e6
        sim_regs = forward_simulation(tr_dir, pitch, yaw, freq, pol)
    else:
        sim_regs = {}
        for chip_id in range(16):
            phases = SAMPLE_PHASE_MAP.get(chip_id, [0]*8)
            if tr_dir == "RX":
                phases = [(p + 8) % 64 for p in phases]
            sim_regs[chip_id] = phases

    for chip_id in range(16):
        phases = sim_regs.get(chip_id, [0]*8)
        amp_str = ",".join(["63"] * 8)
        phase_str = ",".join([f"{p:02d}" for p in phases])
        process_uart3_line(f"BEAM_REG,{tr_dir},ID={chip_id:02d},AMP={amp_str}")
        process_uart3_line(f"BEAM_REG,{tr_dir},ID={chip_id:02d},PHASE={phase_str}")
    process_uart3_line(f"========== BEAM AMP/PHASE REGISTER DUMP {tr_dir} COMPLETE: chips=16 channels=128 read_errors=0 ==========")

# Simulation Worker (only emits data if simulation_mode is active)
def simulation_worker():
    while True:
        sim_active = False
        with state.lock:
            sim_active = state.simulation_mode
        if sim_active:
            time.sleep(5.0)
        else:
            time.sleep(0.5)

# Threading HTTP Server
class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    daemon_threads = True
    allow_reuse_address = True

class RequestHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        # Suppress noisy HTTP logs
        return

    def send_json(self, data, code=200):
        body = json.dumps(data, ensure_ascii=False).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', str(len(body)))
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_GET(self):
        url_path = self.path.split('?')[0]
        if url_path == "/api/ports":
            ports = []
            if HAS_SERIAL:
                for p in serial.tools.list_ports.comports():
                    ports.append({
                        "device": p.device,
                        "description": p.description or p.device
                    })
            else:
                ports = [{"device": "COM1 (Sim)", "description": "Serial lib missing"}]
            self.send_json({"success": True, "ports": ports})
            return

        elif url_path == "/api/status":
            with state.lock:
                data = {
                    "uart0": {
                        "port": state.uart0_port,
                        "baud": state.uart0_baud,
                        "status": state.uart0_status
                    },
                    "uart3": {
                        "port": state.uart3_port,
                        "baud": state.uart3_baud,
                        "status": state.uart3_status
                    },
                    "simulation_mode": state.simulation_mode,
                    "system": state.system_status,
                    "registers": state.registers,
                    "uart0_logs": state.uart0_logs[-50:],
                    "uart3_logs": state.uart3_logs[-50:],
                    "grids": {
                        "TX": TX_CHIP_GRID,
                        "RX": RX_CHIP_GRID
                    }
                }
            self.send_json(data)
            return

        elif url_path == "/api/presets":
            data = load_custom_presets()
            self.send_json({"success": True, "presets": data})
            return

        # Static files serving
        file_path = ""
        if url_path == "/" or url_path == "/index.html":
            file_path = os.path.join(STATIC_DIR, "index.html")
            content_type = "text/html; charset=utf-8"
        elif url_path.startswith("/"):
            rel_name = url_path.lstrip("/")
            file_path = os.path.join(STATIC_DIR, rel_name)
            if file_path.endswith(".css"):
                content_type = "text/css; charset=utf-8"
            elif file_path.endswith(".js"):
                content_type = "application/javascript; charset=utf-8"
            elif file_path.endswith(".png"):
                content_type = "image/png"
            elif file_path.endswith(".ico"):
                content_type = "image/x-icon"
            else:
                content_type = "text/plain; charset=utf-8"

        if file_path and os.path.isfile(file_path):
            with open(file_path, "rb") as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(content)))
            self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
            self.send_header("Pragma", "no-cache")
            self.send_header("Expires", "0")
            self.end_headers()
            self.wfile.write(content)
        else:
            self.send_error(404, "File Not Found")

    def do_POST(self):
        url_path = self.path.split('?')[0]
        length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(length) if length > 0 else b'{}'
        try:
            req = json.loads(post_data.decode('utf-8'))
        except Exception:
            req = {}

        if url_path == "/api/connect":
            target = req.get("target") # "uart0" or "uart3"
            port_name = req.get("port")
            default_baud = 921600 if target == "uart3" else 115200
            baud_rate = int(req.get("baud", default_baud))

            if not port_name:
                self.send_json({"success": False, "msg": "未指定端口名称"}, 400)
                return

            if not HAS_SERIAL:
                self.send_json({"success": False, "msg": "未检测到 pyserial 库"}, 500)
                return

            try:
                ser = serial.Serial(port_name, baud_rate, timeout=0.02)
                with state.lock:
                    if target == "uart0":
                        if state.uart0_conn and state.uart0_conn.is_open:
                            state.uart0_conn.close()
                        state.uart0_conn = ser
                        state.uart0_port = port_name
                        state.uart0_baud = baud_rate
                        state.uart0_status = "connected"
                    elif target == "uart3":
                        if state.uart3_conn and state.uart3_conn.is_open:
                            state.uart3_conn.close()
                        state.uart3_conn = ser
                        state.uart3_port = port_name
                        state.uart3_baud = baud_rate
                        state.uart3_status = "connected"
                self.send_json({"success": True, "msg": f"{target} 连接成功: {port_name} ({baud_rate}bps)"})
            except Exception as e:
                self.send_json({"success": False, "msg": f"连接失败: {str(e)}"}, 500)
            return

        elif url_path == "/api/disconnect":
            target = req.get("target")
            with state.lock:
                if target == "uart0" and state.uart0_conn:
                    try:
                        state.uart0_conn.close()
                    except Exception:
                        pass
                    state.uart0_status = "disconnected"
                elif target == "uart3" and state.uart3_conn:
                    try:
                        state.uart3_conn.close()
                    except Exception:
                        pass
                    state.uart3_status = "disconnected"
            self.send_json({"success": True, "msg": f"{target} 已断开"})
            return

        elif url_path == "/api/toggle_sim":
            with state.lock:
                state.simulation_mode = not state.simulation_mode
                sim_state = state.simulation_mode
                if sim_state:
                    # Populate simulated sample data
                    for chip_id, phases in SAMPLE_PHASE_MAP.items():
                        for ch in range(8):
                            p = phases[ch]
                            deg = round(p * 5.625, 3)
                            state.registers["TX"][chip_id][ch] = {
                                "amp": 63, "phase": p, "phase_deg": deg,
                                "has_readback": True, "is_open": True, "error": False, "updated_at": time.time()
                            }
                            rx_p = (p + 8) % 64
                            state.registers["RX"][chip_id][ch] = {
                                "amp": 63, "phase": rx_p, "phase_deg": round(rx_p * 5.625, 3),
                                "has_readback": True, "is_open": True, "error": False, "updated_at": time.time()
                            }
                else:
                    # Reset back to unread state so real hardware readback is strict
                    for d in ["TX", "RX"]:
                        for chip_id in range(16):
                            for ch in range(8):
                                state.registers[d][chip_id][ch] = {
                                    "amp": None, "phase": None, "phase_deg": None,
                                    "has_readback": False, "is_open": False, "error": False, "updated_at": 0
                                }
            self.send_json({"success": True, "simulation_mode": sim_state})
        elif url_path == "/api/set_pol_mode":
            mode = req.get("mode", "+2*pol")
            if mode in ["+2*pol", "-2*pol"]:
                state.pol_algorithm_mode = mode
                state.system_status["pol_algorithm_mode"] = mode
                with state.lock:
                    for direction in ["TX", "RX"]:
                        f_hz = 14.25e9 if direction == "TX" else 12.25e9
                        if state.system_status.get("last_beam_update") and state.system_status["last_beam_update"].get("dir") == direction:
                            f_hz = state.system_status["last_beam_update"].get("freq_mhz", 14250.0 if direction == "TX" else 12250.0) * 1e6
                        state.system_status["beam_inverse"][direction] = solve_beam_angles(state.registers[direction], direction, f_hz, pol_mode=state.pol_algorithm_mode)
            self.send_json({"success": True, "mode": state.pol_algorithm_mode, "beam_inverse": state.system_status["beam_inverse"]})
            return

        elif url_path == "/api/clear_logs":
            target = req.get("target", "all") # "uart0", "uart3", or "all"
            with state.lock:
                if target in ("uart0", "all"):
                    state.uart0_logs.clear()
                if target in ("uart3", "all"):
                    state.uart3_logs.clear()
            self.send_json({"success": True, "msg": f"日志已清空: {target}"})
        elif url_path == "/api/presets":
            saved = save_custom_presets(req)
            self.send_json({"success": True, "presets": saved})
            return

        elif url_path == "/api/send":
            cmd_type = req.get("type") # "POWER", "PTS", "ATS", "REG", "RAW", "HEX", "DUMP"
            raw_frame = ""
            binary_data = None
            custom_log = ""

            if cmd_type == "POWER":
                target_dir = int(req.get("dir", 0x0F)) & 0xFF
                action = 1 if int(req.get("action", 1)) == 1 else 0
                onf_flg = 0xFF if action == 1 else 0x00
                
                # Construct ATCL binary frame (0xAA 0x99 ...)
                payload_body = bytes([0xAA, 0x99, 0x00, 0x0A, 0x03, target_dir, 0x00, onf_flg])
                crc_val = calc_crc16_ccitt_false(payload_body)
                binary_data = payload_body + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                
                dir_desc = "双阵面(ALL)" if target_dir == 0x0F else ("发射阵面(TX)" if target_dir == 0xFF else "接收阵面(RX)")
                act_desc = "上电 (ON)" if action == 1 else "下电 (OFF)"
                
                body = f"xxPWR,{target_dir},{action}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                custom_log = f">> [阵面{act_desc}] {dir_desc} [BIN: {binary_data.hex(' ').upper()}]"

            elif cmd_type == "CHANNEL_ALL":
                target = str(req.get("target", "TX")).upper() # "TX" or "RX"
                action = 1 if int(req.get("action", 1)) == 1 else 0 # 1: 全开, 0: 全关
                
                # Protocol codes matching beam.h & TR_channel_update_notes.md:
                # OPE_TX_ALL_ON = 0xFF, OPE_TX_ALL_OFF = 0x1F
                # OPE_RX_ALL_ON = 0x00, OPE_RX_ALL_OFF = 0x10
                if target == "TX":
                    rxtx_sft = 0xFF if action == 1 else 0x1F
                else:
                    rxtx_sft = 0x00 if action == 1 else 0x10

                # ATCL binary frame for FCODE 0x05 (Whole-array channel switch)
                # Frame: AA 99 00 0A 05 00 00 [rxtx_sft] [CRC_H] [CRC_L]
                payload_body = bytes([0xAA, 0x99, 0x00, 0x0A, 0x05, 0x00, 0x00, rxtx_sft])
                crc_val = calc_crc16_ccitt_false(payload_body)
                binary_data = payload_body + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                
                target_desc = "发射阵面(TX)" if target == "TX" else "接收阵面(RX)"
                act_desc = "全通道开启 (ALL ON)" if action == 1 else "全通道关闭 (ALL OFF)"
                body = f"xxOPE,{target},{'ON' if action == 1 else 'OFF'},0x{rxtx_sft:02X}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                custom_log = f">> [{target_desc}] {act_desc} [0x{rxtx_sft:02X}] [BIN: {binary_data.hex(' ').upper()}]"

            elif cmd_type == "SINGLE_CH":
                target = str(req.get("target", "TX")).upper() # "TX" or "RX"
                chip_id = int(req.get("chip_id", 0)) & 0x0F
                ch = int(req.get("ch", 0)) & 0x07
                action = str(req.get("action", "SET")).upper() # "OPEN", "CLOSE", "SET"
                amp = int(req.get("amp", 63)) & 0x3F
                phase = int(req.get("phase", 0)) & 0x3F
                pole = int(req.get("pole", 0)) & 0x01
                
                tr_dir = 0xFF if target == "TX" else 0x00
                ch_num = (1 << ch) & 0xFF
                fcode_byte = 0x01 if action == "OPEN" else (0x00 if action == "CLOSE" else 0x02)
                
                # ATCL 16-byte frame for FCODE 0x02:
                # AA 99 00 10 02 [tr_dir] 00 00 [pole] [chip_id] [ch_num] [fcode] [amp] [phase] [CRC_H] [CRC_L]
                payload_body = bytes([
                    0xAA, 0x99, 0x00, 0x10,
                    0x02, tr_dir, 0x00, 0x00,
                    pole, chip_id, ch_num, fcode_byte,
                    amp, phase
                ])
                crc_val = calc_crc16_ccitt_false(payload_body)
                binary_data = payload_body + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                
                target_desc = "发射(TX)" if target == "TX" else "接收(RX)"
                act_desc = "开启通道" if action == "OPEN" else ("关闭通道" if action == "CLOSE" else f"写入幅相(A={amp},P={phase})")
                body = f"xxSCC,{target},{chip_id},{ch},{action},{amp},{phase}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                custom_log = f">> [单通道{act_desc}] {target_desc} 芯片ID={chip_id:02d}, CH{ch} [BIN: {binary_data.hex(' ').upper()}]"

            elif cmd_type == "RESET":
                # FCODE 0x01 Reset command (复位所有TR芯片)
                # Frame: AA 99 00 0A 01 00 00 00 [CRC_H] [CRC_L]
                payload_body = bytes([0xAA, 0x99, 0x00, 0x0A, 0x01, 0x00, 0x00, 0x00])
                crc_val = calc_crc16_ccitt_false(payload_body)
                binary_data = payload_body + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                
                body = "xxRST,ALL"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                custom_log = f">> [阵面芯片复位] 所有 TR 芯片复位，所有寄存器与通道状态重置回初始未读状态 [BIN: {binary_data.hex(' ').upper()}]"
                
                # 重置上位机内所有寄存器值和通道开启状态回初始状态
                for d in ["TX", "RX"]:
                    for cid in range(16):
                        for ch in range(8):
                            state.registers[d][cid][ch] = {
                                "amp": None, "phase": None, "phase_deg": None,
                                "has_readback": False, "is_open": False, "error": False, "updated_at": 0
                            }
                state.system_status["beam_inverse"] = {"TX": None, "RX": None}
                state.system_status["tx_channels"] = "ALL_OFF"
                state.system_status["rx_channels"] = "ALL_OFF"

            elif cmd_type == "PTS":
                mode = int(req.get("mode", 0))
                cs = calc_nmea_checksum(f"xxPTS,{mode}")
                raw_frame = f"$xxPTS,{mode}*{cs}\r\n"

            elif cmd_type == "ATS":
                pitch = float(req.get("pitch", 0.0))
                yaw = float(req.get("yaw", 0.0))
                polr = float(req.get("polr", 0.0))
                polt = float(req.get("polt", 0.0))
                freq_r = float(req.get("freq_r", 12250.0))
                freq_t = float(req.get("freq_t", 14250.0))
                adcl = int(req.get("adcl", 0))
                body = f"xxATS,{pitch:.2f},{yaw:.2f},{polr:.2f},{polt:.2f},{freq_r:.1f},{freq_t:.1f},{adcl}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                # 波束更新时，默认发射和接收阵面所有通道都是打开的，回读后直接全亮
                for d in ["TX", "RX"]:
                    for cid in range(16):
                        for ch in range(8):
                            state.registers[d][cid][ch]["is_open"] = True

            elif cmd_type == "REG":
                r1 = int(str(req.get("reg1", "32")), 0) & 0xFF
                r2 = int(str(req.get("reg2", "32")), 0) & 0xFF
                r3 = int(str(req.get("reg3", "32")), 0) & 0xFF
                r4 = int(str(req.get("reg4", "32")), 0) & 0xFF
                dir_val = 1 if int(req.get("dir", 1)) == 1 else 0
                dir_str = "TX" if dir_val == 1 else "RX"
                chip_val = req.get("chip", 16)
                try:
                    chip_id = int(chip_val)
                except (ValueError, TypeError):
                    chip_id = 16

                if 0 <= chip_id < 16:
                    body = f"xxREG,{chip_id},{r1},{r2},{r3},{r4},{dir_val}"
                    custom_log = f">> [TR单芯片寄存器写入] CHIP={chip_id:02d}, R1={r1}, R2={r2}, R3={r3}, R4={r4}, Dir={dir_str}"
                else:
                    body = f"xxREG,{r1},{r2},{r3},{r4},{dir_val}"
                    custom_log = f">> [TR全阵面寄存器广播] R1={r1}, R2={r2}, R3={r3}, R4={r4}, Dir={dir_str}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"

            elif cmd_type == "BIP":
                profile = int(req.get("profile", 0)) & 0x07
                body = f"xxBIP,{profile}"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                p_desc = {
                    0: "标准均衡 (48 16 32 32 全阵面)",
                    1: "中心增强 (中心4片 48 24 36 32, 外围12片 48 16 32 32)",
                    2: "极限强增益 (48 24 36 32 全阵面)"
                }.get(profile, f"剖面 {profile}")
                custom_log = f">> [TR偏置剖面切换] Profile {profile}: {p_desc}"

            elif cmd_type == "RAW":
                text = req.get("command", "").strip()
                if not text.startswith('$'):
                    text = '$' + text
                if '*' not in text:
                    cs = calc_nmea_checksum(text)
                    text = f"{text}*{cs}"
                raw_frame = text + "\r\n"

            elif cmd_type == "HEX":
                hex_raw = req.get("hex", "").replace(" ", "").replace("\t", "").replace(",", "")
                auto_crc = bool(req.get("auto_crc", False))
                if not hex_raw:
                    self.send_json({"success": False, "msg": "十六进制指令内容不能为空"}, 400)
                    return
                try:
                    raw_b = bytes.fromhex(hex_raw)
                    if auto_crc:
                        crc_val = calc_crc16_ccitt_false(raw_b)
                        raw_b += bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                    binary_data = raw_b
                    custom_log = f">> [HEX TX] {raw_b.hex(' ').upper()} ({len(raw_b)} 字节)"
                    raw_frame = raw_b.hex(' ').upper()
                except ValueError as ve:
                    self.send_json({"success": False, "msg": f"无效的16进制格式: {str(ve)}"}, 400)
                    return

            elif cmd_type == "DUMP":
                # Manual trigger of array readback
                target_dir = req.get("dir", "TX")
                pitch = float(req.get("pitch", 0.0))
                yaw = float(req.get("yaw", 0.0))
                body = f"xxATS,{pitch:.2f},{yaw:.2f},0.00,0.00,12250.0,14250.0,0"
                cs = calc_nmea_checksum(body)
                raw_frame = f"${body}*{cs}\r\n"
                custom_log = f">> [幅相回读触发] {target_dir} ({raw_frame.strip()})"
                # 重新回读时将对应阵面所有通道置为开启点亮
                for cid in range(16):
                    for ch in range(8):
                        state.registers[target_dir][cid][ch]["is_open"] = True

            elif cmd_type == "ERT_CLEAR":
                tab_type = int(req.get("tab_type", 2)) # 0x02 = point clear, 0x03 = all clear
                pitch = float(req.get("pitch", 0.0))
                yaw = float(req.get("yaw", 0.0))
                freq = float(req.get("freq", 12.250)) # in GHz
                pole = int(req.get("pole", 0)) # 0: H, 1: V
                
                # Construct ATCL 17-byte frame for FCODE 0x08
                payload_body = bytearray(15)
                payload_body[0] = 0xAA
                payload_body[1] = 0x99
                payload_body[2] = 0x00
                payload_body[3] = 0x11 # length 17
                payload_body[4] = 0x08 # FCODE
                payload_body[5] = tab_type & 0xFF
                payload_body[6] = int(pitch) & 0xFF
                payload_body[7] = int(round((pitch - int(pitch)) * 10)) & 0xFF
                payload_body[8] = (int(yaw) >> 8) & 0xFF
                payload_body[9] = int(yaw) & 0xFF
                payload_body[10] = int(round((yaw - int(yaw)) * 10)) & 0xFF
                payload_body[11] = int(freq) & 0xFF
                frac_f = int(round((freq - int(freq)) * 1000))
                payload_body[12] = (frac_f >> 8) & 0xFF
                payload_body[13] = frac_f & 0xFF
                payload_body[14] = pole & 0xFF

                crc_val = calc_crc16_ccitt_false(payload_body)
                binary_data = bytes(payload_body) + bytes([(crc_val >> 8) & 0xFF, crc_val & 0xFF])
                
                tab_desc = "单点误差表清零" if tab_type == 2 else "全阵面误差表清空"
                pole_desc = "水平极化(H)" if pole == 0 else "垂直极化(V)"
                custom_log = f">> [标校{tab_desc}] 离轴/俯仰={pitch:.1f}°, 方位={yaw:.1f}°, 频点={freq:.3f}GHz, 极化={pole_desc} [BIN: {binary_data.hex(' ').upper()}]"
                raw_frame = binary_data.hex(' ').upper()

            else:
                self.send_json({"success": False, "msg": f"未知指令类型: {cmd_type}"}, 400)
                return

            # Transmit via UART0
            sent_bytes = 0
            with state.lock:
                log_entry = custom_log if custom_log else f">> {raw_frame.strip()}"
                hex_dump = binary_data.hex(' ').upper() if binary_data else raw_frame.encode('utf-8', errors='ignore').hex(' ').upper()
                state.uart0_logs.append({
                    "time": time.strftime("%H:%M:%S"),
                    "text": log_entry,
                    "hex": hex_dump
                })
                if len(state.uart0_logs) > 400:
                    state.uart0_logs.pop(0)

                if cmd_type == "POWER":
                    state.system_status["array_power"] = "ON" if action == 1 else "OFF"
                elif cmd_type == "CHANNEL_ALL":
                    state.system_status["tx_channels" if target == "TX" else "rx_channels"] = "ALL_ON" if action == 1 else "ALL_OFF"

                if state.uart0_status == "connected" and state.uart0_conn and state.uart0_conn.is_open:
                    try:
                        if binary_data:
                            sent_bytes = state.uart0_conn.write(binary_data)
                            if cmd_type in ("POWER", "CHANNEL_ALL"):
                                try:
                                    state.uart0_conn.write(raw_frame.encode('utf-8'))
                                except Exception:
                                    pass
                        else:
                            sent_bytes = state.uart0_conn.write(raw_frame.encode('utf-8'))
                    except Exception as e:
                        self.send_json({"success": False, "msg": f"UART0 发送出错: {str(e)}"}, 500)
                        return
                elif state.simulation_mode:
                    sent_bytes = len(binary_data) if binary_data else len(raw_frame)
                    if cmd_type == "POWER":
                        process_uart3_line(f"POWER: 阵面{'上电' if action==1 else '下电'}成功 (tr_dir=0x{target_dir:02X}, onf_flg=0x{onf_flg:02X})")
                    elif cmd_type == "CHANNEL_ALL":
                        target_name = '发射' if target == 'TX' else '接收'
                        act_name = '全开' if action == 1 else '全关'
                        process_uart0_line(f"${body}*{cs}", binary_data.hex(' ').upper())
                        process_uart3_line(f"OPE: {target_name}通道{act_name}成功 (rxtx_sft=0x{rxtx_sft:02X}, chips=16, ch=128)")
                        for cid in range(16):
                            for ch in range(8):
                                state.registers[target][cid][ch]["amp"] = 63 if action == 1 else 0
                                state.registers[target][cid][ch]["is_open"] = (action == 1)
                                state.registers[target][cid][ch]["has_readback"] = True
                                state.registers[target][cid][ch]["updated_at"] = time.time()
                        f_hz = 14.25e9 if target == "TX" else 12.25e9
                        state.system_status["beam_inverse"][target] = solve_beam_angles(state.registers[target], target, f_hz)
                    elif cmd_type == "SINGLE_CH":
                        process_uart0_line(raw_frame.strip(), binary_data.hex(' ').upper())
                        if action == "OPEN":
                            state.registers[target][chip_id][ch]["is_open"] = True
                            process_uart3_line(f"CH_STATUS,{target},ID={chip_id:02d},CH={ch},OPEN=1,MASK=0x{ch_num:02X}")
                        elif action == "CLOSE":
                            state.registers[target][chip_id][ch]["is_open"] = False
                            process_uart3_line(f"CH_STATUS,{target},ID={chip_id:02d},CH={ch},OPEN=0,MASK=0x00")
                        else: # "SET"
                            state.registers[target][chip_id][ch]["is_open"] = True
                            process_uart3_line(f"SINGLE_CH_SET,{target},ID={chip_id:02d},CH={ch},AMP={amp},PHASE={phase}")
                            # 写入指定通道的寄存器值
                            state.registers[target][chip_id][ch]["amp"] = amp
                            state.registers[target][chip_id][ch]["phase"] = phase
                            state.registers[target][chip_id][ch]["phase_deg"] = round(phase * 5.625, 3)
                            state.registers[target][chip_id][ch]["has_readback"] = True
                            state.registers[target][chip_id][ch]["updated_at"] = time.time()
                            # 模拟触发全寄存器回读流
                            emit_simulated_chip_dump(target)
                    elif cmd_type == "RESET":
                        process_uart0_line("$xxRST,ALL*12", binary_data.hex(' ').upper())
                        process_uart3_line("rst  ")
                    elif cmd_type == "PTS":
                        mode = int(req.get("mode", 0))
                        process_uart0_line(f"$xxPTS,{mode}*55")
                        process_uart3_line(f"PTS: 切换为{'自动' if mode==1 else '手动'}对星模式 (auto_mode={mode})")
                    elif cmd_type == "ATS":
                        pitch_val = float(req.get('pitch', 0))
                        yaw_val = float(req.get('yaw', 0))
                        polr_val = float(req.get('polr', 0))
                        polt_val = float(req.get('polt', 0))
                        freq_r_val = float(req.get('freq_r', 12250))
                        freq_t_val = float(req.get('freq_t', 14250))

                        process_uart3_line(f"BEAM_UPDATE,RX,PITCH={pitch_val:.3f},YAW={yaw_val:.3f},POL={polr_val:.3f},FREQ_MHZ={freq_r_val:.3f},AMP_CMD=63,DUMP=QUEUED")
                        emit_simulated_chip_dump("RX")

                        process_uart3_line(f"BEAM_UPDATE,TX,PITCH={pitch_val:.3f},YAW={yaw_val:.3f},POL={polt_val:.3f},FREQ_MHZ={freq_t_val:.3f},AMP_CMD=63,DUMP=QUEUED")
                        emit_simulated_chip_dump("TX")
                    elif cmd_type == "REG":
                        chip_val = req.get("chip", 16)
                        try:
                            chip_id = int(chip_val)
                        except (ValueError, TypeError):
                            chip_id = 16
                        r1 = req.get('reg1', 32)
                        r2 = req.get('reg2', 32)
                        r3 = req.get('reg3', 32)
                        r4 = req.get('reg4', 32)
                        d = req.get('dir', 1)
                        if 0 <= chip_id < 16:
                            reply_line = f"$xxREG,{chip_id},{r1},{r2},{r3},{r4},{d},OK"
                        else:
                            reply_line = f"$xxREG,{r1},{r2},{r3},{r4},{d},OK"
                        cs = calc_nmea_checksum(reply_line)
                        full_reply = f"{reply_line}*{cs}"
                        process_uart3_line(full_reply)
                    elif cmd_type == "BIP":
                        prof = int(req.get("profile", 0))
                        reply_line = f"$xxBIP,{prof},OK"
                        cs = calc_nmea_checksum(reply_line)
                        full_reply = f"{reply_line}*{cs}"
                        process_uart3_line(full_reply)
                    elif cmd_type == "HEX":
                        process_uart3_line(f"HEX_RECV_ACK,LEN={sent_bytes}")
                    elif cmd_type == "DUMP":
                        target_dir = req.get("dir", "TX")
                        emit_simulated_chip_dump(target_dir)
                    elif cmd_type == "ERT_CLEAR":
                        tab_desc = "单点" if tab_type == 2 else "全阵面"
                        process_uart0_line(binary_data.hex(' ').upper(), binary_data.hex(' ').upper())
                        process_uart3_line(f"ERT clear start: type=0x{tab_type:02X} pitch={pitch:.1f} yaw={yaw:.1f} freq={freq:.3f}")
                        process_uart3_line(f"ERT {tab_desc}误差表清零成功 (Flash 擦除响应, 0x08 ACK 回传)", binary_data.hex(' ').upper())
                else:
                    self.send_json({"success": False, "msg": "UART0 未连接，请先连接串口或开启模拟模式"}, 400)
                    return

            self.send_json({"success": True, "frame": raw_frame.strip(), "bytes": sent_bytes})
            return

        self.send_error(404, "API Not Found")

def has_webview2_runtime():
    """Return True when a usable Evergreen WebView2 Runtime is registered."""
    if os.name != 'nt':
        return True

    try:
        import winreg
    except ImportError:
        return False

    client_id = '{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}'
    key_paths = (
        rf'SOFTWARE\Microsoft\EdgeUpdate\Clients\{client_id}',
        rf'SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{client_id}',
    )
    registry_roots = (winreg.HKEY_CURRENT_USER, winreg.HKEY_LOCAL_MACHINE)

    for root in registry_roots:
        for key_path in key_paths:
            try:
                with winreg.OpenKey(root, key_path) as key:
                    version, _ = winreg.QueryValueEx(key, 'pv')
                if str(version).strip() not in ('', '0.0.0.0'):
                    print(f'[GUI] 检测到 Microsoft Edge WebView2 Runtime {version}')
                    return True
            except OSError:
                continue

    print('[WARN] 未检测到 Microsoft Edge WebView2 Runtime，将使用系统浏览器兼容模式。')
    return False

def open_system_browser(url, reason=''):
    if reason:
        print(f'[BROWSER] {reason}')
    try:
        opened = webbrowser.open(url)
        if not opened:
            print(f'[WARN] 无法自动打开系统浏览器，请手动访问 {url}')
        return opened
    except Exception as exc:
        print(f'[WARN] 打开系统浏览器失败: {exc}；请手动访问 {url}')
        return False

def start_server():
    for arg in sys.argv:
        if arg.startswith("--pol-mode="):
            m = arg.split("=")[1].strip()
            if m in ["+2*pol", "-2*pol"]:
                state.pol_algorithm_mode = m
                state.system_status["pol_algorithm_mode"] = m

    # Start threads
    t0 = threading.Thread(target=uart0_worker, daemon=True)
    t3 = threading.Thread(target=uart3_worker, daemon=True)
    tsim = threading.Thread(target=simulation_worker, daemon=True)
    t0.start()
    t3.start()
    tsim.start()

    actual_port = PORT
    server = None
    for p in range(PORT, PORT + 10):
        try:
            server = ThreadedHTTPServer(('127.0.0.1', p), RequestHandler)
            actual_port = p
            break
        except OSError:
            continue

    if not server:
        print(f"[ERROR] 启动失败: 端口 {PORT}~{PORT+9} 全部被占用！")
        return

    print(f"============================================================")
    print(f"  GT2001 双串口回读与相控阵波束监控软件已启动!")
    print(f"  内部服务通信端口: http://127.0.0.1:{actual_port}")
    print(f"  当前极化反推算法: {state.pol_algorithm_mode} ({'当前固件修正版本' if state.pol_algorithm_mode == '+2*pol' else '旧版固件算法版本'})")
    print(f"  UART0: 命令发送口 ($PTS, $xxATS, $xxREG, FCODE 0x08)")
    print(f"  UART3: 调试回读口 (BEAM_REG 幅相回读, BEAM_UPDATE 状态)")
    print(f"============================================================")

    url = f"http://127.0.0.1:{actual_port}"

    # Run HTTP backend in daemon thread
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()

    # Determine whether to run in native GUI desktop window mode or fallback to browser
    use_browser = "--browser" in sys.argv
    webview2_ready = has_webview2_runtime() if HAS_WEBVIEW and not use_browser else False
    use_gui = HAS_WEBVIEW and webview2_ready and not use_browser

    if use_gui:
        print("[GUI] 正在启动纯 GUI 独立桌面窗口应用...")
        base_dir = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
        icon_path = os.path.join(base_dir, "app_icon.ico")
        if not os.path.exists(icon_path):
            icon_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "app_icon.ico")
        if not os.path.exists(icon_path):
            icon_path = None

        try:
            window = webview.create_window(
                title="GT2001 相控阵天线综合监视与控制台",
                url=url,
                width=1520,
                height=920,
                min_size=(1120, 680),
                text_select=True,
                zoomable=True,
                background_color="#0b1120"
            )
            # If WebView2 is installed but damaged or blocked by policy/driver,
            # never leave the user with an unexplained dark window.
            page_loaded = threading.Event()

            def on_page_loaded():
                page_loaded.set()

            def webview_startup_watchdog():
                if not page_loaded.wait(timeout=12.0):
                    open_system_browser(
                        url,
                        '内嵌窗口 12 秒内未完成加载，已自动切换到系统浏览器。'
                    )

            window.events.loaded += on_page_loaded
            threading.Thread(target=webview_startup_watchdog, daemon=True).start()
            webview.start(
                gui='edgechromium',
                icon=icon_path,
                debug=False,
                private_mode=False,
                storage_path=os.path.join(APP_STORAGE_DIR, ".cache")
            )
            print("\nGUI 桌面窗口已关闭，程序安全退出。")
        except Exception as e:
            print(f"[WARN] GUI 桌面窗口启动异常: {e}\n{traceback.format_exc()}")
            open_system_browser(url, '内嵌窗口启动失败，已自动切换到系统浏览器。')
            try:
                server_thread.join()
            except KeyboardInterrupt:
                pass
    else:
        reason = '已按 --browser 参数启动兼容模式。' if use_browser else (
            '未检测到可用的 WebView2 Runtime，已使用系统浏览器兼容模式。'
        )
        open_system_browser(url, reason)
        try:
            while server_thread.is_alive():
                server_thread.join(timeout=1.0)
        except KeyboardInterrupt:
            print("\n服务器正在关闭...")

    # Cleanup open serial ports on application exit safely
    try:
        if state.uart0_conn and state.uart0_conn.is_open:
            state.uart0_conn.close()
    except Exception:
        pass
    try:
        if state.uart3_conn and state.uart3_conn.is_open:
            state.uart3_conn.close()
    except Exception:
        pass
    try:
        server.shutdown()
    except Exception:
        pass

if __name__ == "__main__":
    start_server()
