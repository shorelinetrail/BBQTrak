#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BBQTrak</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: #1a1a2e;
    color: #e0e0e0;
    min-height: 100vh;
}
.header {
    background: linear-gradient(135deg, #c0392b, #e74c3c);
    padding: 16px 20px;
    text-align: center;
    box-shadow: 0 2px 10px rgba(0,0,0,0.3);
}
.header h1 { font-size: 1.5rem; color: #fff; letter-spacing: 2px; }
.header .subtitle { font-size: 0.75rem; color: rgba(255,255,255,0.7); margin-top: 2px; }
.container { max-width: 800px; margin: 0 auto; padding: 16px; }

.status-bar { display: flex; gap: 8px; margin-bottom: 16px; flex-wrap: wrap; align-items: center; }
.badge { padding: 4px 12px; border-radius: 12px; font-size: 0.75rem; font-weight: 600; }
.badge.running { background: #27ae60; color: #fff; }
.badge.stopped { background: #7f8c8d; color: #fff; }
.badge.lid-open { background: #f39c12; color: #fff; }
.badge.connected { background: #2980b9; color: #fff; }
.badge.disconnected { background: #c0392b; color: #fff; }
.badge.ramp { background: #8e44ad; color: #fff; }
.badge.autotune { background: #e67e22; color: #fff; animation: pulse 1.5s infinite; }
@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.6} }

.temp-cards { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 16px; }
.temp-card {
    background: #16213e; border-radius: 12px; padding: 20px;
    text-align: center; border: 1px solid #0f3460;
}
.temp-card .label { font-size: 0.8rem; text-transform: uppercase; color: #7f8c8d; margin-bottom: 4px; }
.temp-card .value { font-size: 2.5rem; font-weight: 700; }
.temp-card .unit { font-size: 0.9rem; color: #7f8c8d; }
.temp-card.pit .value { color: #e74c3c; }
.temp-card.meat .value { color: #f39c12; }
.temp-card .target { font-size: 0.8rem; color: #7f8c8d; margin-top: 4px; }
.temp-card .eff-target { font-size: 0.7rem; color: #8e44ad; margin-top: 2px; }

.fan-card {
    background: #16213e; border-radius: 12px; padding: 16px;
    margin-bottom: 16px; border: 1px solid #0f3460;
}
.fan-bar-container {
    background: #0f3460; border-radius: 8px; height: 24px;
    overflow: hidden; margin: 8px 0;
}
.fan-bar {
    height: 100%;
    background: linear-gradient(90deg, #27ae60, #f39c12, #e74c3c);
    border-radius: 8px; transition: width 0.5s ease;
}
.fan-label { display: flex; justify-content: space-between; font-size: 0.8rem; color: #7f8c8d; }

.chart-card {
    background: #16213e; border-radius: 12px; padding: 16px;
    margin-bottom: 16px; border: 1px solid #0f3460;
}
.chart-card canvas { width: 100%; height: 200px; }
.chart-duration {
    display: flex; gap: 4px; margin-bottom: 8px;
}
.chart-duration button {
    padding: 4px 10px; border: 1px solid #0f3460; border-radius: 4px;
    background: #0f3460; color: #7f8c8d; cursor: pointer;
    font-size: 0.75rem; font-weight: 600; transition: all 0.2s;
}
.chart-duration button.active {
    background: #2980b9; color: #fff; border-color: #2980b9;
}

.controls {
    background: #16213e; border-radius: 12px; padding: 16px;
    margin-bottom: 16px; border: 1px solid #0f3460;
}
.controls h3 { font-size: 0.9rem; margin-bottom: 12px; color: #7f8c8d; text-transform: uppercase; }
.control-row {
    display: flex; align-items: center; gap: 8px; margin-bottom: 10px; flex-wrap: wrap;
}
.control-row label { font-size: 0.85rem; min-width: 80px; }
.control-row input[type="number"] {
    background: #0f3460; border: 1px solid #1a4a8a; color: #e0e0e0;
    padding: 6px 10px; border-radius: 6px; width: 80px; font-size: 0.9rem;
}
.btn {
    padding: 8px 16px; border: none; border-radius: 6px; cursor: pointer;
    font-size: 0.85rem; font-weight: 600; transition: opacity 0.2s;
}
.btn:hover { opacity: 0.85; }
.btn-start { background: #27ae60; color: #fff; }
.btn-stop { background: #c0392b; color: #fff; }
.btn-set { background: #2980b9; color: #fff; }
.btn-auto { background: #8e44ad; color: #fff; }
.btn-tune { background: #e67e22; color: #fff; }
.btn-group { display: flex; gap: 8px; margin-bottom: 12px; flex-wrap: wrap; }

.profile-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(140px, 1fr)); gap: 8px; margin-bottom: 12px; }
.profile-btn {
    padding: 10px 8px; border: 1px solid #0f3460; border-radius: 8px;
    background: #0f3460; color: #e0e0e0; cursor: pointer; text-align: center;
    font-size: 0.8rem; transition: all 0.2s;
}
.profile-btn:hover { border-color: #e74c3c; }
.profile-btn.active { border-color: #e74c3c; background: #1a3a5c; }
.profile-btn .pname { font-weight: 600; margin-bottom: 2px; }
.profile-btn .pinfo { font-size: 0.7rem; color: #7f8c8d; }

.at-result {
    background: #0f3460; border-radius: 8px; padding: 12px; margin-top: 8px;
    font-size: 0.85rem;
}
.at-result .at-vals { color: #27ae60; font-family: monospace; margin: 4px 0; }
.zone-label { font-size: 0.75rem; color: #8e44ad; margin-bottom: 8px; }

@media (max-width: 480px) {
    .temp-cards { grid-template-columns: 1fr; }
    .temp-card .value { font-size: 2rem; }
    .profile-grid { grid-template-columns: 1fr 1fr; }
}
</style>
</head>
<body>

<div class="header">
    <h1>BBQTrak</h1>
    <div class="subtitle">Smoker Controller</div>
</div>

<div class="container">
    <div class="status-bar">
        <span id="statusBadge" class="badge stopped">STOPPED</span>
        <span id="lidBadge" class="badge lid-open" style="display:none">LID OPEN</span>
        <span id="rampBadge" class="badge ramp" style="display:none">RAMP DOWN</span>
        <span id="atBadge" class="badge autotune" style="display:none">AUTOTUNING</span>
        <span id="pitBadge" class="badge disconnected">PIT: --</span>
        <span id="meatBadge" class="badge disconnected">MEAT: --</span>
    </div>

    <div class="temp-cards">
        <div class="temp-card pit">
            <div class="label">Pit Temperature</div>
            <div class="value" id="pitTemp">--</div>
            <div class="unit">&deg;C</div>
            <div class="target">Target: <span id="pitTarget">--</span>&deg;C</div>
            <div class="eff-target" id="effTarget" style="display:none">Effective: <span id="effTargetVal">--</span>&deg;C</div>
        </div>
        <div class="temp-card meat">
            <div class="label">Meat Temperature</div>
            <div class="value" id="meatTemp">--</div>
            <div class="unit">&deg;C</div>
            <div class="target">Target: <span id="meatTarget">--</span>&deg;C</div>
        </div>
    </div>

    <div class="fan-card">
        <div class="fan-label">
            <span>Fan Speed</span>
            <span id="fanSpeed">0%</span>
        </div>
        <div class="fan-bar-container">
            <div class="fan-bar" id="fanBar" style="width:0%"></div>
        </div>
        <div class="fan-label">
            <span>PID Output: <span id="pidOut">0</span>%</span>
            <span id="zoneLabel" class="zone-label"></span>
        </div>
    </div>

    <div class="chart-card">
        <div class="chart-duration" id="durationBar">
            <button class="active" onclick="setDuration(60)">1m</button>
            <button onclick="setDuration(300)">5m</button>
            <button onclick="setDuration(600)">10m</button>
            <button onclick="setDuration(1800)">30m</button>
        </div>
        <canvas id="chart"></canvas>
    </div>

    <div class="controls">
        <h3>Cook Profiles</h3>
        <div class="profile-grid" id="profileGrid"></div>
    </div>

    <div class="controls">
        <h3>Control</h3>
        <div class="btn-group">
            <button class="btn btn-start" onclick="apiCall('/api/start')">Start</button>
            <button class="btn btn-stop" onclick="apiCall('/api/stop')">Stop</button>
            <button class="btn btn-auto" onclick="apiCall('/api/auto')">Auto Mode</button>
        </div>

        <div class="control-row">
            <label>Pit Target:</label>
            <input type="number" id="inPitTarget" step="1" value="121">
            <span>&deg;C</span>
            <button class="btn btn-set" onclick="setTarget()">Set</button>
        </div>
        <div class="control-row">
            <label>Meat Target:</label>
            <input type="number" id="inMeatTarget" step="1" value="91">
            <span>&deg;C</span>
            <button class="btn btn-set" onclick="setMeatTarget()">Set</button>
        </div>
        <div class="control-row">
            <label>Manual Fan:</label>
            <input type="number" id="inFanSpeed" min="0" max="100" step="5" value="0">
            <span>%</span>
            <button class="btn btn-set" onclick="setFan()">Set</button>
        </div>
    </div>

    <div class="controls">
        <h3>PID Tuning</h3>
        <div id="zoneInfo" class="zone-label" style="margin-bottom:8px"></div>
        <div class="control-row">
            <label>Kp:</label>
            <input type="number" id="inKp" step="0.1" value="4.0">
            <label>Ki:</label>
            <input type="number" id="inKi" step="0.001" value="0.02">
            <label>Kd:</label>
            <input type="number" id="inKd" step="0.1" value="10.0">
            <button class="btn btn-set" onclick="setPID()">Apply</button>
        </div>
        <div class="btn-group">
            <button class="btn btn-tune" id="btnAutotune" onclick="toggleAutotune()">Start Autotune</button>
            <button class="btn btn-set" id="btnApplyAT" onclick="applyAutotune()" style="display:none">Apply Result</button>
        </div>
        <div id="atResult" class="at-result" style="display:none">
            <div>Autotune Result:</div>
            <div class="at-vals" id="atVals"></div>
        </div>
    </div>
</div>

<script>
let chartData = { pit: [], meat: [], time: [] };
let chartDuration = 300;
let profiles = [];
let activeProfile = -1;

function apiCall(url) { fetch(url).catch(e => console.error(e)); }

function setTarget() {
    fetch('/api/target?pit=' + document.getElementById('inPitTarget').value);
}
function setMeatTarget() {
    fetch('/api/target?meat=' + document.getElementById('inMeatTarget').value);
}
function setFan() { fetch('/api/fan?speed=' + document.getElementById('inFanSpeed').value); }
function setPID() {
    fetch('/api/pid?kp=' + document.getElementById('inKp').value +
        '&ki=' + document.getElementById('inKi').value +
        '&kd=' + document.getElementById('inKd').value);
}

function selectProfile(id) {
    fetch('/api/profile?id=' + id);
    activeProfile = id;
    renderProfiles();
}

function renderProfiles() {
    let grid = document.getElementById('profileGrid');
    grid.innerHTML = '';
    profiles.forEach(p => {
        let div = document.createElement('div');
        div.className = 'profile-btn' + (p.id === activeProfile ? ' active' : '');
        div.innerHTML = '<div class="pname">' + p.name + '</div>' +
            '<div class="pinfo">Pit: ' + Math.round(p.pit) + '&deg;C / Meat: ' + Math.round(p.meat) + '&deg;C</div>';
        div.onclick = () => selectProfile(p.id);
        grid.appendChild(div);
    });
}

function toggleAutotune() {
    let btn = document.getElementById('btnAutotune');
    fetch('/api/autotune?action=' + (btn.textContent.includes('Start') ? 'start' : 'cancel'));
}
function applyAutotune() { fetch('/api/autotune?action=apply'); }

function updateStatus() {
    fetch('/api/status').then(r => r.json()).then(d => {
        document.getElementById('pitTemp').textContent = d.pit.toFixed(1);
        document.getElementById('meatTemp').textContent = d.meat.toFixed(1);
        document.getElementById('pitTarget').textContent = Math.round(d.target);
        document.getElementById('meatTarget').textContent = Math.round(d.meat_target);
        document.getElementById('fanSpeed').textContent = Math.round(d.fan) + '%';
        document.getElementById('fanBar').style.width = d.fan + '%';
        document.getElementById('pidOut').textContent = Math.round(d.pid_output);
        document.getElementById('zoneLabel').textContent = d.pid_zone || '';
        document.getElementById('zoneInfo').textContent = 'Active zone: ' + (d.pid_zone || 'Custom');

        let sb = document.getElementById('statusBadge');
        sb.textContent = d.running ? 'RUNNING' : 'STOPPED';
        sb.className = 'badge ' + (d.running ? 'running' : 'stopped');

        document.getElementById('lidBadge').style.display = d.lid_open ? '' : 'none';
        document.getElementById('rampBadge').style.display = d.ramping_down ? '' : 'none';

        let effEl = document.getElementById('effTarget');
        if (d.ramping_down) {
            effEl.style.display = '';
            document.getElementById('effTargetVal').textContent = d.eff_target.toFixed(1);
        } else {
            effEl.style.display = 'none';
        }

        document.getElementById('atBadge').style.display = d.autotuning ? '' : 'none';
        document.getElementById('btnAutotune').textContent = d.autotuning ? 'Cancel Autotune' : 'Start Autotune';

        if (d.autotune_done && d.autotune_result && d.autotune_result.valid) {
            document.getElementById('atResult').style.display = '';
            document.getElementById('btnApplyAT').style.display = '';
            document.getElementById('atVals').textContent =
                'Kp=' + d.autotune_result.kp.toFixed(3) +
                '  Ki=' + d.autotune_result.ki.toFixed(4) +
                '  Kd=' + d.autotune_result.kd.toFixed(3) +
                '  (Ku=' + d.autotune_result.ku.toFixed(3) +
                ', Tu=' + d.autotune_result.tu.toFixed(1) + 's)';
        } else {
            document.getElementById('atResult').style.display = 'none';
            document.getElementById('btnApplyAT').style.display = 'none';
        }

        let pb = document.getElementById('pitBadge');
        pb.textContent = 'PIT: ' + (d.pit_connected ? 'OK' : 'N/C');
        pb.className = 'badge ' + (d.pit_connected ? 'connected' : 'disconnected');
        let mb = document.getElementById('meatBadge');
        mb.textContent = 'MEAT: ' + (d.meat_connected ? 'OK' : 'N/C');
        mb.className = 'badge ' + (d.meat_connected ? 'connected' : 'disconnected');

        document.getElementById('inKp').value = d.kp;
        document.getElementById('inKi').value = d.ki;
        document.getElementById('inKd').value = d.kd;

        activeProfile = d.profile;
        renderProfiles();
    }).catch(() => {});
}

function setDuration(secs) {
    chartDuration = secs;
    document.querySelectorAll('.chart-duration button').forEach(b => b.classList.remove('active'));
    event.target.classList.add('active');
    drawChart();
}

function drawChart() {
    fetch('/api/history?limit=' + chartDuration).then(r => r.json()).then(d => {
        chartData = d; renderChart();
    }).catch(() => {});
}

function renderChart() {
    const canvas = document.getElementById('chart');
    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    canvas.width = rect.width * dpr;
    canvas.height = 200 * dpr;
    ctx.scale(dpr, dpr);
    const W = rect.width, H = 200;

    ctx.fillStyle = '#16213e';
    ctx.fillRect(0, 0, W, H);

    if (!chartData.pit || chartData.pit.length < 2) {
        ctx.fillStyle = '#7f8c8d'; ctx.font = '14px sans-serif'; ctx.textAlign = 'center';
        ctx.fillText('Waiting for data...', W/2, H/2);
        return;
    }

    const pad = { t: 20, r: 10, b: 18, l: 40 };
    const cW = W - pad.l - pad.r, cH = H - pad.t - pad.b;
    const all = chartData.pit.concat(chartData.meat).filter(v => v > 0);
    if (all.length === 0) return;

    let minV = Math.min(...all) - 5, maxV = Math.max(...all) + 5;
    if (maxV - minV < 10) { minV -= 5; maxV += 5; }

    ctx.strokeStyle = '#0f3460'; ctx.lineWidth = 1;
    for (let i = 0; i <= 4; i++) {
        let y = pad.t + cH - (i/4) * cH;
        ctx.beginPath(); ctx.moveTo(pad.l, y); ctx.lineTo(W - pad.r, y); ctx.stroke();
        ctx.fillStyle = '#7f8c8d'; ctx.font = '10px sans-serif'; ctx.textAlign = 'right';
        ctx.fillText(Math.round(minV + (maxV-minV) * i/4) + '\u00B0C', pad.l - 4, y + 3);
    }

    function drawLine(vals, color) {
        ctx.strokeStyle = color; ctx.lineWidth = 2; ctx.beginPath();
        for (let i = 0; i < vals.length; i++) {
            let x = pad.l + (i / (vals.length - 1)) * cW;
            let y = pad.t + cH - ((vals[i] - minV) / (maxV - minV)) * cH;
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
        }
        ctx.stroke();
    }
    drawLine(chartData.pit, '#e74c3c');
    drawLine(chartData.meat, '#f39c12');

    // X-axis time labels
    ctx.fillStyle = '#7f8c8d'; ctx.font = '10px sans-serif'; ctx.textAlign = 'center';
    let tLen = chartData.time.length;
    if (tLen >= 2) {
        let t0 = chartData.time[0], t1 = chartData.time[tLen - 1];
        let span = t1 - t0;
        for (let i = 0; i <= 4; i++) {
            let x = pad.l + (i / 4) * cW;
            let ago = Math.round(span * (1 - i / 4));
            let lbl = ago >= 60 ? Math.round(ago / 60) + 'm' : ago + 's';
            if (i === 4) lbl = 'now';
            ctx.fillText(i === 4 ? lbl : '-' + lbl, x, H - 2);
        }
    }

    ctx.font = '11px sans-serif'; ctx.textAlign = 'left';
    ctx.fillStyle = '#e74c3c'; ctx.fillText('Pit', W - pad.r - 70, pad.t + 14);
    ctx.fillStyle = '#f39c12'; ctx.fillText('Meat', W - pad.r - 35, pad.t + 14);
}

fetch('/api/profiles').then(r => r.json()).then(p => { profiles = p; renderProfiles(); });
setInterval(updateStatus, 2000);
setInterval(drawChart, 5000);
updateStatus();
drawChart();
</script>
</body>
</html>
)rawliteral";
