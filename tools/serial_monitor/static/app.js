/**
 * GT2001 Dual-UART Phased-Array Antenna Monitor & Control Frontend
 * Enhanced Version: Strict Readback Display, HEX Terminal & Custom Sending,
 * Persistent Recommended TR Presets, and Expandable Log View.
 */

let currentArray = 'TX';
let displayMode = 'PHASE';
let isDeductPortComp = false; // 是否开启“去端口补偿显示”(还原测试系统最初写入的理论初值)
let searchQuery = '';
let activeLogTab = 'uart3';
let activeCmdTab = 'power_pts';
let autoScroll = false; // 默认状态自动滚屏关闭，用户手动翻看历史日志不被强制置底
let logViewMode = 'ascii'; // 'ascii', 'hex', 'both'
let isLogExpanded = false;
let userCustomLogHeight = null; // 用户手动上下拖拽设定的日志高度 (px)

// ========================================================
// 端口补偿偏置映射表 (Port Bias Lookup Table)
// 依据天线微带拓扑与 phase_calmat_beam_after(in_phase=0, pol=0) 严格计算
// ========================================================
function getPortBiasDeg(trDir, chipId, ch) {
    if (trDir === 'TX') {
        const isGrp1 = [10, 15, 0, 3, 8, 6, 14, 1].includes(chipId);
        if (isGrp1) {
            const tableGrp1 = [0.0, 270.0, 0.0, 90.0, 270.0, 180.0, 90.0, 180.0];
            return tableGrp1[ch] !== undefined ? tableGrp1[ch] : 0.0;
        } else {
            const tableGrp2 = [180.0, 90.0, 180.0, 270.0, 90.0, 0.0, 270.0, 0.0];
            return tableGrp2[ch] !== undefined ? tableGrp2[ch] : 0.0;
        }
    } else { // RX
        const isGrp1 = [12, 4, 8, 0, 13, 9, 5, 1].includes(chipId);
        if (isGrp1) {
            const tableGrp1 = [0.0, 270.0, 0.0, 90.0, 270.0, 180.0, 90.0, 180.0];
            return tableGrp1[ch] !== undefined ? tableGrp1[ch] : 0.0;
        } else {
            const tableGrp2 = [180.0, 90.0, 180.0, 270.0, 90.0, 0.0, 270.0, 0.0];
            return tableGrp2[ch] !== undefined ? tableGrp2[ch] : 0.0;
        }
    }
}

/**
 * 扣除端口微带补偿偏置，反推测试系统最初写入的理论初值
 */
function getDeductedPortPhase(trDir, chipId, ch, phaseDeg) {
    const bias = getPortBiasDeg(trDir, chipId, ch);
    let deductedDeg = (phaseDeg - bias) % 360.0;
    if (deductedDeg < 0) deductedDeg += 360.0;
    let deductedCode = Math.round(deductedDeg / 5.625) % 64;
    if (deductedCode < 0) deductedCode += 64;
    return {
        deductedDeg: deductedDeg,
        deductedCode: deductedCode,
        biasDeg: bias
    };
}

let statusData = null;
let pollTimer = null;

// 8 TR Presets (4 Recommended + 4 Custom Slots)
const TR_PRESET_DEFINITIONS = {
    group1: {
        name: '标称均衡',
        badge: '推荐 1: 标称均衡 (32, 32, 32, 32)',
        icon: '⭐',
        reg1: '32', reg2: '32', reg3: '32', reg4: '32',
        descTag: '50% 标称均衡工作点',
        desc: '标称衰减均衡工作点（各寄存器置 32 / 0x20），放大器处于线性动态区间中心，适用于常规暗室方向图与出厂基础校准。'
    },
    group2: {
        name: '全功率发射',
        badge: '推荐 2: 全功率发射 (63, 63, 63, 63)',
        icon: '🚀',
        reg1: '63', reg2: '63', reg3: '63', reg4: '63',
        descTag: '全通零衰减饱和模式',
        desc: '微带通道全通零衰减（全 63 / 0x3F），末级功放 PA 最大功率饱和输出，适用于阵面最大全向辐射功率 (EIRP) 远场测试。'
    },
    group3: {
        name: '黄金高敏',
        badge: '推荐 3: 黄金高敏 (48, 16, 32, 32)',
        icon: '📡',
        reg1: '48', reg2: '16', reg3: '32', reg4: '32',
        descTag: '最优信噪比/低噪声高敏接收',
        desc: '前级LNA1高偏置(48)压制系统前端噪声系数(NF)，次级LNA2适度偏置(16)保障线性动态范围，AMP驱动平衡分配(32, 32)，实测信噪比(SNR)与门限灵敏度最佳。'
    },
    group4: {
        name: '极限强增益',
        badge: '推荐 4: 极限强增益 (48, 24, 36, 32)',
        icon: '⚡',
        reg1: '48', reg2: '24', reg3: '36', reg4: '32',
        descTag: '超远距离/弱星标微弱捕获',
        desc: '提升次级LNA与一级驱动放大器跨导增益，获得额外+2.5dB微弱星标链路增益，适合极端仰角或降雨衰减对星场景。'
    },
    custom1: {
        name: '预设组 A',
        badge: '自定义 A: 用户记忆组 A',
        icon: '💾',
        reg1: '32', reg2: '32', reg3: '32', reg4: '32'
    },
    custom2: {
        name: '预设组 B',
        badge: '自定义 B: 用户记忆组 B',
        icon: '💾',
        reg1: '48', reg2: '48', reg3: '48', reg4: '48'
    },
    custom3: {
        name: '预设组 C',
        badge: '自定义 C: 用户记忆组 C',
        icon: '💾',
        reg1: '20', reg2: '28', reg3: '36', reg4: '44'
    },
    custom4: {
        name: '预设组 D',
        badge: '自定义 D: 用户记忆组 D',
        icon: '💾',
        reg1: '16', reg2: '20', reg3: '24', reg4: '28'
    }
};
let activePresetKey = 'group1';

document.addEventListener('DOMContentLoaded', () => {
    initEvents();
    initSidebarResizer();
    initLogResizer();
    initRecommendedPresets();
    updateErtHexPreview();
    refreshPortList();
    startPolling();
    handleWindowResize();
    autoFitLeftPanelHeight('power_pts');
});

function initEvents() {
    const chkAuto = document.getElementById('chkAutoScroll');
    if (chkAuto) {
        chkAuto.checked = false;
        autoScroll = false;
        chkAuto.addEventListener('change', (e) => {
            autoScroll = e.target.checked;
        });
    }
}

// ========================================================
// 侧边栏展开/折叠功能 (Sidebar Collapse / Expand)
// ========================================================
let isSidebarCollapsed = false;

function toggleSidebarCollapse(event) {
    if (event) {
        event.stopPropagation();
    }
    const container = document.querySelector('.main-container');
    const hdrBtn = document.getElementById('btnHdrSidebarToggle');
    const hdrTxt = document.getElementById('hdrSidebarBtnText');
    const resizerIcon = document.getElementById('resizerToggleIcon');
    if (!container) return;

    isSidebarCollapsed = !isSidebarCollapsed;
    container.classList.toggle('sidebar-collapsed', isSidebarCollapsed);

    if (hdrBtn) {
        hdrBtn.classList.toggle('collapsed', isSidebarCollapsed);
    }
    if (hdrTxt) {
        hdrTxt.textContent = isSidebarCollapsed ? '展开侧栏' : '收起侧栏';
    }
    if (resizerIcon) {
        resizerIcon.textContent = isSidebarCollapsed ? '▶' : '◀';
    }

    localStorage.setItem('GT2001_SIDEBAR_COLLAPSED', isSidebarCollapsed ? '1' : '0');
    showToast(isSidebarCollapsed ? '🖥️ 控制侧栏已折叠 · 全屏沉浸监视相控阵面 (点击边缘▶或按Alt+S展开)' : '📋 控制侧栏已展开');
}

function initSidebarResizer() {
    const resizer = document.getElementById('sidebarResizer');
    const sidebar = document.querySelector('.sidebar-panel');
    if (!resizer || !sidebar) return;

    // Load saved width from localStorage (default to 520px if not set)
    const savedWidth = localStorage.getItem('GT2001_SIDEBAR_WIDTH');
    if (savedWidth) {
        sidebar.style.width = `${Math.max(420, Math.min(750, parseInt(savedWidth, 10)))}px`;
    } else {
        sidebar.style.width = '520px';
    }

    // Restore saved collapse state
    const savedCollapsed = localStorage.getItem('GT2001_SIDEBAR_COLLAPSED');
    if (savedCollapsed === '1') {
        isSidebarCollapsed = false; // will be flipped to true in toggleSidebarCollapse
        toggleSidebarCollapse();
    }

    let isResizing = false;
    let startX = 0;
    let startWidth = 0;

    resizer.addEventListener('mousedown', (e) => {
        if (e.target.closest('.resizer-toggle-btn')) return; // 点中折叠按钮不触发拖拽
        if (isSidebarCollapsed) return; // 折叠状态下不拖拽
        isResizing = true;
        startX = e.clientX;
        startWidth = sidebar.offsetWidth;
        resizer.classList.add('resizing');
        sidebar.classList.add('is-resizing');
        document.body.style.cursor = 'col-resize';
        document.body.style.userSelect = 'none';
    });

    document.addEventListener('mousemove', (e) => {
        if (!isResizing || isSidebarCollapsed) return;
        const delta = e.clientX - startX;
        const newWidth = Math.max(400, Math.min(750, startWidth + delta));
        sidebar.style.width = `${newWidth}px`;
    });

    document.addEventListener('mouseup', () => {
        if (isResizing) {
            isResizing = false;
            resizer.classList.remove('resizing');
            sidebar.classList.remove('is-resizing');
            document.body.style.cursor = '';
            document.body.style.userSelect = '';
            localStorage.setItem('GT2001_SIDEBAR_WIDTH', sidebar.offsetWidth);
        }
    });

    // Double click to reset to comfortable default 520px
    resizer.addEventListener('dblclick', (e) => {
        if (e.target.closest('.resizer-toggle-btn')) return;
        if (isSidebarCollapsed) {
            toggleSidebarCollapse();
            return;
        }
        sidebar.style.width = '520px';
        localStorage.setItem('GT2001_SIDEBAR_WIDTH', '520');
        showToast('📏 控制面板已重置为舒适宽度 520px');
    });

    // Keyboard shortcut: Alt + S
    window.addEventListener('keydown', (e) => {
        if (e.altKey && (e.key === 's' || e.key === 'S')) {
            e.preventDefault();
            toggleSidebarCollapse();
        }
    });
}

// ========================================================
// 1. Port List & Connect Handling
// ========================================================
async function refreshPortList() {
    try {
        const res = await fetch('/api/ports');
        const data = await res.json();
        if (data.success) {
            populatePortSelect('selPortUart0', data.ports, 'COM1');
            populatePortSelect('selPortUart3', data.ports, 'COM4');
        }
    } catch (err) {
        console.error('Failed to fetch serial ports:', err);
    }
}

function populatePortSelect(elementId, ports, defaultVal) {
    const sel = document.getElementById(elementId);
    if (!sel) return;
    const currentVal = sel.value;
    sel.innerHTML = '';
    
    if (ports.length === 0) {
        const opt = document.createElement('option');
        opt.value = '';
        opt.textContent = '无可用串口';
        sel.appendChild(opt);
        return;
    }

    ports.forEach(p => {
        const opt = document.createElement('option');
        opt.value = p.device;
        opt.textContent = `${p.device} (${p.description})`;
        sel.appendChild(opt);
    });

    if (currentVal && Array.from(sel.options).some(o => o.value === currentVal)) {
        sel.value = currentVal;
    } else if (defaultVal && Array.from(sel.options).some(o => o.value === defaultVal)) {
        sel.value = defaultVal;
    }
}

async function toggleConnectUart(target) {
    if (!statusData) return;
    const uartInfo = statusData[target];
    const isConnected = uartInfo && uartInfo.status === 'connected';

    if (isConnected) {
        try {
            const res = await fetch('/api/disconnect', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ target: target })
            });
            const data = await res.json();
            if (!data.success) alert(data.msg);
        } catch (err) {
            alert('断开连接失败: ' + err.message);
        }
    } else {
        const portSel = document.getElementById(target === 'uart0' ? 'selPortUart0' : 'selPortUart3');
        const baudSel = document.getElementById(target === 'uart0' ? 'selBaudUart0' : 'selBaudUart3');
        const port = portSel ? portSel.value : '';
        const defaultBaud = target === 'uart3' ? '921600' : '115200';
        const baud = baudSel ? baudSel.value : defaultBaud;

        if (!port) {
            alert('请选择串口号！');
            return;
        }

        try {
            const res = await fetch('/api/connect', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ target: target, port: port, baud: baud })
            });
            const data = await res.json();
            if (!data.success) alert(data.msg);
        } catch (err) {
            alert('连接串口异常: ' + err.message);
        }
    }
}

// ========================================================
// 2. Command Center Tabs & Actions
// ========================================================
function switchCmdTab(tabName) {
    activeCmdTab = tabName;
    const tabBtns = document.querySelectorAll('.cmd-tab-btn');
    tabBtns.forEach(btn => {
        const clickAttr = btn.getAttribute('onclick') || '';
        btn.classList.toggle('active', clickAttr.includes(`'${tabName}'`));
    });

    const contents = document.querySelectorAll('.cmd-tab-content');
    contents.forEach(content => {
        content.classList.toggle('active', content.id === `tabContent_${tabName}`);
    });

    if (tabName === 'ert') {
        updateErtHexPreview();
    }

    // 用户切换窗口时，清除临时手动调整标记，触发最优自适应几何布局
    userCustomLogHeight = null;

    // 立即执行自适应尺寸调整
    autoFitLeftPanelHeight(tabName);

    // 下一帧再次校准（消除不同字体或复杂控件初次渲染导致的微小高度延迟）
    requestAnimationFrame(() => {
        autoFitLeftPanelHeight(tabName);
    });
}

/**
 * 智能自适应左侧控制面板与日志区高度（核心优化算法）：
 * 1. 确保顶部双串口（UART0/UART3）与当前激活的指令窗口（电源、波束指向、TR寄存器等）100%完整可见且不截断；
 * 2. 消除控制卡片下方的无效空白，回读日志区精准吸收全部剩余垂直高度（零留白）；
 * 3. 当窗口在极端矮屏幕下时，保护日志区最小高度，并优雅开启平滑滚动。
 */
function autoFitLeftPanelHeight(tabName) {
    tabName = tabName || activeCmdTab || 'power_pts';
    const sidebar = document.querySelector('.sidebar-panel');
    const topSection = document.getElementById('sidebarTopSection');
    const logCard = document.getElementById('mainLogCard');
    const resizer = document.getElementById('logResizer');
    const activeContent = document.getElementById(`tabContent_${tabName}`);
    if (!sidebar || !topSection || !logCard || !activeContent) return;

    if (sidebar.classList.contains('log-maximized')) return;

    // 若当前处于纯控制台模式（左右双栏并排：左侧控制，右侧日志），高度各自撑满 100%
    const mainContainer = document.querySelector('.main-container');
    const isCompact = mainContainer && (mainContainer.classList.contains('compact-mode') || window.innerWidth < 1080);
    if (isCompact) {
        topSection.style.height = '100%';
        topSection.style.maxHeight = '100%';
        topSection.style.flex = '0 0 540px';
        topSection.style.overflowY = 'auto';

        logCard.style.height = '100%';
        logCard.style.maxHeight = '100%';
        logCard.style.flex = '1 1 0%';
        return;
    }

    // 清除手动拖拽临时锁死，每次切换均回归当前窗口专属最优尺寸
    userCustomLogHeight = null;

    // 1. 获取串口卡片与指令卡片的真实内容高度
    const serialCard = topSection.querySelector('.serial-card') || topSection.querySelector('.anchor-card-uart');
    const cmdHeader = topSection.querySelector('.cmd-tabs-header');
    
    // 串口卡片物理高度（严格保证 UART0 与 UART3 绝不被压缩裁切，实测至少 118px）
    const serialH = serialCard ? Math.max(serialCard.scrollHeight, 118) : 118;
    const headerH = cmdHeader ? cmdHeader.offsetHeight : 34;
    const contentH = activeContent.scrollHeight;

    // 上部区域所需总高度（串口卡片 + 卡片gap(6px) + 指令Tab头 + 激活Tab内容 + 内外边距补正(14px)）
    const neededTopH = serialH + 6 + headerH + contentH + 14;

    // 2. 侧边栏总可用内部净高度
    const sidebarStyle = window.getComputedStyle(sidebar);
    const sidePadY = (parseFloat(sidebarStyle.paddingTop) || 0) + (parseFloat(sidebarStyle.paddingBottom) || 0);
    const totalSidebarH = sidebar.clientHeight - (sidePadY || 18);

    // 分割手柄占用垂直高度
    const resizerH = 16;

    // 3. 计算日志卡片分配高度
    const minLogH = 90; // 保证日志区至少 90px，显示标题和实时最新数据
    let targetLogH = totalSidebarH - neededTopH - resizerH;

    if (targetLogH >= minLogH) {
        // 【最优全显模式】：空间充足，上方精准包裹所有内容（两串口与当前Tab全部控件），不出现滚动条；
        // 下方日志区吸收全部剩余垂直空间，紧密贴合，零多余留白！
        topSection.style.height = `${neededTopH}px`;
        topSection.style.maxHeight = `${neededTopH}px`;
        topSection.style.flex = `0 0 ${neededTopH}px`;
        topSection.style.overflowY = 'hidden';

        const finalLogH = Math.floor(targetLogH);
        logCard.style.height = `${finalLogH}px`;
        logCard.style.maxHeight = 'none';
        logCard.style.flex = `1 1 ${finalLogH}px`;
    } else {
        // 【矮屏幕自适应模式】：窗口较矮或内容较多时，保证日志 minLogH，上方自适应剩余高度并开启平滑滚动
        const adjustedTopH = Math.max(140, totalSidebarH - minLogH - resizerH);
        topSection.style.height = `${adjustedTopH}px`;
        topSection.style.maxHeight = `${adjustedTopH}px`;
        topSection.style.flex = `0 0 ${adjustedTopH}px`;
        topSection.style.overflowY = 'auto';

        logCard.style.height = `${minLogH}px`;
        logCard.style.maxHeight = `${minLogH}px`;
        logCard.style.flex = `0 0 ${minLogH}px`;
    }

    // 重置上方滚动条到最顶端，保证两个串口始终第一眼可见
    topSection.scrollTop = 0;
}

// Array Power Command (上下电)
function sendPowerCmd(action) {
    let targetDir = 15; // 0x0F default (Both)
    const radios = document.getElementsByName('pwrTarget');
    for (let r of radios) {
        if (r.checked) {
            targetDir = parseInt(r.value);
            break;
        }
    }

    sendCommand({
        type: 'POWER',
        dir: targetDir,
        action: action
    });
}

// Whole-Array Channel Control (发射/接收通道全开全关 - FCODE 0x05)
// 自带复位防呆：先下发 0x01 RESET 复位阵面芯片，再下发通道全开/全关指令
async function sendChannelAllCmd(target, action) {
    const actStr = action === 1 ? '全开' : '全关';
    const tgtStr = target === 'TX' ? '发射' : '接收';
    
    showToast(`🔄 [防呆复位] 正在下发阵面复位 (0x01 RESET)...`);
    try {
        await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ type: 'RESET' })
        });
    } catch (e) {
        console.warn('Reset pre-step warning:', e);
    }

    await new Promise(r => setTimeout(r, 80));

    showToast(`⚡ [通道配置] 正在执行 ${tgtStr} 通道 ${actStr} (FCODE 0x05)...`);
    sendCommand({
        type: 'CHANNEL_ALL',
        target: target,
        action: action
    });
    showToast(`✅ [完成] ${tgtStr} 通道 ${actStr} 成功 (已自动完成芯片复位防呆)`);
}

// PTS Pointing Mode Command (自动/手动对星)
function sendPtsCmd(mode) {
    sendCommand({ type: 'PTS', mode: mode });
}

// ATS Manual Beam Command
// 波束更新指令下发流程 (移除复位命令，避免冲掉工作寄存器配置)
// [1/2 阵面上电] ➔ [2/2 下发波束指向与通道全开点亮]
async function sendAtsCmd() {
    const pitch = parseFloat(document.getElementById('inpPitch').value) || 0;
    const yaw = parseFloat(document.getElementById('inpYaw').value) || 0;
    const polr = parseFloat(document.getElementById('inpPolR').value) || 0;
    const polt = parseFloat(document.getElementById('inpPolT').value) || 0;
    const freq_r = parseFloat(document.getElementById('inpFreqR').value) || 12250;
    const freq_t = parseFloat(document.getElementById('inpFreqT').value) || 14250;
    const adcl = parseInt(document.getElementById('inpAdcl').value) || 0;

    // 1. 自动步骤 1/2: 阵面总电源上电 (POWER ON, ALL 0x0F)
    showToast('⚡ [波束更新 1/2] 正在阵面上电 (POWER ON)...');
    try {
        await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ type: 'POWER', dir: 15, action: 1 })
        });
    } catch (e) {
        console.warn('Beam auto power on error:', e);
    }

    await new Promise(r => setTimeout(r, 60));

    // 2. 自动步骤 2/2: 波束更新时，默认所有通道都处于开启状态，直接全亮
    if (typeof statusData !== 'undefined' && statusData && statusData.registers) {
        for (let d of ['TX', 'RX']) {
            for (let cid = 0; cid < 16; cid++) {
                for (let ch = 0; ch < 8; ch++) {
                    if (statusData.registers[d]?.[cid]?.[ch]) {
                        statusData.registers[d][cid][ch].is_open = true;
                    }
                }
            }
        }
        if (typeof renderMatrix === 'function') {
            renderMatrix(statusData);
        }
    }

    showToast('📡 [波束更新 2/2] 正在下发手动波束指向指令 ($xxATS)...');
    sendCommand({
        type: 'ATS',
        pitch: pitch,
        yaw: yaw,
        polr: polr,
        polt: polt,
        freq_r: freq_r,
        freq_t: freq_t,
        adcl: adcl
    });
    showToast(`✅ 波束更新成功！(已下发波束指向指令)`);
}


function applyAtsPreset(pitch, yaw) {
    document.getElementById('inpPitch').value = pitch.toFixed(1);
    document.getElementById('inpYaw').value = yaw.toFixed(1);
    sendAtsCmd();
}

// ========================================================
// 2.0 TR Registers & Array Bias Profile Setting ($xxREG / $xxBIP)
// ========================================================
let selectedTargetChipId = 16; // 16 = 全阵面广播

function sendRegCmd() {
    const reg1 = document.getElementById('inpReg1').value.trim();
    const reg2 = document.getElementById('inpReg2').value.trim();
    const reg3 = document.getElementById('inpReg3').value.trim();
    const reg4 = document.getElementById('inpReg4').value.trim();
    
    let dir = 0;
    const dirRadios = document.getElementsByName('regDir');
    for (let r of dirRadios) {
        if (r.checked) {
            dir = parseInt(r.value);
            break;
        }
    }

    const sel = document.getElementById('selRegTargetChip');
    const chip = sel ? parseInt(sel.value) : 16;
    const dirStr = dir === 1 ? 'TX' : 'RX';

    sendCommand({
        type: 'REG',
        chip: chip,
        reg1: reg1,
        reg2: reg2,
        reg3: reg3,
        reg4: reg4,
        dir: dir
    });

    const targetDesc = chip < 16 ? `CHIP ${String(chip).padStart(2, '0')}` : '全阵面广播';
    showToast(`💾 正在写入 ${targetDesc} [${dirStr}] 寄存器: (${reg1}, ${reg2}, ${reg3}, ${reg4})...`);
}

// BIP Bias Profile Command ($xxBIP,profile)
async function sendBipCmd(profile) {
    const pNames = ['P0 标称均衡 (48 16 32 32 全阵)', 'P1 中心增强 (中心强增益/外围高敏)', 'P2 极限高敏 (48 24 36 32 全阵)'];
    const pName = pNames[profile] || `剖面 ${profile}`;
    showToast(`⚡ 正在生效偏置剖面 ($xxBIP,${profile}) · ${pName}...`);
    try {
        const res = await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                type: 'BIP',
                profile: profile
            })
        });
        const d = await res.json();
        if (d.success) {
            showToast(`✅ [已生效] ${pName} 下发成功！`);
            // 根据剖面智能同步前端显示输入框
            if (profile === 0) {
                setRegInputValues('48', '16', '32', '32', true);
            } else if (profile === 2) {
                setRegInputValues('48', '24', '36', '32', true);
            }
        } else {
            showToast(`❌ 下发失败: ${d.msg}`, 'error');
        }
    } catch (e) {
        showToast(`❌ 通信请求失败: ${e.message}`, 'error');
    }
}

// 目标芯片选择联动控制
function selectChipForReg(chipId) {
    selectedTargetChipId = chipId;
    const sel = document.getElementById('selRegTargetChip');
    if (sel) {
        sel.value = String(chipId);
    }
    updateTargetChipUI();
    switchCmdTab('reg');
    const chipPad = String(chipId).padStart(2, '0');
    const isCenter = [5, 6, 9, 10].includes(chipId);
    const posDesc = isCenter ? '⭐ 中心核心阵元' : '🛡️ 外围边缘阵元';
    showToast(`🎯 已联动选中 CHIP ${chipPad} [${posDesc}]，可单独调节寄存器`);
}

function onTargetChipChanged() {
    const sel = document.getElementById('selRegTargetChip');
    if (!sel) return;
    selectedTargetChipId = parseInt(sel.value);
    updateTargetChipUI();
}

function setTargetChipQuick(chipId) {
    selectedTargetChipId = chipId;
    const sel = document.getElementById('selRegTargetChip');
    if (sel) {
        sel.value = String(chipId);
    }
    updateTargetChipUI();
}

function updateTargetChipUI() {
    const btnAll = document.getElementById('btnChipTargetAll');
    const btnCenter = document.getElementById('btnChipTargetCenter');
    const btnPeriphery = document.getElementById('btnChipTargetPeriphery');
    if (btnAll) btnAll.classList.toggle('active', selectedTargetChipId === 16);
    if (btnCenter) btnCenter.classList.toggle('active', false);
    if (btnPeriphery) btnPeriphery.classList.toggle('active', false);

    // 4x4 阵列高亮当前选中的芯片
    document.querySelectorAll('.chip-card').forEach(card => {
        const cId = parseInt(card.dataset.chip);
        card.classList.toggle('chip-card-selected', selectedTargetChipId < 16 && cId === selectedTargetChipId);
    });

    // 动态刷新写入按钮文字
    const btnSend = document.getElementById('btnSendReg');
    if (btnSend) {
        if (selectedTargetChipId < 16) {
            const pad = String(selectedTargetChipId).padStart(2, '0');
            btnSend.innerHTML = `💾 写入 CHIP ${pad} 单片寄存器 ($xxREG,${selectedTargetChipId},...)`;
        } else {
            btnSend.innerHTML = `💾 写入全阵面 TR 寄存器并校验回读 ($xxREG 全广播)`;
        }
    }
}

// 批量快速配置中心4片 / 外围12片
async function batchSetCenterChips() {
    const r1 = document.getElementById('inpReg1').value.trim();
    const r2 = document.getElementById('inpReg2').value.trim();
    const r3 = document.getElementById('inpReg3').value.trim();
    const r4 = document.getElementById('inpReg4').value.trim();
    let dir = 0;
    const dirRadios = document.getElementsByName('regDir');
    for (let r of dirRadios) { if (r.checked) { dir = parseInt(r.value); break; } }
    
    showToast('🚀 正在批量下发中心 4 颗芯片 (05, 06, 09, 10)...');
    const centerChips = [5, 6, 9, 10];
    for (let cid of centerChips) {
        await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                type: 'REG',
                chip: cid,
                reg1: r1, reg2: r2, reg3: r3, reg4: r4,
                dir: dir
            })
        });
        await new Promise(r => setTimeout(r, 60));
    }
    showToast(`✅ 中心 4 颗芯片偏置配置完成: (${r1}, ${r2}, ${r3}, ${r4})`);
}

async function batchSetPeripheryChips() {
    const r1 = document.getElementById('inpReg1').value.trim();
    const r2 = document.getElementById('inpReg2').value.trim();
    const r3 = document.getElementById('inpReg3').value.trim();
    const r4 = document.getElementById('inpReg4').value.trim();
    let dir = 0;
    const dirRadios = document.getElementsByName('regDir');
    for (let r of dirRadios) { if (r.checked) { dir = parseInt(r.value); break; } }
    
    showToast('🛡️ 正在批量下发外围 12 颗芯片...');
    const peripheryChips = [0, 1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 15];
    for (let cid of peripheryChips) {
        await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                type: 'REG',
                chip: cid,
                reg1: r1, reg2: r2, reg3: r3, reg4: r4,
                dir: dir
            })
        });
        await new Promise(r => setTimeout(r, 50));
    }
    showToast(`✅ 外围 12 颗芯片偏置配置完成: (${r1}, ${r2}, ${r3}, ${r4})`);
}

// 动态功能引脚标签显示 (RX: LNA1/LNA2 vs TX: PD/PA)
function updateRegFnLabels() {
    let dir = 0;
    const dirRadios = document.getElementsByName('regDir');
    for (let r of dirRadios) {
        if (r.checked) { dir = parseInt(r.value); break; }
    }
    const lbl1 = document.getElementById('regFn1');
    const lbl2 = document.getElementById('regFn2');
    const lbl3 = document.getElementById('regFn3');
    const lbl4 = document.getElementById('regFn4');
    if (dir === 0) { // RX
        if (lbl1) lbl1.textContent = 'LNA1';
        if (lbl2) lbl2.textContent = 'LNA2';
        if (lbl3) lbl3.textContent = 'AMP1';
        if (lbl4) lbl4.textContent = 'AMP2';
    } else { // TX
        if (lbl1) lbl1.textContent = 'PD';
        if (lbl2) lbl2.textContent = 'PA';
        if (lbl3) lbl3.textContent = 'AMP1';
        if (lbl4) lbl4.textContent = 'AMP2';
    }
}

// ========================================================
// 2.1 TR Recommended Presets Management (8 Groups: 4 Recommended + 4 Custom)
// ========================================================
// 2.1 TR Recommended Presets Management (8 Groups: 4 Recommended + 4 Custom)
// ========================================================
async function initRecommendedPresets() {
    let loadedFromBackend = false;
    let targetKey = 'group3';

    // 1. 优先从后端磁盘读取永久保存的 config_presets.json 配置文件
    try {
        const resp = await fetch('/api/presets');
        if (resp.ok) {
            const json = await resp.json();
            if (json.success && json.presets) {
                const p = json.presets;
                ['custom1', 'custom2', 'custom3', 'custom4'].forEach(k => {
                    if (p[k]) {
                        TR_PRESET_DEFINITIONS[k].reg1 = String(p[k].reg1 ?? TR_PRESET_DEFINITIONS[k].reg1);
                        TR_PRESET_DEFINITIONS[k].reg2 = String(p[k].reg2 ?? TR_PRESET_DEFINITIONS[k].reg2);
                        TR_PRESET_DEFINITIONS[k].reg3 = String(p[k].reg3 ?? TR_PRESET_DEFINITIONS[k].reg3);
                        TR_PRESET_DEFINITIONS[k].reg4 = String(p[k].reg4 ?? TR_PRESET_DEFINITIONS[k].reg4);
                    }
                });
                if (p.active_preset_key && TR_PRESET_DEFINITIONS[p.active_preset_key]) {
                    targetKey = p.active_preset_key;
                }
                loadedFromBackend = true;
                // 同步镜像缓存至本地 localStorage 作为容灾双保险
                try {
                    localStorage.setItem('GT2001_TR_CUSTOM_PRESETS_V3', JSON.stringify(p));
                    localStorage.setItem('GT2001_ACTIVE_PRESET_KEY', targetKey);
                } catch (e) {}
            }
        }
    } catch (e) {
        console.warn('Backend presets fetch error:', e);
    }

    // 2. 若后端因网络未就绪，使用浏览器本地 localStorage 容灾回退
    if (!loadedFromBackend) {
        try {
            const raw = localStorage.getItem('GT2001_TR_CUSTOM_PRESETS_V3');
            if (raw) {
                const parsed = JSON.parse(raw);
                ['custom1', 'custom2', 'custom3', 'custom4'].forEach(k => {
                    if (parsed[k]) {
                        TR_PRESET_DEFINITIONS[k].reg1 = String(parsed[k].reg1 ?? TR_PRESET_DEFINITIONS[k].reg1);
                        TR_PRESET_DEFINITIONS[k].reg2 = String(parsed[k].reg2 ?? TR_PRESET_DEFINITIONS[k].reg2);
                        TR_PRESET_DEFINITIONS[k].reg3 = String(parsed[k].reg3 ?? TR_PRESET_DEFINITIONS[k].reg3);
                        TR_PRESET_DEFINITIONS[k].reg4 = String(parsed[k].reg4 ?? TR_PRESET_DEFINITIONS[k].reg4);
                    }
                });
            }
            const savedKey = localStorage.getItem('GT2001_ACTIVE_PRESET_KEY');
            if (savedKey && TR_PRESET_DEFINITIONS[savedKey]) {
                targetKey = savedKey;
            }
        } catch (e) {
            console.warn('Failed to load custom presets from localStorage:', e);
        }
    }

    updateCustomCardDisplays();
    applyPresetGroup(targetKey, false);
    updateRegFnLabels();
    updateTargetChipUI();
}

function updateCustomCardDisplays() {
    const slots = [
        { key: 'custom1', id: 'lblCustomVals1' },
        { key: 'custom2', id: 'lblCustomVals2' },
        { key: 'custom3', id: 'lblCustomVals3' },
        { key: 'custom4', id: 'lblCustomVals4' }
    ];
    slots.forEach(s => {
        const lbl = document.getElementById(s.id);
        if (lbl) {
            const c = TR_PRESET_DEFINITIONS[s.key];
            lbl.textContent = `${c.reg1},${c.reg2},${c.reg3},${c.reg4}`;
        }
    });
}

function applyPresetGroup(groupKey, animate = true) {
    if (!TR_PRESET_DEFINITIONS[groupKey]) groupKey = 'group1';
    activePresetKey = groupKey;
    try {
        localStorage.setItem('GT2001_ACTIVE_PRESET_KEY', groupKey);
    } catch (e) {}

    // 异步同步最近选定的预设至后端磁盘文件
    fetch('/api/presets', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ active_preset_key: groupKey })
    }).catch(() => {});

    const btnIdMap = {
        group1: 'btnPresetGroup1',
        group2: 'btnPresetGroup2',
        group3: 'btnPresetGroup3',
        group4: 'btnPresetGroup4',
        custom1: 'btnPresetCustom1',
        custom2: 'btnPresetCustom2',
        custom3: 'btnPresetCustom3',
        custom4: 'btnPresetCustom4'
    };

    Object.keys(btnIdMap).forEach(k => {
        const el = document.getElementById(btnIdMap[k]);
        if (el) el.classList.toggle('active', k === groupKey);
    });

    const p = TR_PRESET_DEFINITIONS[groupKey];
    setRegInputValues(p.reg1, p.reg2, p.reg3, p.reg4, animate);

    const badge = document.getElementById('badgeActivePreset');
    if (badge) badge.textContent = p.badge;

    const descBanner = document.getElementById('presetDescBanner');
    const descContent = document.getElementById('presetDescContent');
    if (descBanner) {
        if (groupKey.startsWith('custom')) {
            descBanner.style.display = 'none';
        } else {
            descBanner.style.display = 'flex';
            if (descContent && p.desc) {
                descContent.innerHTML = `<strong>[${escapeHtml(p.descTag || '')}]</strong> ${escapeHtml(p.desc)}`;
            }
        }
    }

    updateHexTags();
}

async function saveCurrentAsCustomSlot(slotKey) {
    if (!TR_PRESET_DEFINITIONS[slotKey]) slotKey = 'custom1';
    const r1 = parseRegVal(document.getElementById('inpReg1').value);
    const r2 = parseRegVal(document.getElementById('inpReg2').value);
    const r3 = parseRegVal(document.getElementById('inpReg3').value);
    const r4 = parseRegVal(document.getElementById('inpReg4').value);

    TR_PRESET_DEFINITIONS[slotKey].reg1 = String(r1);
    TR_PRESET_DEFINITIONS[slotKey].reg2 = String(r2);
    TR_PRESET_DEFINITIONS[slotKey].reg3 = String(r3);
    TR_PRESET_DEFINITIONS[slotKey].reg4 = String(r4);

    const storedObj = {};
    ['custom1', 'custom2', 'custom3', 'custom4'].forEach(k => {
        storedObj[k] = {
            reg1: TR_PRESET_DEFINITIONS[k].reg1,
            reg2: TR_PRESET_DEFINITIONS[k].reg2,
            reg3: TR_PRESET_DEFINITIONS[k].reg3,
            reg4: TR_PRESET_DEFINITIONS[k].reg4
        };
    });
    storedObj.active_preset_key = slotKey;

    // 1. 立即更新界面与客户端双缓存
    updateCustomCardDisplays();
    applyPresetGroup(slotKey, true);
    try {
        localStorage.setItem('GT2001_TR_CUSTOM_PRESETS_V3', JSON.stringify(storedObj));
        localStorage.setItem('GT2001_ACTIVE_PRESET_KEY', slotKey);
    } catch (e) {}

    const slotName = slotKey === 'custom1' ? 'A' : (slotKey === 'custom2' ? 'B' : (slotKey === 'custom3' ? 'C' : 'D'));

    // 2. 核心：向 Python 后端发送 POST 请求，直接写入本地磁盘 config_presets.json 文件！
    try {
        const resp = await fetch('/api/presets', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(storedObj)
        });
        const res = await resp.json();
        if (res.success) {
            showToast(`💾 [永久保存] 自定义组 ${slotName} 已存入本地磁盘文件，重启软件永久保留: (${r1}, ${r2}, ${r3}, ${r4})`);
        } else {
            showToast(`✅ 已存入【自定义组 ${slotName}】: REG=(${r1}, ${r2}, ${r3}, ${r4})`);
        }
    } catch (e) {
        showToast(`✅ 已存入【自定义组 ${slotName}】: REG=(${r1}, ${r2}, ${r3}, ${r4})`);
    }
}

function saveCurrentToActiveCustom() {
    if (activePresetKey.startsWith('custom')) {
        saveCurrentAsCustomSlot(activePresetKey);
    } else {
        saveCurrentAsCustomSlot('custom1');
    }
}

function parseRegVal(v) {
    if (typeof v === 'string') {
        v = v.trim();
        if (v.startsWith('0x') || v.startsWith('0X')) {
            const parsed = parseInt(v, 16);
            return isNaN(parsed) ? 32 : Math.max(0, Math.min(255, parsed));
        }
    }
    const num = parseInt(v, 10);
    return isNaN(num) ? 32 : Math.max(0, Math.min(255, num));
}

function formatHex(val) {
    const num = parseRegVal(val);
    return '0x' + num.toString(16).padStart(2, '0').toUpperCase();
}

function updateHexTags() {
    for (let i = 1; i <= 4; i++) {
        const inp = document.getElementById(`inpReg${i}`);
        const tag = document.getElementById(`hexTag${i}`);
        if (inp && tag) {
            tag.textContent = formatHex(inp.value);
        }
    }
}

function onRegInputChanged(regIdx) {
    updateHexTags();
    const badge = document.getElementById('badgeActivePreset');
    if (badge && !badge.textContent.includes('(手动)')) {
        badge.textContent += ' (手动)';
    }
}

function stepRegValue(inputId, delta) {
    const el = document.getElementById(inputId);
    if (!el) return;
    let current = parseRegVal(el.value);
    current = Math.max(0, Math.min(255, current + delta));
    el.value = current;
    updateHexTags();
    el.classList.remove('flash-active');
    void el.offsetWidth;
    el.classList.add('flash-active');
    const badge = document.getElementById('badgeActivePreset');
    if (badge && !badge.textContent.includes('(手动)')) {
        badge.textContent += ' (手动)';
    }
}

function setAllRegs(val) {
    setRegInputValues(val, val, val, val, true);
    updateHexTags();
    const badge = document.getElementById('badgeActivePreset');
    if (badge) badge.textContent = `全 ${val} 设定`;
}

function randomizeRegs() {
    const r1 = Math.floor(Math.random() * 64);
    const r2 = Math.floor(Math.random() * 64);
    const r3 = Math.floor(Math.random() * 64);
    const r4 = Math.floor(Math.random() * 64);
    setRegInputValues(r1, r2, r3, r4, true);
    updateHexTags();
    const badge = document.getElementById('badgeActivePreset');
    if (badge) badge.textContent = `随机生成 (${r1},${r2},${r3},${r4})`;
    showToast(`🎲 随机 0~63 寄存器设定: [${r1}, ${r2}, ${r3}, ${r4}]`);
}

function setRegInputValues(r1, r2, r3, r4, animate) {
    const fields = [
        { id: 'inpReg1', val: r1 },
        { id: 'inpReg2', val: r2 },
        { id: 'inpReg3', val: r3 },
        { id: 'inpReg4', val: r4 }
    ];

    fields.forEach(f => {
        const el = document.getElementById(f.id);
        if (el) {
            el.value = f.val;
            if (animate) {
                el.classList.remove('flash-active');
                void el.offsetWidth;
                el.classList.add('flash-active');
            }
        }
    });
}

function showToast(msg) {
    let t = document.getElementById('appToast');
    if (!t) {
        t = document.createElement('div');
        t.id = 'appToast';
        t.className = 'app-toast';
        document.body.appendChild(t);
    }
    t.textContent = msg;
    t.classList.add('show');
    clearTimeout(t._timer);
    t._timer = setTimeout(() => {
        t.classList.remove('show');
    }, 2800);
}

// ========================================================
// 2.2 Custom Command Sending (ASCII & HEX)
// ========================================================
function switchCustomMode(mode) {
    const secAscii = document.getElementById('secCustomAscii');
    const secHex = document.getElementById('secCustomHex');
    if (secAscii && secHex) {
        secAscii.style.display = mode === 'ascii' ? 'block' : 'none';
        secHex.style.display = mode === 'hex' ? 'block' : 'none';
    }
    autoFitLeftPanelHeight('raw');
}

function sendRawCmd() {
    const rawInp = document.getElementById('inpRawCmd');
    const cmd = rawInp.value.trim();
    if (!cmd) return;
    sendCommand({ type: 'RAW', command: cmd });
    rawInp.value = '';
}

function insertRawTemplate(tmpl) {
    const rawInp = document.getElementById('inpRawCmd');
    if (rawInp) {
        rawInp.value = tmpl;
        rawInp.focus();
    }
}

function insertHexTemplate(hexStr) {
    const hexInp = document.getElementById('inpHexCmd');
    if (hexInp) {
        hexInp.value = hexStr;
        hexInp.focus();
        onHexInputChange();
    }
}

function onHexInputChange() {
    const hexInp = document.getElementById('inpHexCmd');
    const previewBox = document.getElementById('hexPreviewBox');
    const chkCrc = document.getElementById('chkAutoCrc16');
    if (!hexInp || !previewBox) return;

    let raw = hexInp.value.replace(/[^0-9A-Fa-f]/g, '');
    if (raw.length % 2 !== 0) {
        previewBox.textContent = `输入中: ${raw.length} 个半字节 (需为偶数)`;
        previewBox.style.color = '#eab308';
        return;
    }

    const byteCount = raw.length / 2;
    if (byteCount === 0) {
        previewBox.textContent = '准备发送: 0 字节';
        previewBox.style.color = '#64748b';
        return;
    }

    const bytes = [];
    for (let i = 0; i < raw.length; i += 2) {
        bytes.push(parseInt(raw.substr(i, 2), 16));
    }

    const formatted = bytes.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
    const autoCrc = chkCrc ? chkCrc.checked : false;

    if (autoCrc) {
        const crc = calcCrc16Ccitt(bytes);
        const crcHi = (crc >> 8) & 0xFF;
        const crcLo = crc & 0xFF;
        const crcStr = `${crcHi.toString(16).padStart(2, '0').toUpperCase()} ${crcLo.toString(16).padStart(2, '0').toUpperCase()}`;
        previewBox.textContent = `待发帧: ${formatted} [CRC16: ${crcStr}] (共 ${byteCount + 2} 字节)`;
    } else {
        previewBox.textContent = `待发帧: ${formatted} (共 ${byteCount} 字节)`;
    }
    previewBox.style.color = '#059669';
}

function calcCrc16Ccitt(bytes) {
    let crc = 0xFFFF;
    for (let b of bytes) {
        crc ^= (b << 8);
        for (let i = 0; i < 8; i++) {
            if ((crc & 0x8000) !== 0) {
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF;
            } else {
                crc = (crc << 1) & 0xFFFF;
            }
        }
    }
    return crc;
}

function sendHexCmd() {
    const hexInp = document.getElementById('inpHexCmd');
    const chkCrc = document.getElementById('chkAutoCrc16');
    if (!hexInp) return;

    let raw = hexInp.value.replace(/[^0-9A-Fa-f]/g, '');
    if (!raw) {
        alert('请输入有效的十六进制内容！');
        return;
    }
    if (raw.length % 2 !== 0) {
        alert('十六进制字符数量必须为偶数！');
        return;
    }

    sendCommand({
        type: 'HEX',
        hex: raw,
        auto_crc: chkCrc ? chkCrc.checked : false
    });
}

function triggerArrayDump() {
    sendCommand({
        type: 'DUMP',
        dir: currentArray,
        pitch: parseFloat(document.getElementById('inpPitch')?.value || 0) || 0,
        yaw: parseFloat(document.getElementById('inpYaw')?.value || 0) || 0
    });
}

async function resetAllRegisters() {
    showToast('🔄 正在下发阵面复位指令 (0x01 RESET)，正在重置所有寄存器...');
    try {
        const res = await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ type: 'RESET' })
        });
        const data = await res.json();
        if (data.success) {
            if (typeof pollStatus === 'function') {
                await pollStatus();
            }
            showToast('✅ 阵面复位成功：所有 128 通道寄存器已恢复至初始化未读状态');
        } else {
            showToast(`❌ 复位失败: ${data.msg || '未知错误'}`);
        }
    } catch (err) {
        showToast(`❌ 复位网络请求失败: ${err.message}`);
    }
}

// ========================================================
// 2.3 标校单点误差表清零 (ERT Clear / FCODE 0x08)
// ========================================================
function setErtFreqPreset(freqGhz) {
    const el = document.getElementById('inpErtFreq');
    if (el) {
        el.value = freqGhz.toFixed(3);
        updateErtHexPreview();
    }
}

function updateErtHexPreview() {
    const pitchInp = document.getElementById('inpErtPitch');
    const yawInp = document.getElementById('inpErtYaw');
    const freqInp = document.getElementById('inpErtFreq');
    const poleSel = document.getElementById('selErtPole');
    const previewBox = document.getElementById('ertHexPreviewBox');
    if (!pitchInp || !yawInp || !freqInp || !poleSel || !previewBox) return;

    const pitch = parseFloat(pitchInp.value) || 0.0;
    const yaw = parseFloat(yawInp.value) || 0.0;
    const freq = parseFloat(freqInp.value) || 12.250;
    const pole = parseInt(poleSel.value) || 0;
    const tabType = 0x02; // Default single-point

    // Build 15-byte payload matching user_atcl.c:
    const bytes = [];
    bytes[0] = 0xAA;
    bytes[1] = 0x99;
    bytes[2] = 0x00;
    bytes[3] = 0x11; // Total length = 17
    bytes[4] = 0x08; // FCODE
    bytes[5] = tabType;
    bytes[6] = Math.floor(pitch) & 0xFF;
    bytes[7] = Math.round((pitch - Math.floor(pitch)) * 10) & 0xFF;
    bytes[8] = (Math.floor(yaw) >> 8) & 0xFF;
    bytes[9] = Math.floor(yaw) & 0xFF;
    bytes[10] = Math.round((yaw - Math.floor(yaw)) * 10) & 0xFF;
    bytes[11] = Math.floor(freq) & 0xFF;
    const fracFreq = Math.round((freq - Math.floor(freq)) * 1000);
    bytes[12] = (fracFreq >> 8) & 0xFF;
    bytes[13] = fracFreq & 0xFF;
    bytes[14] = pole & 0xFF;

    const crc = calcCrc16Ccitt(bytes);
    const crcHi = (crc >> 8) & 0xFF;
    const crcLo = crc & 0xFF;
    bytes.push(crcHi);
    bytes.push(crcLo);

    const hexStr = bytes.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
    previewBox.textContent = `待下发 17 字节: ${hexStr} (CRC16: 0x${crc.toString(16).padStart(4, '0').toUpperCase()})`;
}

function sendErtClearCmd(tabType) {
    const pitch = parseFloat(document.getElementById('inpErtPitch')?.value || 0) || 0;
    const yaw = parseFloat(document.getElementById('inpErtYaw')?.value || 0) || 0;
    const freq = parseFloat(document.getElementById('inpErtFreq')?.value || 12.25) || 12.25;
    const pole = parseInt(document.getElementById('selErtPole')?.value || 0) || 0;

    if (tabType === 3) {
        const ok = confirm("⚠️ 警告：全阵面误差表清空将擦除 Flash 内所有角度及频点的全部校准偏置数据！\n\n此操作不可逆，是否确认执行全阵面清空？");
        if (!ok) return;
    }

    sendCommand({
        type: 'ERT_CLEAR',
        tab_type: tabType,
        pitch: pitch,
        yaw: yaw,
        freq: freq,
        pole: pole
    });

    const desc = tabType === 2 ? '单点标校误差表清零' : '全阵面误差表清空';
    showToast(`🚀 已发送【${desc}】指令 (FCODE 0x08)`);
}

// Generic Dispatcher
async function sendCommand(payload) {
    try {
        const res = await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const data = await res.json();
        if (!data.success) {
            alert('指令发送失败: ' + data.msg);
        } else {
            // Instantly poll status right after sending for real-time responsiveness!
            pollStatus();
            setTimeout(pollStatus, 35);
        }
    } catch (err) {
        alert('发送网络错误: ' + err.message);
    }
}

// ========================================================
// 3. Array & View Mode Controls
// ========================================================
function switchArray(arr) {
    currentArray = arr;
    document.getElementById('btnTxArray').classList.toggle('active', arr === 'TX');
    document.getElementById('btnRxArray').classList.toggle('active', arr === 'RX');
    
    const titleEl = document.getElementById('currentArrayTitle');
    const subEl = document.getElementById('currentArraySubtitle');
    if (titleEl) {
        titleEl.textContent = arr === 'TX' ? 'TX 发射阵面' : 'RX 接收阵面';
    }
    if (subEl) {
        subEl.textContent = isDeductPortComp
            ? `✨ 去端口补偿模式 · 当前显示 ${arr} 阵面测试系统最初写入的理论初值 (已扣除微带走线固定偏置)`
            : (arr === 'TX' ? '16 芯片 · 128 通道高密度发射微带排布 · 物理回读原值' : '16 芯片 · 128 通道高密度接收微带排布 · 物理回读原值');
    }

    // 同步 Tab 3 中 TR 寄存器配置的 TX/RX 单选框
    const dirRadios = document.getElementsByName('regDir');
    for (let r of dirRadios) {
        if (arr === 'TX' && r.value === '1') r.checked = true;
        if (arr === 'RX' && r.value === '0') r.checked = true;
    }
    updateRegFnLabels();
    
    if (statusData) {
        renderMatrix(statusData);
    }
}

function setDisplayMode(mode) {
    displayMode = mode;
    document.getElementById('btnModeBoth').classList.toggle('active', mode === 'BOTH');
    document.getElementById('btnModePhase').classList.toggle('active', mode === 'PHASE');
    document.getElementById('btnModeAmp').classList.toggle('active', mode === 'AMP');

    if (statusData) {
        renderMatrix(statusData);
    }
}

// 切换去端口补偿显示状态 (显示测试系统最初写的值)
// 支持在【物理寄存器回读原值】与【去端口补偿显示(测试系统初值)】之间自由切换
function toggleDeductPortComp() {
    isDeductPortComp = !isDeductPortComp;

    // 更新相控阵面唯一【去端口补偿】按钮状态
    const gridBtn = document.getElementById('btnGridPortComp');
    const gridTxt = document.getElementById('gridPortCompText');
    if (gridBtn) gridBtn.classList.toggle('active', isDeductPortComp);
    if (gridTxt) gridTxt.textContent = isDeductPortComp ? '去端口补偿 (已开启)' : '去端口补偿';

    // 同步阵面标题栏状态徽章与副标题提示
    const badge = document.getElementById('badgePortCompStatus');
    const subEl = document.getElementById('currentArraySubtitle');
    if (badge) badge.style.display = isDeductPortComp ? 'inline-flex' : 'none';
    if (subEl) {
        subEl.textContent = isDeductPortComp
            ? `✨ 去端口补偿模式 · 当前显示 ${currentArray} 阵面测试系统最初写入的理论初值 (已扣除微带走线固定偏置)`
            : (currentArray === 'TX' ? '16 芯片 · 128 通道高密度发射微带排布 · 物理回读原值' : '16 芯片 · 128 通道高密度接收微带排布 · 物理回读原值');
    }

    showToast(isDeductPortComp 
        ? '✨ 已开启【去端口补偿显示】功能：当前正在展示测试系统最初写入的理论初值（已扣除微带走线固有偏置）' 
        : '🔄 已切换回【物理回读原值】显示：当前展示芯片寄存器实测回读原值');
    
    if (statusData) {
        lastMatrixFingerprint = ''; // 强制清除指纹立即重绘
        renderMatrix(statusData);
    }
}

function onSearchFilter() {
    const inp = document.getElementById('searchInput');
    const clearBtn = document.getElementById('searchClearBtn');
    searchQuery = inp.value.trim().toUpperCase();

    if (clearBtn) {
        clearBtn.style.display = searchQuery ? 'block' : 'none';
    }

    if (statusData) {
        applySearchFilter();
    }
}

function clearSearch() {
    const inp = document.getElementById('searchInput');
    if (inp) {
        inp.value = '';
        onSearchFilter();
        inp.focus();
    }
}

async function toggleSimulation() {
    try {
        const res = await fetch('/api/toggle_sim', { method: 'POST' });
        const data = await res.json();
        updateSimButton(data.simulation_mode);
    } catch (err) {
        console.error('Failed to toggle simulation:', err);
    }
}

function updateSimButton(isSim) {
    const simBtn = document.getElementById('btnSimToggle');
    const textEl = document.getElementById('simBtnText');
    if (simBtn && textEl) {
        if (isSim) {
            simBtn.classList.add('sim-on');
            textEl.textContent = '模拟测试: 开';
        } else {
            simBtn.classList.remove('sim-on');
            textEl.textContent = '模拟测试: 关';
        }
    }
}

// ========================================================
// 4. Terminal Log Output & View Modes (Hex, Ascii, Both)
// ========================================================
function switchLogTab(tab) {
    activeLogTab = tab;
    document.getElementById('tabUart3Btn').classList.toggle('active', tab === 'uart3');
    document.getElementById('tabUart0Btn').classList.toggle('active', tab === 'uart0');
    document.getElementById('logUart3').classList.toggle('active', tab === 'uart3');
    document.getElementById('logUart0').classList.toggle('active', tab === 'uart0');
}

function setLogViewMode(mode) {
    logViewMode = mode;
    ['btnLogAscii', 'btnLogHex', 'btnLogBoth'].forEach(id => {
        const btn = document.getElementById(id);
        if (btn) btn.classList.remove('active');
    });

    if (mode === 'ascii') document.getElementById('btnLogAscii')?.classList.add('active');
    else if (mode === 'hex') document.getElementById('btnLogHex')?.classList.add('active');
    else if (mode === 'both') document.getElementById('btnLogBoth')?.classList.add('active');

    if (statusData) {
        appendLogs('logUart3', statusData.uart3_logs);
        appendLogs('logUart0', statusData.uart0_logs);
    }
}

function initLogResizer() {
    const resizer = document.getElementById('logResizer');
    const logCard = document.getElementById('mainLogCard');
    const sidebar = document.querySelector('.sidebar-panel');
    if (!resizer || !logCard || !sidebar) return;

    // 初始化时采用智能自适应最优高度
    userCustomLogHeight = null;
    autoFitLeftPanelHeight(activeCmdTab);

    // 2. 拖拽核心逻辑（采用 Pointer Events + Pointer Capture 保证任何情况下绝对不丢指针）
    let isDragging = false;
    let startY = 0;
    let startLogH = 0;

    const onPointerDown = (e) => {
        if (e.button !== 0) return; // 仅左键
        isDragging = true;
        startY = e.clientY;
        startLogH = logCard.getBoundingClientRect().height;

        resizer.classList.add('resizing');
        document.body.classList.add('is-resizing-log');

        if (resizer.setPointerCapture && e.pointerId !== undefined) {
            try {
                resizer.setPointerCapture(e.pointerId);
            } catch (err) {
                // Ignore fallback
            }
        }
        e.preventDefault();
    };

    const onPointerMove = (e) => {
        if (!isDragging) return;
        const deltaY = startY - e.clientY; // 鼠标往上，deltaY > 0，日志高度增加；鼠标往下，deltaY < 0，日志高度减小
        const sidebarRect = sidebar.getBoundingClientRect();
        const sidebarH = sidebarRect.height || window.innerHeight;

        // 顶端吸附检测：向上拖拽到仅剩 60px 时吸附为最大化
        if (startLogH + deltaY >= sidebarH - 60) {
            if (!sidebar.classList.contains('log-maximized')) {
                sidebar.classList.add('log-maximized');
                const btnExpand = document.getElementById('btnExpandLog');
                if (btnExpand) btnExpand.innerHTML = '⬇️ 还原';
            }
            return;
        }

        // 如果在最大化模式下向下拖拽，退出最大化
        if (sidebar.classList.contains('log-maximized') && deltaY < 0) {
            sidebar.classList.remove('log-maximized');
            const btnExpand = document.getElementById('btnExpandLog');
            if (btnExpand) btnExpand.innerHTML = '🔝 放大至顶端';
        }

        // 计算受约束的目标高度
        const minH = 70;
        const maxH = Math.max(minH, sidebarH - 120);
        const newH = Math.round(Math.max(minH, Math.min(maxH, startLogH + deltaY)));

        userCustomLogHeight = newH;
        logCard.style.height = `${newH}px`;
        logCard.style.flex = `0 0 ${newH}px`;
    };

    const onPointerUp = (e) => {
        if (!isDragging) return;
        isDragging = false;
        resizer.classList.remove('resizing');
        document.body.classList.remove('is-resizing-log');

        if (resizer.releasePointerCapture && e.pointerId !== undefined) {
            try {
                if (resizer.hasPointerCapture(e.pointerId)) {
                    resizer.releasePointerCapture(e.pointerId);
                }
            } catch (err) {
                // Ignore fallback
            }
        }

        if (userCustomLogHeight !== null) {
            localStorage.setItem('GT2001_LOG_HEIGHT', String(userCustomLogHeight));
        }
    };

    // 绑定 Pointer Events（现代浏览器及 WebView2 标准）
    resizer.addEventListener('pointerdown', onPointerDown);
    resizer.addEventListener('pointermove', onPointerMove);
    resizer.addEventListener('pointerup', onPointerUp);
    resizer.addEventListener('pointercancel', onPointerUp);

    // 降级支持原生鼠标事件（双保险兜底）
    resizer.addEventListener('mousedown', onPointerDown);
    window.addEventListener('mousemove', onPointerMove);
    window.addEventListener('mouseup', onPointerUp);

    // 双击手柄一键复位
    resizer.addEventListener('dblclick', (e) => {
        e.preventDefault();
        userCustomLogHeight = null;
        localStorage.removeItem('GT2001_LOG_HEIGHT');
        if (sidebar.classList.contains('log-maximized')) {
            sidebar.classList.remove('log-maximized');
            const btnExpand = document.getElementById('btnExpandLog');
            if (btnExpand) btnExpand.innerHTML = '🔝 放大至顶端';
        }
        autoFitLeftPanelHeight(activeCmdTab);
        showToast('🔄 回读日志已恢复默认自适应高度');
    });
}

// Expand Log Directly to Top / Restore
function toggleLogHeight() {
    const sidebar = document.querySelector('.sidebar-panel');
    const logCard = document.getElementById('mainLogCard');
    const btnExpand = document.getElementById('btnExpandLog');
    if (!sidebar || !logCard) return;

    const isMax = sidebar.classList.contains('log-maximized');
    if (!isMax) {
        // Expand directly all the way to top
        sidebar.classList.add('log-maximized');
        logCard.style.height = '';
        logCard.style.flex = '1 1 100%';
        if (btnExpand) btnExpand.innerHTML = '⬇️ 还原';
        showToast('🔝 回读日志已直接拉到最顶端 (全高终端模式)');
    } else {
        // Restore to normal
        sidebar.classList.remove('log-maximized');
        if (btnExpand) btnExpand.innerHTML = '🔝 放大至顶端';
        autoFitLeftPanelHeight(activeCmdTab);
        showToast('⬇️ 已还原控制面板至初始位置，内容完整显示');
    }
}

function clearActiveLog() {
    const target = activeLogTab; // 'uart3' or 'uart0'
    const targetEl = target === 'uart3' ? 'logUart3' : 'logUart0';
    const logBox = document.getElementById(targetEl);
    if (logBox) {
        logBox.innerHTML = '<div class="log-empty-tip">已清屏，等待新数据...</div>';
    }

    // Immediately clear local cache so polling won't re-render stale entries
    if (statusData) {
        if (target === 'uart3') statusData.uart3_logs = [];
        else statusData.uart0_logs = [];
    }
    lastLogFingerprints[targetEl] = 'CLEARED';

    // Request backend to clear its memory buffer
    fetch('/api/clear_logs', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: target })
    }).then(res => res.json()).then(data => {
        showToast(`🧹 ${target === 'uart3' ? 'UART3 回读' : 'UART0 发送'} 日志已彻底清空`);
    }).catch(err => {
        console.error('Failed to clear logs on server:', err);
    });
}

function clearAllLogs() {
    ['logUart3', 'logUart0'].forEach(id => {
        const b = document.getElementById(id);
        if (b) b.innerHTML = '<div class="log-empty-tip">已清屏，等待新数据...</div>';
    });

    if (statusData) {
        statusData.uart3_logs = [];
        statusData.uart0_logs = [];
    }
    lastLogFingerprints['logUart3'] = 'CLEARED';
    lastLogFingerprints['logUart0'] = 'CLEARED';

    fetch('/api/clear_logs', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ target: 'all' })
    }).then(res => res.json()).then(data => {
        showToast('🧹 已彻底清空所有串口收发日志');
    }).catch(err => {
        console.error('Failed to clear all logs on server:', err);
    });
}

function copyActiveLog() {
    const target = activeLogTab === 'uart3' ? 'logUart3' : 'logUart0';
    const logBox = document.getElementById(target);
    if (logBox) {
        navigator.clipboard.writeText(logBox.innerText).then(() => {
            showToast('📋 日志已成功复制到剪贴板！');
        }).catch(() => {
            alert('复制失败，请手动选取文本。');
        });
    }
}

const lastLogFingerprints = {
    'logUart3': '',
    'logUart0': ''
};

function appendLogs(containerId, logs) {
    const container = document.getElementById(containerId);
    if (!container) return;

    if (!logs || logs.length === 0) {
        if (lastLogFingerprints[containerId] !== 'EMPTY') {
            lastLogFingerprints[containerId] = 'EMPTY';
            container.innerHTML = '<div class="log-empty-tip">暂无数据记录 (已清屏)</div>';
        }
        return;
    }

    // 脏检查：如果日志条数、头尾时间与内容均无变化，跳过 DOM 重建
    const lastItem = logs[logs.length - 1] || {};
    const firstItem = logs[0] || {};
    const fp = `${logViewMode}_${logs.length}_${firstItem.time || ''}_${lastItem.time || ''}_${lastItem.text || ''}_${lastItem.hex || ''}`;
    if (lastLogFingerprints[containerId] === fp) {
        return;
    }
    lastLogFingerprints[containerId] = fp;

    let html = '';
    logs.forEach(log => {
        let textClass = 'log-line';
        const txt = log.text || '';
        const hex = log.hex || '';

        if (txt.includes('BEAM_REG')) textClass += ' log-highlight';
        else if (txt.includes('BEAM_UPDATE')) textClass += ' log-highlight';
        else if (txt.includes('>>')) textClass += ' log-cmd';
        else if (txt.includes('PTS:')) textClass += ' log-cmd';
        else if (txt.includes('POWER:')) textClass += ' log-cmd';
        else if (txt.includes('ERROR') || txt.includes('ERR')) textClass += ' log-err';
        else if (txt.includes('WARN')) textClass += ' log-warn';

        if (logViewMode === 'ascii') {
            html += `<div class="${textClass}"><span class="log-time">[${log.time}]</span>${escapeHtml(txt || hex)}</div>`;
        } else if (logViewMode === 'hex') {
            html += `<div class="${textClass}"><span class="log-time">[${log.time}]</span>${escapeHtml(hex || txt)}</div>`;
        } else { // both
            html += `<div class="${textClass}">`;
            html += `  <span class="log-time">[${log.time}]</span>${escapeHtml(txt)}`;
            if (hex) {
                html += `  <span class="log-sub-hex">HEX: ${escapeHtml(hex)}</span>`;
            }
            html += `</div>`;
        }
    });
    
    const prevScrollTop = container.scrollTop;
    container.innerHTML = html;
    if (autoScroll) {
        container.scrollTop = container.scrollHeight;
    } else {
        container.scrollTop = prevScrollTop;
    }
}

function escapeHtml(str) {
    return str.replace(/&/g, '&amp;')
              .replace(/</g, '&lt;')
              .replace(/>/g, '&gt;');
}

// ========================================================
// 5. Polling & Status Loop
// ========================================================
function startPolling() {
    pollStatus();
    pollTimer = setInterval(pollStatus, 80); // 80ms for ultra-responsive real-time display!
}

async function pollStatus() {
    try {
        const res = await fetch('/api/status');
        const data = await res.json();
        statusData = data;
        updateUIState(data);
        renderMatrix(data);
    } catch (err) {
        // Server offline
    }
}

function updateUIState(data) {
    // UART0 connection status
    const ind0 = document.getElementById('indUart0');
    const btn0 = document.getElementById('btnConnectUart0');
    if (ind0 && btn0) {
        const isConn = data.uart0.status === 'connected';
        ind0.className = 'status-indicator' + (isConn ? ' connected' : '');
        btn0.textContent = isConn ? '断开' : '连接';
        btn0.className = 'btn-sm ' + (isConn ? 'btn-danger' : 'btn-primary');
    }

    // UART3 connection status
    const ind3 = document.getElementById('indUart3');
    const btn3 = document.getElementById('btnConnectUart3');
    if (ind3 && btn3) {
        const isConn = data.uart3.status === 'connected';
        ind3.className = 'status-indicator' + (isConn ? ' connected' : '');
        btn3.textContent = isConn ? '断开' : '连接';
        btn3.className = 'btn-sm ' + (isConn ? 'btn-danger' : 'btn-primary');
    }

    // Update Header UART Quick Status Badges
    const hdrUart0 = document.getElementById('hdrUart0Pill');
    const isU0Conn = data.uart0.status === 'connected';
    if (hdrUart0) {
        hdrUart0.classList.toggle('connected', isU0Conn);
        const baudSpan = hdrUart0.querySelector('.baud-tag');
        if (baudSpan) baudSpan.textContent = `${data.uart0.baud || 115200} bps`;
    }

    const hdrUart3 = document.getElementById('hdrUart3Pill');
    const isU3Conn = data.uart3.status === 'connected';
    if (hdrUart3) {
        hdrUart3.classList.toggle('connected', isU3Conn);
        const baudSpan = hdrUart3.querySelector('.baud-tag');
        if (baudSpan) baudSpan.textContent = `${data.uart3.baud || 921600} bps`;
    }

    // Simulation toggle
    updateSimButton(data.simulation_mode);

    // Array Power State Indicator
    const pwrState = data.system.array_power || 'UNKNOWN';
    const hdrPwrPill = document.getElementById('hdrPowerPill');
    const hdrPwrTxt = document.getElementById('hdrPowerText');
    const badgePwr = document.getElementById('badgePowerState');

    if (hdrPwrPill && hdrPwrTxt) {
        if (pwrState === 'ON') {
            hdrPwrPill.className = 'system-status-pill power-pill pwr-on';
            hdrPwrTxt.textContent = '阵面供电: 已上电 (ON)';
        } else if (pwrState === 'OFF') {
            hdrPwrPill.className = 'system-status-pill power-pill pwr-off';
            hdrPwrTxt.textContent = '阵面供电: 已下电 (OFF)';
        } else {
            hdrPwrPill.className = 'system-status-pill power-pill';
            hdrPwrTxt.textContent = '阵面供电: 未知';
        }
    }
    if (badgePwr) {
        badgePwr.textContent = pwrState === 'ON' ? '状态: 已上电' : (pwrState === 'OFF' ? '状态: 已下电' : '状态: 未知');
        badgePwr.style.color = pwrState === 'ON' ? '#059669' : (pwrState === 'OFF' ? '#dc2626' : '#64748b');
    }

    // Whole-Array Channel State Indicators (TX/RX 全开全关)
    const txState = data.system.tx_channels || 'UNKNOWN';
    const rxState = data.system.rx_channels || 'UNKNOWN';
    const badgeTxChl = document.getElementById('badgeTxChlState');
    const badgeRxChl = document.getElementById('badgeRxChlState');
    if (badgeTxChl) {
        badgeTxChl.textContent = txState === 'ALL_ON' ? 'TX: 全开' : (txState === 'ALL_OFF' ? 'TX: 全关' : 'TX: 未知');
        badgeTxChl.style.color = txState === 'ALL_ON' ? '#059669' : (txState === 'ALL_OFF' ? '#dc2626' : '#64748b');
        badgeTxChl.style.fontWeight = '700';
    }
    if (badgeRxChl) {
        badgeRxChl.textContent = rxState === 'ALL_ON' ? 'RX: 全开' : (rxState === 'ALL_OFF' ? 'RX: 全关' : 'RX: 未知');
        badgeRxChl.style.color = rxState === 'ALL_ON' ? '#059669' : (rxState === 'ALL_OFF' ? '#dc2626' : '#64748b');
        badgeRxChl.style.fontWeight = '700';
    }

    // Pointing Mode Indicator
    const pMode = data.system.pointing_mode || 'MANUAL';
    const hdrModePill = document.getElementById('hdrModePill');
    const hdrModeTxt = document.getElementById('hdrModeText');
    const badgePts = document.getElementById('badgePtsState');

    if (hdrModePill && hdrModeTxt) {
        if (pMode === 'AUTO') {
            hdrModePill.className = 'system-status-pill mode-pill auto';
            hdrModeTxt.textContent = '对星模式: 自动 (AUTO)';
        } else {
            hdrModePill.className = 'system-status-pill mode-pill';
            hdrModeTxt.textContent = '对星模式: 手动 (MANUAL)';
        }
    }
    if (badgePts) {
        badgePts.textContent = pMode === 'AUTO' ? '自动模式' : '手动模式';
        badgePts.style.color = pMode === 'AUTO' ? '#16a34a' : '#ea580c';
    }

    // Live REG Readback Verification Card
    const regReplyBox = document.getElementById('boxRegReadback');
    const txtRegReply = document.getElementById('txtRegReply');
    if (regReplyBox && txtRegReply) {
        if (data.system.last_reg_reply) {
            const reply = data.system.last_reg_reply;
            txtRegReply.textContent = `${reply.text} (${reply.time})`;
            if (reply.text.includes('OK')) {
                txtRegReply.className = 'reg-reply-content ok';
            } else if (reply.text.includes('FAIL') || reply.text.includes('ERR')) {
                txtRegReply.className = 'reg-reply-content fail';
            } else {
                txtRegReply.className = 'reg-reply-content';
            }
        }
    }

    // Terminal Logs
    appendLogs('logUart3', data.uart3_logs);
    appendLogs('logUart0', data.uart0_logs);

    // Grid stats
    const nowStr = new Date().toLocaleTimeString();
    const lblUpdate = document.getElementById('lblLastUpdate');
    if (lblUpdate) lblUpdate.textContent = nowStr;

    // Beam angle inverse estimation
    renderBeamInverse(data);
}

// ========================================================
// 6. 4x4 Phased-Array Matrix Rendering (Strict Readback Display - Anti-Flicker)
// ========================================================
let lastMatrixFingerprint = '';

function renderMatrix(data) {
    const container = document.getElementById('chipMatrixGrid');
    if (!container || !data.grids) return;

    const grid = data.grids[currentArray];
    const regMap = data.registers[currentArray];
    if (!grid || !regMap) return;

    // 1. Calculate read chips count
    let readChipsCount = 0;
    for (let r = 0; r < 4; r++) {
        for (let c = 0; c < 4; c++) {
            const chipId = grid[r][c];
            const chipRegs = regMap[chipId];
            if (chipRegs) {
                for (let ch = 0; ch < 8; ch++) {
                    if (chipRegs[ch] && chipRegs[ch].has_readback) {
                        readChipsCount++;
                        break;
                    }
                }
            }
        }
    }

    // Update chip readback badge
    const lblChip = document.getElementById('lblChipCount');
    if (lblChip) {
        if (readChipsCount === 0) {
            lblChip.textContent = '等待回读 (0/16)';
            lblChip.style.color = '#64748b';
        } else if (readChipsCount === 16) {
            lblChip.textContent = '全部回读 (16/16)';
            lblChip.style.color = '#059669';
        } else {
            lblChip.textContent = `${readChipsCount}/16 已回读`;
            lblChip.style.color = '#2563eb';
        }
    }

    // 2. Build fingerprint to detect changes (Anti-Flicker Guard, including port compensation toggle)
    const currentFingerprint = `${currentArray}_${displayMode}_${isDeductPortComp}_${JSON.stringify(regMap)}`;
    const hasExistingCards = (container.dataset.renderedArray === currentArray) && (container.querySelectorAll('.chip-card').length === 16);

    // If data, array and mode haven't changed, DO NOT touch the DOM at all!
    if (currentFingerprint === lastMatrixFingerprint && hasExistingCards) {
        return;
    }
    lastMatrixFingerprint = currentFingerprint;

    // 3. Initial full build if array changed or DOM not yet rendered
    if (!hasExistingCards) {
        let html = '';
        for (let r = 0; r < 4; r++) {
            const isYellowTheme = (r % 2 === 0);
            const themeClass = isYellowTheme ? 'theme-yellow' : 'theme-green';

            for (let c = 0; c < 4; c++) {
                const chipId = grid[r][c];
                const chipPad = String(chipId).padStart(2, '0');
                const coordStr = `R${r + 1} C${c + 1}`;
                const cardId = `chip-card-${currentArray}-${chipId}`;
                const chipRegs = regMap[chipId];

                const leftChannels = isYellowTheme ? [7, 5, 3, 1] : [0, 2, 4, 6];
                const rightChannels = isYellowTheme ? [6, 4, 2, 0] : [1, 3, 5, 7];

                const isSelected = (selectedTargetChipId < 16 && selectedTargetChipId === chipId);
                const selClass = isSelected ? ' chip-card-selected' : '';
                const isCenter = [5, 6, 9, 10].includes(chipId);
                const posDesc = isCenter ? '⭐ 中心核心阵元' : '🛡️ 外围边缘阵元';
                html += `<div class="chip-card ${themeClass}${selClass}" id="${cardId}" data-chip="${chipId}" onclick="selectChipForReg(${chipId})" title="CHIP ${chipPad} (${coordStr}) [${posDesc}]: 点击联动单片寄存器配置">`;
                html += `  <div class="chip-card-header">`;
                html += `    <span class="chip-id-text">${currentArray} · CHIP ${chipPad}</span>`;
                html += `    <span class="chip-coord-text">${coordStr}</span>`;
                html += `  </div>`;
                html += `  <div class="chip-channels-container">`;

                // Left Column
                html += `    <div class="channel-column">`;
                leftChannels.forEach(ch => {
                    html += renderChannelRow(chipId, ch, chipRegs ? chipRegs[ch] : null);
                });
                html += `    </div>`;

                // Right Column
                html += `    <div class="channel-column">`;
                rightChannels.forEach(ch => {
                    html += renderChannelRow(chipId, ch, chipRegs ? chipRegs[ch] : null);
                });
                html += `    </div>`;

                html += `  </div>`;
                html += `</div>`;
            }
        }
        container.innerHTML = html;
        container.dataset.renderedArray = currentArray;
        applySearchFilter();
        return;
    }

    // 4. In-Place fine-grained update: ONLY update changed channel rows without destroying cards!
    for (let r = 0; r < 4; r++) {
        for (let c = 0; c < 4; c++) {
            const chipId = grid[r][c];
            const chipRegs = regMap[chipId];
            const card = document.getElementById(`chip-card-${currentArray}-${chipId}`);
            if (!card) continue;
            card.classList.toggle('chip-card-selected', selectedTargetChipId < 16 && chipId === selectedTargetChipId);

            for (let ch = 0; ch < 8; ch++) {
                const row = card.querySelector(`.channel-row[data-ch="${ch}"]`);
                if (!row) continue;
                const newRowHtml = renderChannelRow(chipId, ch, chipRegs ? chipRegs[ch] : null).trim();
                if (row.outerHTML.trim() !== newRowHtml) {
                    row.outerHTML = newRowHtml;
                }
            }
        }
    }
    applySearchFilter();
}

/**
 * Strict Channel Row Rendering:
 * Strictly reflect readback status:
 * - If has_readback === false: render unread state (--° / P--)
 * - If error === true: render error state (ERR / 0xFF)
 * - If valid: render actual value (xx.xxx° / Pxx)
 */
function renderChannelRow(chipId, ch, regData) {
    const chLabel = `CH${ch}`;
    let rowClass = 'channel-row';
    let badgeHtml = '';
    let rowTooltip = `通道 ${chLabel}`;

    const hasReadback = regData && regData.has_readback;
    const isError = regData && (regData.error === true || regData.phase === 255 || regData.amp === 255);
    const isOpen = regData && (regData.is_open === true || (regData.amp !== null && regData.amp > 0 && regData.amp !== 255));
    const isLit = isOpen && hasReadback && !isError;

    if (!hasReadback) {
        rowClass += ' unread';
        rowTooltip = `${chLabel}: 等待回读 (通道${isOpen ? '已开' : '已关'})`;
        if (displayMode === 'PHASE') {
            badgeHtml = `<span class="pill-badge unread">P--</span>`;
        } else if (displayMode === 'AMP') {
            badgeHtml = `<span class="pill-badge unread">A--</span>`;
        } else {
            badgeHtml = `<span class="pill-badge unread">P-- A--</span>`;
        }
    } else if (isError) {
        rowClass += ' error';
        rowTooltip = `${chLabel}: 回读校验错误 (0xFF)`;
        badgeHtml = `<span class="pill-badge err-badge">0xFF</span>`;
    } else {
        const phase = regData.phase !== null ? regData.phase : 0;
        const amp = regData.amp !== null ? regData.amp : 63;
        const phaseDeg = regData.phase_deg !== null ? regData.phase_deg : (phase * 5.625);

        // 去端口补偿计算 (显示测试系统最初写的值)
        const deducted = getDeductedPortPhase(currentArray, chipId, ch, phaseDeg);
        const dispPhase = isDeductPortComp ? deducted.deductedCode : phase;
        const dispPhaseDeg = isDeductPortComp ? deducted.deductedDeg : phaseDeg;
        const rot = Math.round(dispPhaseDeg);
        const compTagHtml = isDeductPortComp ? '<span class="comp-tag">*</span>' : '';
        const compDescText = isDeductPortComp ? ` [✨去补偿初值: P${String(dispPhase).padStart(2, '0')}/${dispPhaseDeg.toFixed(3)}°, 偏置: -${deducted.biasDeg}°, 实读: P${String(phase).padStart(2, '0')}]` : ` (REG: P${String(phase).padStart(2, '0')})`;

        if (isLit) {
            // 通道开且有寄存器值 -> 触发点亮！
            rowClass += ' channel-lit';
            if (displayMode === 'PHASE') {
                rowTooltip = `${chLabel} [🟢已点亮]: 相位 ${dispPhaseDeg.toFixed(3)}°${compDescText}`;
                badgeHtml = `
                    <div class="ch-reg-capsule">
                        <span class="mini-phase-dot" style="--deg: ${rot}deg"></span>
                        <span class="pill-badge lit-badge">P${String(dispPhase).padStart(2, '0')}${compTagHtml}</span>
                    </div>`;
            } else if (displayMode === 'AMP') {
                rowTooltip = `${chLabel} [🟢已点亮]: 幅度 ${amp} (REG: A${String(amp).padStart(2, '0')})`;
                badgeHtml = `
                    <div class="ch-reg-capsule">
                        <span class="pill-badge amp-badge lit-badge">A${String(amp).padStart(2, '0')}</span>
                    </div>`;
            } else { // BOTH
                rowTooltip = `${chLabel} [🟢已点亮]: 相位 ${dispPhaseDeg.toFixed(3)}° · 幅度 ${amp}${compDescText}`;
                badgeHtml = `
                    <div class="ch-reg-capsule">
                        <span class="mini-phase-dot" style="--deg: ${rot}deg"></span>
                        <span class="pill-badge both-badge lit-badge">P${String(dispPhase).padStart(2, '0')}${compTagHtml} A${String(amp).padStart(2, '0')}</span>
                    </div>`;
            }
        } else {
            // 通道未开 -> 即使寄存器有值也不点亮（灰色暗态）
            rowClass += ' channel-closed';
            if (displayMode === 'PHASE') {
                rowTooltip = `${chLabel} [⚪未开/未点亮]: 相位 ${dispPhaseDeg.toFixed(3)}°${compDescText}`;
                badgeHtml = `
                    <div class="ch-reg-capsule closed">
                        <span class="mini-phase-dot dim" style="--deg: ${rot}deg"></span>
                        <span class="pill-badge closed-badge">P${String(dispPhase).padStart(2, '0')}${compTagHtml}</span>
                    </div>`;
            } else if (displayMode === 'AMP') {
                rowTooltip = `${chLabel} [⚪未开/未点亮]: 寄存器值 A${String(amp).padStart(2, '0')}`;
                badgeHtml = `
                    <div class="ch-reg-capsule closed">
                        <span class="pill-badge closed-badge">A${String(amp).padStart(2, '0')}</span>
                    </div>`;
            } else { // BOTH
                rowTooltip = `${chLabel} [⚪未开/未点亮]: 相位 ${dispPhaseDeg.toFixed(3)}° · 幅度 ${amp}${compDescText}`;
                badgeHtml = `
                    <div class="ch-reg-capsule closed">
                        <span class="mini-phase-dot dim" style="--deg: ${rot}deg"></span>
                        <span class="pill-badge closed-badge">P${String(dispPhase).padStart(2, '0')}${compTagHtml} A${String(amp).padStart(2, '0')}</span>
                    </div>`;
            }
        }
    }

    return `
        <div class="${rowClass}" data-ch="${ch}" title="${rowTooltip}">
            <span class="ch-label">${chLabel}</span>
            ${badgeHtml}
        </div>
    `;
}

function applySearchFilter() {
    if (!searchQuery || !searchQuery.trim()) {
        document.querySelectorAll('.chip-card').forEach(card => {
            card.classList.remove('filter-matched', 'filter-dimmed');
            card.querySelectorAll('.channel-row').forEach(row => row.classList.remove('ch-search-highlight'));
        });
        return;
    }

    const rawQ = searchQuery.trim().toUpperCase();
    // If input is purely digits like "3" or "03" or "13"
    const isNumOnly = /^\d+$/.test(rawQ);
    const targetChipNum = isNumOnly ? parseInt(rawQ, 10) : null;

    document.querySelectorAll('.chip-card').forEach(card => {
        const chipId = card.getAttribute('data-chip');
        const chipNum = parseInt(chipId, 10);
        let matched = false;

        card.querySelectorAll('.channel-row').forEach(row => row.classList.remove('ch-search-highlight'));

        if (isNumOnly) {
            // STRICT chip ID matching: "3" matches CHIP 03, NEVER CHIP 13!
            matched = (chipNum === targetChipNum);
        } else if (rawQ.startsWith('CHIP')) {
            const numPart = parseInt(rawQ.replace('CHIP', '').trim(), 10);
            matched = !isNaN(numPart) && (chipNum === numPart);
        } else if (rawQ.startsWith('CH')) {
            const chNum = rawQ.replace('CH', '').trim();
            const chRow = card.querySelector(`.channel-row[data-ch="${chNum}"]`);
            if (chRow) {
                matched = true;
                chRow.classList.add('ch-search-highlight');
            }
        } else {
            matched = (chipId === rawQ || chipNum.toString() === rawQ);
        }

        if (matched) {
            card.classList.add('filter-matched');
            card.classList.remove('filter-dimmed');
        } else {
            card.classList.remove('filter-matched');
            card.classList.add('filter-dimmed');
        }
    });
}

// ========================================================
// 7. Beam Angle Inverse Estimation Rendering
// ========================================================
let currentPolMode = '+2*pol';

async function togglePolMode() {
    currentPolMode = (currentPolMode === '+2*pol') ? '-2*pol' : '+2*pol';
    const label = document.getElementById('polModeLabel');
    const btn = document.getElementById('btnPolModeToggle');
    if (label) {
        label.textContent = currentPolMode === '+2*pol' ? '算法: +2.0*Pol (当前固件)' : '算法: -2.0*Pol (旧版固件)';
    }
    if (btn) {
        btn.classList.toggle('mode-neg', currentPolMode === '-2*pol');
    }
    showToast(`🔀 已切换极化反推算法为 ${currentPolMode === '+2*pol' ? '【+2.0*Pol 当前固件修正】' : '【-2.0*Pol 旧版算法】'}`);

    try {
        const res = await fetch('/api/set_pol_mode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode: currentPolMode })
        });
        const result = await res.json();
        if (result.success && result.beam_inverse && typeof statusData !== 'undefined' && statusData && statusData.system) {
            statusData.system.beam_inverse = result.beam_inverse;
            statusData.system.pol_algorithm_mode = result.mode;
            renderBeamInverse(statusData);
        }
    } catch (e) {
        console.warn('Set pol mode error:', e);
    }
}

function renderBeamInverse(data) {
    const card = document.getElementById('beamInverseCard');
    if (!card) return;

    const invPitchVal = document.getElementById('invPitchVal');
    const invYawVal = document.getElementById('invYawVal');
    const invPolVal = document.getElementById('invPolVal');
    const invPolSub = document.getElementById('invPolSub');
    const invGradXVal = document.getElementById('invGradXVal');
    const invGradYVal = document.getElementById('invGradYVal');
    const invConfVal = document.getElementById('invConfVal');
    const invChannelsVal = document.getElementById('invChannelsVal');
    const invStatusBadge = document.getElementById('invStatusBadge');
    const invFreqMeta = document.getElementById('invFreqMeta');
    const polModeLabel = document.getElementById('polModeLabel');
    const btnPolModeToggle = document.getElementById('btnPolModeToggle');

    if (data && data.system && data.system.pol_algorithm_mode) {
        currentPolMode = data.system.pol_algorithm_mode;
    }
    if (polModeLabel) {
        polModeLabel.textContent = currentPolMode === '+2*pol' ? '算法: +2.0*Pol (当前固件)' : '算法: -2.0*Pol (旧版固件)';
    }
    if (btnPolModeToggle) {
        btnPolModeToggle.classList.toggle('mode-neg', currentPolMode === '-2*pol');
    }

    const freqGhz = (currentArray === 'TX' ? 14.250 : 12.250);
    if (invFreqMeta) {
        invFreqMeta.textContent = `基准频率: ${freqGhz.toFixed(3)} GHz (${currentArray})`;
    }

    const inv = (data && data.system && data.system.beam_inverse) ? data.system.beam_inverse[currentArray] : null;

    if (inv && inv.status === 'success' && inv.pitch !== null) {
        if (invPitchVal) invPitchVal.textContent = `${inv.pitch.toFixed(2)}°`;
        if (invYawVal) invYawVal.textContent = `${inv.yaw.toFixed(2)}°`;
        
        const activePol = currentPolMode === '+2*pol' 
            ? (inv.pol_pos !== undefined ? inv.pol_pos : inv.pol)
            : (inv.pol_neg !== undefined ? inv.pol_neg : inv.pol);
        if (invPolVal) invPolVal.textContent = `${activePol.toFixed(2)}°`;
        if (invPolSub) {
            if (inv.pol_pos !== undefined && inv.pol_neg !== undefined) {
                invPolSub.textContent = `+2*Pol: ${inv.pol_pos.toFixed(1)}° | -2*Pol: ${inv.pol_neg.toFixed(1)}°`;
            } else {
                invPolSub.textContent = '正交支路相位差合成';
            }
        }

        if (invGradXVal) invGradXVal.textContent = inv.grad_x !== null ? `${inv.grad_x >= 0 ? '+' : ''}${inv.grad_x.toFixed(2)}°/elem` : '--°/elem';
        if (invGradYVal) invGradYVal.textContent = inv.grad_y !== null ? `${inv.grad_y >= 0 ? '+' : ''}${inv.grad_y.toFixed(2)}°/elem` : '--°/elem';
        if (invConfVal) invConfVal.textContent = `${inv.confidence.toFixed(1)}%`;
        if (invChannelsVal) invChannelsVal.textContent = `有效通道: ${inv.valid_channels || 128} / 128`;
        if (invStatusBadge) {
            invStatusBadge.className = 'inverse-badge badge-success';
            invStatusBadge.textContent = '反推成功';
        }
    } else {
        if (invPitchVal) invPitchVal.textContent = '--°';
        if (invYawVal) invYawVal.textContent = '--°';
        if (invPolVal) invPolVal.textContent = '--°';
        if (invPolSub) invPolSub.textContent = '正交支路相位差合成';
        if (invGradXVal) invGradXVal.textContent = '--°/elem';
        if (invGradYVal) invGradYVal.textContent = '--°/elem';
        if (invConfVal) invConfVal.textContent = '--%';
        const validCh = inv && inv.valid_channels ? inv.valid_channels : 0;
        if (invChannelsVal) invChannelsVal.textContent = `有效通道: ${validCh} / 128`;
        if (invStatusBadge) {
            invStatusBadge.className = 'inverse-badge badge-waiting';
            invStatusBadge.textContent = validCh > 0 ? `回读中 (${validCh} CH)` : '等待回读流';
        }
    }
}


// ========================================================
// 8. Compact View & Window Resize Adaptation
// ========================================================
let isCompactManualOverride = false;

function toggleCompactMode() {
    const mainContainer = document.querySelector('.main-container');
    const btnToggle = document.getElementById('btnToggleCompact');
    const btnText = document.getElementById('compactBtnText');
    if (!mainContainer) return;

    const isNowCompact = mainContainer.classList.toggle('compact-mode');
    isCompactManualOverride = isNowCompact;

    if (btnText) {
        btnText.textContent = isNowCompact ? '📱纯控制台' : '🖥️双列阵面';
    }
    if (btnToggle) {
        btnToggle.classList.toggle('active', isNowCompact);
    }
    autoFitLeftPanelHeight(activeCmdTab);
    showToast(isNowCompact ? '📱 已切换为纯控制台模式 (隐藏右侧阵面)' : '🖥️ 已恢复双列全阵面模式');
}

function handleWindowResize() {
    const mainContainer = document.querySelector('.main-container');
    const btnText = document.getElementById('compactBtnText');
    const btnToggle = document.getElementById('btnToggleCompact');
    if (!mainContainer) return;

    const winW = window.innerWidth;
    if (winW < 1080) {
        if (!mainContainer.classList.contains('compact-mode')) {
            mainContainer.classList.add('compact-mode');
            if (btnText) btnText.textContent = '📱纯控制台';
            if (btnToggle) btnToggle.classList.add('active');
        }
    } else {
        if (!isCompactManualOverride && mainContainer.classList.contains('compact-mode')) {
            mainContainer.classList.remove('compact-mode');
            if (btnText) btnText.textContent = '🖥️双列阵面';
            if (btnToggle) btnToggle.classList.remove('active');
        }
    }
    autoFitLeftPanelHeight(activeCmdTab);
}

window.addEventListener('resize', handleWindowResize);

// ==========================================
// 单通道精细控制 (0x02) 前端下发逻辑
// ==========================================
function initSingleChannelControls() {
    const selChip = document.getElementById('singleChChipId');
    if (selChip && selChip.options.length === 0) {
        for (let i = 0; i < 16; i++) {
            const opt = document.createElement('option');
            opt.value = i;
            opt.textContent = `芯片 ID ${String(i).padStart(2, '0')}`;
            selChip.appendChild(opt);
        }
    }
}

async function sendSingleChannelCmd(action) {
    const dirRadio = document.querySelector('input[name="singleChDir"]:checked');
    const target = dirRadio ? dirRadio.value : 'TX';
    const chipId = parseInt(document.getElementById('singleChChipId')?.value || '0', 10);
    const ch = parseInt(document.getElementById('singleChNum')?.value || '0', 10);
    const amp = Math.min(63, Math.max(0, parseInt(document.getElementById('singleChAmp')?.value || '63', 10)));
    const phase = Math.min(63, Math.max(0, parseInt(document.getElementById('singleChPhase')?.value || '0', 10)));
    const pole = 0;

    const actName = action === 'OPEN' ? '🟢开启通道' : (action === 'CLOSE' ? '🔴关闭通道' : `✍️写入幅相(A=${amp}, P=${phase})并回读`);

    // 单通道开启防呆：仅对当前目标单通道先执行单通道复位 (CLOSE 关断)，绝不影响阵面其他 127 个通道
    if (action === 'OPEN') {
        showToast(`🔄 [单通道复位防呆] 正在对 [${target}] 芯片${chipId} CH${ch} 执行先单通道复位(关断)...`);
        try {
            await fetch('/api/send', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    type: 'SINGLE_CH',
                    target: target,
                    chip_id: chipId,
                    ch: ch,
                    action: 'CLOSE',
                    amp: amp,
                    phase: phase,
                    pole: pole
                })
            });
            await new Promise(r => setTimeout(r, 60));
        } catch (e) {
            console.warn('Single channel pre-reset error:', e);
        }
    }

    showToast(`⚡ 正在下发单通道控制: [${target}] 芯片${chipId} CH${ch} - ${actName}...`);

    try {
        const res = await fetch('/api/send', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                type: 'SINGLE_CH',
                target: target,
                chip_id: chipId,
                ch: ch,
                action: action,
                amp: amp,
                phase: phase,
                pole: pole
            })
        });
        const data = await res.json();
        if (data.success) {
            const extraNote = action === 'OPEN' ? ' (已完成先单通道复位防呆)' : '';
            showToast(`✅ [${target}] 芯片${chipId} CH${ch} ${actName} 成功${extraNote}`);
        } else {
            showToast(`❌ 下发失败: ${data.msg || '未知错误'}`);
        }
    } catch (err) {
        showToast(`❌ 网络请求失败: ${err.message}`);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    initSingleChannelControls();
});
initSingleChannelControls();
