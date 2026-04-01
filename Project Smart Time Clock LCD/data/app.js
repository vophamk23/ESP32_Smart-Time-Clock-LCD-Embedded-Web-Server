/* ============================================================
   app.js — Smart Clock: API logic + UI panel switching
   ============================================================ */
"use strict";

// ── Client-side interpolation state ──
let lastData = null;
let swRunning = false,
  swElapsed = 0,
  swStartedAt = 0;
let cdRunning = false,
  cdRemaining = 0,
  cdStartedAt = 0;

// ── Panel/action mapping ──
const panelMap = { 0: "sensor", 1: "clock", 2: "alarm", 3: "sw", 4: "cd" };
const actionMap = { 0: "none", 1: "none", 2: "alarm", 3: "sw", 4: "cd" };
let currentMode = 1;

// ============================================================
// UTILITIES
// ============================================================
function pad(n, len = 2) {
  return String(Math.floor(n)).padStart(len, "0");
}

function fmtMs(ms) {
  ms = Math.max(0, ms);
  return `${pad(Math.floor(ms / 3600000) % 100)}:${pad(Math.floor(ms / 60000) % 60)}:${pad(Math.floor(ms / 1000) % 60)}.${pad(Math.floor(ms / 10) % 100)}`;
}

function fmtMsShort(ms) {
  ms = Math.max(0, ms);
  return `${pad(Math.floor(ms / 3600000) % 100)}:${pad(Math.floor(ms / 60000) % 60)}:${pad(Math.floor(ms / 1000) % 60)}`;
}

function showToast(msg, color = "#39ff14") {
  const t = document.getElementById("toast");
  t.textContent = msg;
  t.style.borderColor = color;
  t.style.color = color;
  t.classList.remove("hidden");
  clearTimeout(t._timer);
  t._timer = setTimeout(() => t.classList.add("hidden"), 2500);
}

// ============================================================
// API
// ============================================================
async function api(path, method = "GET", body = null) {
  try {
    const opts = { method, headers: { "Content-Type": "application/json" } };
    if (body) opts.body = JSON.stringify(body);
    const r = await fetch(path, opts);
    return await r.json();
  } catch (e) {
    showToast("Lỗi kết nối!", "#ff3333");
    return null;
  }
}

// ============================================================
// UI PANEL SWITCHING
// ============================================================
function selectMode(m) {
  // Swap LCD panel
  document
    .querySelectorAll(".panel")
    .forEach((p) => p.classList.remove("active"));
  const panel = document.getElementById("panel-" + panelMap[m]);
  if (panel) panel.classList.add("active");

  // Swap mode button highlight
  document
    .querySelectorAll(".mode-btn")
    .forEach((b) =>
      b.classList.toggle("active", parseInt(b.dataset.mode) === m),
    );

  // Swap action controls
  document
    .querySelectorAll(".action-group")
    .forEach((g) => g.classList.add("hidden"));
  const act = document.getElementById("action-" + actionMap[m]);
  if (act) act.classList.remove("hidden");

  currentMode = m;
  setMode(m);
}

// ============================================================
// UPDATE UI FROM SERVER DATA
// ============================================================
function updateUI(d) {
  lastData = d;
  const t = d.time;

  // Header sensor
  el("tempVal").textContent = d.sensor.temp.toFixed(1);
  el("humiVal").textContent = d.sensor.humidity.toFixed(1);

  // System info
  el("heapInfo").textContent = `HEAP:${(d.system.heap / 1024).toFixed(1)}KB`;
  const up = d.system.uptime;
  el("uptimeInfo").textContent =
    `UP:${pad(Math.floor(up / 3600))}:${pad(Math.floor(up / 60) % 60)}:${pad(up % 60)}`;

  // Clock panel
  el("clockDisplay").textContent =
    `${pad(t.hour)}:${pad(t.minute)}:${pad(t.second)}`;
  el("dateDisplay").textContent =
    `${pad(t.day)}/${pad(t.month)}/${t.year} ${t.weekday}`;

  // Sensor panel — format as single string, no split rendering
  el("tempVal2").textContent = `TEMP: ${d.sensor.temp.toFixed(1)} \u00b0C`;
  el("humiVal2").textContent = `HUMI: ${d.sensor.humidity.toFixed(1)}%`;

  // Alarm panel
  el("alarmDisplay").textContent =
    `${pad(d.alarm.hour)}:${pad(d.alarm.minute)}`;
  const alarmPanel = el("panel-alarm");
  if (d.alarm.triggered) {
    el("alarmStatus").innerHTML = '<span style="color:#ff3333">RINGING!</span>';
    alarmPanel.classList.add("alarm-ring");
  } else {
    el("alarmStatus").textContent = "WAITING...";
    alarmPanel.classList.remove("alarm-ring");
  }
  if (document.activeElement !== el("alarmH"))
    el("alarmH").value = d.alarm.hour;
  if (document.activeElement !== el("alarmM"))
    el("alarmM").value = d.alarm.minute;

  // Stopwatch state
  swRunning = d.stopwatch.running;
  if (swRunning) {
    swStartedAt = performance.now() - d.stopwatch.elapsed;
  } else {
    swElapsed = d.stopwatch.elapsed;
  }

  // Lap list
  const lapEl = el("lapsList");
  if (d.stopwatch.laps && d.stopwatch.laps.length) {
    lapEl.innerHTML = d.stopwatch.laps
      .map(
        (l, i) =>
          `<div class="lap-item"><span>LAP ${i + 1}</span><span>${fmtMs(l)}</span></div>`,
      )
      .join("");
  } else {
    lapEl.innerHTML = "";
  }

  // Countdown state
  cdRunning = d.countdown.running;
  if (cdRunning) {
    cdRemaining = d.countdown.remaining;
    cdStartedAt = performance.now();
  } else {
    cdRemaining = d.countdown.remaining;
  }

  const cdPanel = el("panel-cd");
  if (d.countdown.triggered) {
    cdPanel.classList.add("cd-triggered");
    cdPanel.classList.remove("cd-running");
    el("cdStatus").textContent = "TIME'S UP!";
  } else if (d.countdown.running) {
    cdPanel.classList.remove("cd-triggered");
    cdPanel.classList.add("cd-running");
    el("cdStatus").textContent = "RUNNING...";
  } else {
    cdPanel.classList.remove("cd-triggered", "cd-running");
    el("cdStatus").textContent = d.countdown.editing ? "SET TIME" : "READY";
  }

  if (d.countdown.editing) {
    if (document.activeElement !== el("cdH"))
      el("cdH").value = d.countdown.editHours;
    if (document.activeElement !== el("cdM"))
      el("cdM").value = d.countdown.editMinutes;
    if (document.activeElement !== el("cdS"))
      el("cdS").value = d.countdown.editSeconds;
  }

  // Sync mode button if server changed mode (e.g. via physical button)
  if (d.mode !== currentMode) {
    currentMode = d.mode;
    document
      .querySelectorAll(".panel")
      .forEach((p) => p.classList.remove("active"));
    const p = document.getElementById("panel-" + panelMap[d.mode]);
    if (p) p.classList.add("active");
    document
      .querySelectorAll(".mode-btn")
      .forEach((b) =>
        b.classList.toggle("active", parseInt(b.dataset.mode) === d.mode),
      );
    document
      .querySelectorAll(".action-group")
      .forEach((g) => g.classList.add("hidden"));
    const a = document.getElementById("action-" + actionMap[d.mode]);
    if (a) a.classList.remove("hidden");
  }
}

function el(id) {
  return document.getElementById(id);
}

// ============================================================
// RENDER LOOP 60fps
// ============================================================
function renderLoop() {
  // Stopwatch interpolation
  const swTime = swRunning ? performance.now() - swStartedAt : swElapsed;
  el("swDisplay").textContent = fmtMs(swTime);
  el("panel-sw").classList.toggle("sw-running", swRunning);

  // Countdown interpolation
  const cdLeft = cdRunning
    ? Math.max(0, cdRemaining - (performance.now() - cdStartedAt))
    : cdRemaining;
  el("cdDisplay").textContent = fmtMsShort(cdLeft);

  requestAnimationFrame(renderLoop);
}

// ============================================================
// FETCH STATUS
// ============================================================
async function fetchStatus() {
  const d = await api("/api/status");
  if (d) updateUI(d);
}

// ============================================================
// CONTROLS — called by onclick in HTML
// ============================================================
async function setMode(m) {
  await api("/api/mode", "POST", { mode: m });
}

async function setAlarm() {
  const h = parseInt(el("alarmH").value);
  const m = parseInt(el("alarmM").value);
  const r = await api("/api/alarm", "POST", { hour: h, minute: m });
  if (r?.ok) showToast(`ALARM SET: ${pad(h)}:${pad(m)}`);
  fetchStatus();
}

async function stopAlarm() {
  await api("/api/alarm/stop", "POST");
  showToast("ALARM OFF", "#00ff9f");
  fetchStatus();
}

async function swStart() {
  await api("/api/stopwatch/start", "POST");
  swRunning = true;
  swStartedAt = performance.now() - swElapsed;
}

async function swStop() {
  await api("/api/stopwatch/stop", "POST");
  swRunning = false;
  fetchStatus();
}

async function swReset() {
  await api("/api/stopwatch/reset", "POST");
  swRunning = false;
  swElapsed = 0;
  el("lapsList").innerHTML = "";
}

async function cdSet() {
  const h = parseInt(el("cdH").value) || 0;
  const m = parseInt(el("cdM").value) || 0;
  const s = parseInt(el("cdS").value) || 0;
  await api("/api/countdown/set", "POST", { hours: h, minutes: m, seconds: s });
  showToast(`CD SET: ${pad(h)}:${pad(m)}:${pad(s)}`);
}

async function cdStart() {
  const r = await api("/api/countdown/start", "POST");
  if (r?.ok) {
    cdRunning = true;
    cdStartedAt = performance.now();
    showToast("COUNTDOWN START!");
  } else {
    showToast("SET TIME FIRST!", "#ffb700");
  }
  fetchStatus();
}

async function cdStop() {
  await api("/api/countdown/stop", "POST");
  cdRunning = false;
  fetchStatus();
}

async function cdReset() {
  await api("/api/countdown/reset", "POST");
  cdRunning = false;
  cdRemaining = 0;
  fetchStatus();
}

// ============================================================
// INIT
// ============================================================
document.addEventListener("DOMContentLoaded", () => {
  // Set initial panel to clock
  selectMode(1);
  fetchStatus();
  setInterval(fetchStatus, 1000);
  requestAnimationFrame(renderLoop);
});
