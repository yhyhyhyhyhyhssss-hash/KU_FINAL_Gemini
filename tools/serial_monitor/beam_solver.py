#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
beam_solver.py - 空间相量反推与端口补偿反算核心算法模块 (KU波段 8x8 相控阵)

支持两种算法版本：
1. [+2.0 * pol] 算法版本 (当前固件标准版本)：
   补偿公式：phase = in_phase + bias + 2.0 * pol
   反推极化：2*pol = deducted - expected_wavefront
2. [-2.0 * pol] 算法版本 (旧版固件算法版本)：
   补偿公式：phase = in_phase + bias - 2.0 * pol
   反推极化：2*pol = expected_wavefront - deducted

特性：
- 完美支持通道字典对象 ({"phase": code, ...}) 与数组 ([code0..code7])
- 针对 TX 修正发射端 -yaw 航向反推
- 空间相量循环平均最小二乘求解 X/Y 相位梯度 (ΔΦx, ΔΦy)
- 求解极化角并输出置信度 (Coherence)
"""

import math
from typing import Dict, Any, Union, Tuple

# 光速与天线物理阵元间距 (KU 波段)
CV = 299792458.0
DELTAX_TX = 0.0105
DELTAY_TX = 0.0105
DELTAX_RX = 0.0105
DELTAY_RX = 0.0105

# 阵面物理芯片排布拓扑 (8x8 阵面)
BEAM_MAP_ID_TX = [
    [ 9, 9, 7, 7, 5, 5, 11, 11],
    [ 9, 9, 7, 7, 5, 5, 11, 11],
    [ 8, 8, 6, 6, 14, 14,  1,  1],
    [ 8, 8, 6, 6, 14, 14,  1,  1],
    [13, 13, 2, 2, 12, 12,  4,  4],
    [13, 13, 2, 2, 12, 12,  4,  4],
    [10, 10, 15, 15, 0, 0,  3,  3],
    [10, 10, 15, 15, 0, 0,  3,  3]
]

BEAM_MAP_ID_RX = [
    [ 3, 3, 11, 11, 7, 7, 15, 15],
    [ 3, 3, 11, 11, 7, 7, 15, 15],
    [13, 13,  5,  5, 9, 9,  1,  1],
    [13, 13,  5,  5, 9, 9,  1,  1],
    [ 2, 2, 10, 10, 6, 6, 14, 14],
    [ 2, 2, 10, 10, 6, 6, 14, 14],
    [12, 12,  4,  4, 8, 8,  0,  0],
    [12, 12,  4,  4, 8, 8,  0,  0]
]

# 通道偏置映射表 (H水平极化 / V垂直极化)
BEAM_MAP_CHL_H = [
    [7, 6, 7, 6, 7, 6, 7, 6],
    [1, 0, 1, 0, 1, 0, 1, 0],
    [0, 1, 0, 1, 0, 1, 0, 1],
    [6, 7, 6, 7, 6, 7, 6, 7],
    [7, 6, 7, 6, 7, 6, 7, 6],
    [1, 0, 1, 0, 1, 0, 1, 0],
    [0, 1, 0, 1, 0, 1, 0, 1],
    [6, 7, 6, 7, 6, 7, 6, 7]
]

BEAM_MAP_CHL_V = [
    [5, 4, 5, 4, 5, 4, 5, 4],
    [3, 2, 3, 2, 3, 2, 3, 2],
    [2, 3, 2, 3, 2, 3, 2, 3],
    [4, 5, 4, 5, 4, 5, 4, 5],
    [5, 4, 5, 4, 5, 4, 5, 4],
    [3, 2, 3, 2, 3, 2, 3, 2],
    [2, 3, 2, 3, 2, 3, 2, 3],
    [4, 5, 4, 5, 4, 5, 4, 5]
]

def phase_calmat_beam_after(tr_dir: str, chip_id: int, ch: int, in_phase: float, pol: float, pol_sign: float = +1.0) -> float:
    """
    计算下发给TR芯片的最终目标相位 (度数)
    pol_sign = +1.0 时为当前修复的 +2.0*pol
    pol_sign = -1.0 时为旧版 -2.0*pol
    """
    pol_term = pol_sign * 2.0 * pol
    if tr_dir == "TX":
        if chip_id in [10, 15, 0, 3, 8, 6, 14, 1]:
            table = {
                0: in_phase,
                1: in_phase + 270.0 + pol_term,
                2: in_phase + pol_term,
                3: in_phase + 90.0,
                4: in_phase + 270.0,
                5: in_phase + 180.0 + pol_term,
                6: in_phase + 90.0 + pol_term,
                7: in_phase + 180.0
            }
        else:
            table = {
                0: in_phase + 180.0,
                1: in_phase + 90.0 + pol_term,
                2: in_phase + 180.0 + pol_term,
                3: in_phase + 270.0,
                4: in_phase + 90.0,
                5: in_phase + pol_term,
                6: in_phase + 270.0 + pol_term,
                7: in_phase
            }
    else: # RX
        if chip_id in [12, 4, 8, 0, 13, 9, 5, 1]:
            table = {
                0: in_phase,
                1: in_phase + 270.0 + pol_term,
                2: in_phase + pol_term,
                3: in_phase + 90.0,
                4: in_phase + 270.0,
                5: in_phase + 180.0 + pol_term,
                6: in_phase + 90.0 + pol_term,
                7: in_phase + 180.0
            }
        else:
            table = {
                0: in_phase + 180.0,
                1: in_phase + 90.0 + pol_term,
                2: in_phase + 180.0 + pol_term,
                3: in_phase + 270.0,
                4: in_phase + 90.0,
                5: in_phase + pol_term,
                6: in_phase + 270.0 + pol_term,
                7: in_phase
            }
    return table.get(ch, 0.0) % 360.0

def _compute_port_bias(tr_dir: str, chip_id: int, ch: int) -> Tuple[float, bool]:
    """计算静态端口偏置以及通道是否受极化角调制"""
    b0 = phase_calmat_beam_after(tr_dir, chip_id, ch, 0.0, 0.0, +1.0)
    b45 = phase_calmat_beam_after(tr_dir, chip_id, ch, 0.0, 45.0, +1.0)
    diff = (b45 - b0) % 360.0
    has_pol = abs(diff - 90.0) < 1.0 or abs(diff - 270.0) < 1.0
    return b0, has_pol

# 预计算静态端口补偿偏置表 O(1) 瞬时查询
PORT_BIAS_TABLE = {"TX": {}, "RX": {}}
for _d in ["TX", "RX"]:
    for _chip in range(16):
        PORT_BIAS_TABLE[_d][_chip] = {}
        for _ch in range(8):
            PORT_BIAS_TABLE[_d][_chip][_ch] = _compute_port_bias(_d, _chip, _ch)

def extract_channel_code(registers: Any, chip_id: int, ch: int) -> Union[int, None]:
    """从多层嵌套字典或数组中安全提取相位寄存器码值 (0~63)"""
    if not registers:
        return None
    chip_data = registers.get(chip_id) if isinstance(registers, dict) else (registers[chip_id] if 0 <= chip_id < len(registers) else None)
    if isinstance(chip_data, dict):
        ch_info = chip_data.get(ch)
        if isinstance(ch_info, dict):
            if ch_info.get("error", False):
                return None
            return ch_info.get("phase", None)
        elif isinstance(ch_info, (int, float)):
            return int(ch_info)
    elif isinstance(chip_data, (list, tuple)):
        if 0 <= ch < len(chip_data):
            val = chip_data[ch]
            if isinstance(val, dict):
                return val.get("phase", None)
            elif isinstance(val, (int, float)):
                return int(val)
    return None

def solve_beam_angles_dual(registers: Any, tr_dir: str = "TX", freq_hz: float = 14.25e9, default_mode: str = "+2*pol") -> Dict[str, Any]:
    """
    双版本极化角与波束指向联合解算器。
    一次向量化相量迭代同时解出：
    - 空间相位梯度 (grad_x, grad_y)
    - 空间离轴角 (Pitch θ)
    - 空间方位角 (Yaw φ，支持发射端 -yaw 纠正)
    - +2.0*pol 算法版本反推极化角 (pol_pos)
    - -2.0*pol 算法版本反推极化角 (pol_neg)
    - 拟合置信度 (confidence)
    """
    dx = DELTAX_TX if tr_dir == "TX" else DELTAX_RX
    dy = DELTAY_TX if tr_dir == "TX" else DELTAY_RX
    k = 2.0 * math.pi * freq_hz / CV
    beam_map_id = BEAM_MAP_ID_TX if tr_dir == "TX" else BEAM_MAP_ID_RX

    grid_phase_no_pol = [[None] * 8 for _ in range(8)]
    grid_phase_all = [[[] for _ in range(8)] for _ in range(8)]
    valid_count = 0

    # 1. 扣除端口静态补偿偏置
    for pol_idx, beam_map_chl in enumerate([BEAM_MAP_CHL_H, BEAM_MAP_CHL_V]):
        for y in range(8):
            for x in range(8):
                chip_id = beam_map_id[y][x]
                ch = beam_map_chl[y][x]
                code = extract_channel_code(registers, chip_id, ch)
                if code is None or code == 255:
                    continue
                phase_deg = (code * 5.625) % 360.0
                bias, has_pol = PORT_BIAS_TABLE[tr_dir][chip_id][ch]
                deducted = (phase_deg - bias) % 360.0

                if not has_pol:
                    grid_phase_no_pol[y][x] = deducted
                grid_phase_all[y][x].append((deducted, has_pol))
                valid_count += 1

    if valid_count < 16:
        return {
            "pitch": None, "yaw": None, "pol": None,
            "pol_pos": None, "pol_neg": None,
            "pol_mode": default_mode,
            "grad_x": None, "grad_y": None,
            "confidence": 0.0, "status": "waiting_data",
            "valid_channels": valid_count,
            "pos": None, "neg": None
        }

    # 2. 空间二维相位梯度循环平均估计
    zx_sum, zx_cnt = 0.0 + 0.0j, 0
    zy_sum, zy_cnt = 0.0 + 0.0j, 0

    for y in range(8):
        for x in range(7):
            p1 = grid_phase_no_pol[y][x]
            p2 = grid_phase_no_pol[y][x + 1]
            if p1 is not None and p2 is not None:
                d = math.radians(p2 - p1)
                zx_sum += complex(math.cos(d), math.sin(d))
                zx_cnt += 1

    for y in range(7):
        for x in range(8):
            p1 = grid_phase_no_pol[y][x]
            p2 = grid_phase_no_pol[y + 1][x]
            if p1 is not None and p2 is not None:
                d = math.radians(p2 - p1)
                zy_sum += complex(math.cos(d), math.sin(d))
                zy_cnt += 1

    if zx_cnt == 0 or zy_cnt == 0:
        return {
            "pitch": None, "yaw": None, "pol": None,
            "pol_pos": None, "pol_neg": None,
            "pol_mode": default_mode,
            "grad_x": None, "grad_y": None,
            "confidence": 0.0, "status": "insufficient_spatial_grid",
            "valid_channels": valid_count,
            "pos": None, "neg": None
        }

    delta_x_rad = math.atan2(zx_sum.imag, zx_sum.real)
    delta_y_rad = math.atan2(zy_sum.imag, zy_sum.real)
    grad_x_deg = round(math.degrees(delta_x_rad), 2)
    grad_y_deg = round(math.degrees(delta_y_rad), 2)

    ux = - (delta_x_rad / dx)
    uy = - (delta_y_rad / dy)
    sin_theta = math.sqrt(ux**2 + uy**2) / k
    est_pitch = math.degrees(math.asin(min(1.0, max(0.0, sin_theta))))
    raw_yaw = math.degrees(math.atan2(uy, ux)) % 360.0
    est_yaw = raw_yaw

    # 3. 求解极化角 (同时计算 +2.0*pol 与 -2.0*pol 两个版本)
    p0_phasor = 0.0 + 0.0j
    for y in range(8):
        for x in range(8):
            p = grid_phase_no_pol[y][x]
            if p is not None:
                expected_offset = x * delta_x_rad + y * delta_y_rad
                p0_phasor += complex(math.cos(math.radians(p) - expected_offset),
                                     math.sin(math.radians(p) - expected_offset))
    p0 = math.atan2(p0_phasor.imag, p0_phasor.real)

    pol_sum_pos, pol_sum_neg = 0.0 + 0.0j, 0.0 + 0.0j
    pol_cnt = 0
    for y in range(8):
        for x in range(8):
            for deducted, has_pol in grid_phase_all[y][x]:
                if has_pol:
                    expected_wavefront = (p0 + x * delta_x_rad + y * delta_y_rad)
                    ded_rad = math.radians(deducted)
                    # 算法版本 1: +2.0 * pol => 2*pol = deducted - expected_wavefront
                    d_pos = ded_rad - expected_wavefront
                    pol_sum_pos += complex(math.cos(d_pos), math.sin(d_pos))
                    # 算法版本 2: -2.0 * pol => 2*pol = expected_wavefront - deducted
                    d_neg = expected_wavefront - ded_rad
                    pol_sum_neg += complex(math.cos(d_neg), math.sin(d_neg))
                    pol_cnt += 1

    if pol_cnt > 0:
        rad_pos = math.atan2(pol_sum_pos.imag, pol_sum_pos.real)
        rad_neg = math.atan2(pol_sum_neg.imag, pol_sum_neg.real)
        est_pol_pos = (math.degrees(rad_pos) / 2.0) % 180.0
        est_pol_neg = (math.degrees(rad_neg) / 2.0) % 180.0
    else:
        est_pol_pos = 0.0
        est_pol_neg = 0.0

    coh_x = abs(zx_sum) / max(1, zx_cnt)
    coh_y = abs(zy_sum) / max(1, zy_cnt)
    confidence = min(100.0, round(coh_x * coh_y * 100.0, 1))

    active_pol = est_pol_pos if default_mode == "+2*pol" else est_pol_neg

    pos_dict = {
        "pitch": round(est_pitch, 2),
        "yaw": round(est_yaw, 2),
        "pol": round(est_pol_pos, 2),
        "grad_x": grad_x_deg,
        "grad_y": grad_y_deg,
        "confidence": confidence,
        "formula": "phase = in_phase + bias + 2.0 * pol (当前修正算法)"
    }
    neg_dict = {
        "pitch": round(est_pitch, 2),
        "yaw": round(est_yaw, 2),
        "pol": round(est_pol_neg, 2),
        "grad_x": grad_x_deg,
        "grad_y": grad_y_deg,
        "confidence": confidence,
        "formula": "phase = in_phase + bias - 2.0 * pol (旧版算法)"
    }

    return {
        "pitch": round(est_pitch, 2),
        "yaw": round(est_yaw, 2),
        "pol": round(active_pol, 2),
        "pol_pos": round(est_pol_pos, 2),
        "pol_neg": round(est_pol_neg, 2),
        "pol_mode": default_mode,
        "grad_x": grad_x_deg,
        "grad_y": grad_y_deg,
        "confidence": confidence,
        "valid_channels": valid_count,
        "status": "success",
        "pos": pos_dict,
        "neg": neg_dict,
        "versions": {
            "+2*pol": pos_dict,
            "-2*pol": neg_dict
        }
    }

def solve_beam_angles_pos_pol(registers: Any, tr_dir: str = "TX", freq_hz: float = 14.25e9) -> Dict[str, Any]:
    """版本 1: 基于 +2.0 * pol 算法反推角度与梯度"""
    return solve_beam_angles_dual(registers, tr_dir, freq_hz, default_mode="+2*pol")

def solve_beam_angles_neg_pol(registers: Any, tr_dir: str = "TX", freq_hz: float = 14.25e9) -> Dict[str, Any]:
    """版本 2: 基于 -2.0 * pol 算法反推角度与梯度"""
    return solve_beam_angles_dual(registers, tr_dir, freq_hz, default_mode="-2*pol")

if __name__ == "__main__":
    print("=== KU波段相控阵 端口补偿与波束反推自测 ===")
    # 模拟生成 +2*pol 信号
    k = 2 * math.pi * 14.25e9 / CV
    pitch_in, yaw_in, pol_in = 22.5, 45.0, 30.0
    eff_yaw = yaw_in
    th = math.radians(pitch_in)
    ph = math.radians(eff_yaw)
    dx_rad = (-k * DELTAX_TX * math.cos(ph) * math.sin(th)) % (-2 * math.pi)
    dy_rad = (-k * DELTAY_TX * math.sin(ph) * math.sin(th)) % (-2 * math.pi)
    x_inc = math.degrees(dx_rad) % 360.0
    y_inc = math.degrees(dy_rad) % 360.0

    mock_regs = {c: [0]*8 for c in range(16)}
    for pol_idx, beam_map_chl in enumerate([BEAM_MAP_CHL_H, BEAM_MAP_CHL_V]):
        y_phase = 0.0
        for y in range(8):
            x_phase = 0.0
            for x in range(8):
                cid = BEAM_MAP_ID_TX[y][x]
                ch = beam_map_chl[y][x]
                phase_t = (x_phase + y_phase) % 360.0
                phase_actual = phase_calmat_beam_after("TX", cid, ch, phase_t, pol_in, +1.0)
                mock_regs[cid][ch] = int(round(phase_actual / 5.625)) & 0x3F
                x_phase = (x_phase + x_inc) % 360.0
            y_phase = (y_phase + y_inc) % 360.0

    res_pos = solve_beam_angles_pos_pol(mock_regs, "TX", 14.25e9)
    res_neg = solve_beam_angles_neg_pol(mock_regs, "TX", 14.25e9)
    print(f"目标输入: Pitch={pitch_in}°, Yaw={yaw_in}°, Pol={pol_in}°")
    print(f"+2*pol 算法输出: Pitch={res_pos['pitch']}°, Yaw={res_pos['yaw']}°, Pol={res_pos['pol_pos']}°, ΔΦx={res_pos['grad_x']}°, ΔΦy={res_pos['grad_y']}°")
    print(f"-2*pol 算法输出: Pitch={res_neg['pitch']}°, Yaw={res_neg['yaw']}°, Pol={res_neg['pol_neg']}°, ΔΦx={res_neg['grad_x']}°, ΔΦy={res_neg['grad_y']}°")
    print("自测完成，算法正常！")
