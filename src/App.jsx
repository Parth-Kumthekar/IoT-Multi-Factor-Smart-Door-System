import React, { useEffect, useMemo, useState } from "react";
import {
  getStatus,
  getLogs,
  armSystem,
  disarmSystem,
  lockDoor,
  unlockDoor,
  triggerAlarm,
  clearAlarm,
  openDoor,
  closeDoor,
  authorizeAccess,
  formatTime,
  formatUptime
} from "./api";

export default function App() {
  const [state, setState] = useState(null);
  const [logs, setLogs] = useState([]);
  const [loading, setLoading] = useState(true);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");
  const [authorizedUser, setAuthorizedUser] = useState("student_01");
  const [autoRefresh, setAutoRefresh] = useState(true);

  const isHealthy = useMemo(() => {
    if (!state) return false;
    return (
      state.apiOnline &&
      state.cameraOnline &&
      state.nfcOnline &&
      state.gpioOnline
    );
  }, [state]);

  async function refreshAll(showLoading = false) {
    try {
      if (showLoading) setLoading(true);
      setError("");

      const [statusData, logsData] = await Promise.all([
        getStatus(),
        getLogs(20)
      ]);

      setState(statusData);
      setLogs(logsData);
    } catch (err) {
      setError(err.message || "Failed to load dashboard");
    } finally {
      if (showLoading) setLoading(false);
    }
  }

  useEffect(() => {
    refreshAll(true);
  }, []);

  useEffect(() => {
    if (!autoRefresh) return;

    const timer = setInterval(() => {
      refreshAll(false);
    }, 2000);

    return () => clearInterval(timer);
  }, [autoRefresh]);

  async function runAction(action) {
    try {
      setBusy(true);
      setError("");
      await action();
      await refreshAll(false);
    } catch (err) {
      setError(err.message || "Action failed");
    } finally {
      setBusy(false);
    }
  }

  if (loading) {
    return (
      <div className="page">
        <div className="container">
          <div className="panel">Loading dashboard...</div>
        </div>
      </div>
    );
  }

  return (
    <div className="page">
      <div className="container">
        <header className="topbar">
          <div>
            <h1>Door Intrusion Alarm System Demo Dashboard</h1>
            <p>
              Realtime embedded security prototype for coursework demonstration.
            </p>
          </div>

          <div className="topbar-actions">
            <label className="toggle-box">
              <span>Auto refresh</span>
              <input
                type="checkbox"
                checked={autoRefresh}
                onChange={(e) => setAutoRefresh(e.target.checked)}
              />
            </label>

            <button className="btn btn-outline" onClick={() => refreshAll(false)}>
              Refresh
            </button>
          </div>
        </header>

        {error && <div className="alert error">{error}</div>}

        {state?.alarmActive && (
          <div className="alert danger">
            Alarm is active. Check the latest events before clearing it.
          </div>
        )}

        <section className="summary-grid">
          <div className="panel">
            <h3>Coursework Demonstration Summary</h3>
            <div className="summary-cards">
              <div className="summary-box">
                <h4>Realtime goal</h4>
                <p>
                  Detect a door event quickly, evaluate authorization within a
                  short verification window, and trigger alarm response without
                  blocking the main control path.
                </p>
              </div>

              <div className="summary-box">
                <h4>Core architecture</h4>
                <p>
                  GPIO and sensor events are handled by the C++ control logic,
                  while this dashboard supports observation, testing, and remote
                  actions.
                </p>
              </div>

              <div className="summary-box">
                <h4>Assessment focus</h4>
                <p>
                  This interface helps present system state, event flow, alarm
                  behaviour, and integration quality during the coursework demo.
                </p>
              </div>
            </div>
          </div>

          <div className="panel">
            <h3>Suggested Demo Flow</h3>
            <div className="flow-list">
              <div>1. Show the system starts armed and healthy.</div>
              <div>2. Simulate authorized access using NFC / Face / API.</div>
              <div>3. Simulate unauthorized door opening.</div>
              <div>4. Show pending verification and timeout.</div>
              <div>5. Show alarm activation and event logging.</div>
              <div>6. Clear alarm and explain system integration.</div>
            </div>
          </div>
        </section>

        {state && (
          <section className="card-grid">
            <div className="panel">
              <h3>System Status</h3>
              <div className="info-list">
                <div><span>Armed</span><strong>{state.systemArmed ? "YES" : "NO"}</strong></div>
                <div><span>Alarm</span><strong>{state.alarmActive ? "ACTIVE" : "CLEAR"}</strong></div>
                <div><span>Door</span><strong>{state.doorOpen ? "OPEN" : "CLOSED"}</strong></div>
                <div><span>Lock</span><strong>{state.lockState.toUpperCase()}</strong></div>
                <div><span>Buzzer</span><strong>{state.buzzerOn ? "ON" : "OFF"}</strong></div>
                <div><span>LED</span><strong>{state.ledStatus.toUpperCase()}</strong></div>
                <div><span>Pending verification</span><strong>{state.pendingVerification ? "YES" : "NO"}</strong></div>
                <div><span>Uptime</span><strong>{formatUptime(state.uptimeSeconds)}</strong></div>
              </div>
            </div>

            <div className="panel">
              <h3>Latest Authorization</h3>
              <div className="info-list">
                <div><span>Method</span><strong>{state.lastAuthorizedMethod || "NONE"}</strong></div>
                <div><span>User</span><strong>{state.lastAuthorizedUser || "—"}</strong></div>
                <div><span>Time</span><strong>{formatTime(state.lastAuthorizationTime)}</strong></div>
              </div>
            </div>

            <div className="panel">
              <h3>Device Health</h3>
              <div className="info-list">
                <div><span>GPIO</span><strong>{state.gpioOnline ? "ONLINE" : "OFFLINE"}</strong></div>
                <div><span>NFC</span><strong>{state.nfcOnline ? "ONLINE" : "OFFLINE"}</strong></div>
                <div><span>Camera</span><strong>{state.cameraOnline ? "ONLINE" : "OFFLINE"}</strong></div>
                <div><span>API</span><strong>{state.apiOnline ? "ONLINE" : "OFFLINE"}</strong></div>
                <div><span>Overall</span><strong>{isHealthy ? "HEALTHY" : "CHECK"}</strong></div>
              </div>
            </div>
          </section>
        )}

        <section className="main-grid">
          <div className="panel">
            <h3>Demonstration Control Panel</h3>

            <div className="button-grid">
              <button className="btn" disabled={busy} onClick={() => runAction(() => armSystem())}>
                Arm System
              </button>

              <button className="btn btn-secondary" disabled={busy} onClick={() => runAction(() => disarmSystem())}>
                Disarm System
              </button>

              <button className="btn btn-outline" disabled={busy} onClick={() => runAction(() => lockDoor())}>
                Lock Door
              </button>

              <button className="btn btn-outline" disabled={busy} onClick={() => runAction(() => unlockDoor())}>
                Unlock Door
              </button>

              <button className="btn btn-danger" disabled={busy} onClick={() => runAction(() => triggerAlarm("Manual dashboard trigger"))}>
                Trigger Alarm
              </button>

              <button className="btn btn-outline" disabled={busy} onClick={() => runAction(() => clearAlarm())}>
                Clear Alarm
              </button>
            </div>

            <hr />

            <h3>Demonstration Actions</h3>

            <div className="button-grid">
              <button className="btn btn-outline" disabled={busy} onClick={() => runAction(() => openDoor())}>
                Simulate Door Open
              </button>

              <button className="btn btn-outline" disabled={busy} onClick={() => runAction(() => closeDoor())}>
                Simulate Door Close
              </button>
            </div>

            <div className="input-box">
              <label>Authorized User</label>
              <input
                value={authorizedUser}
                onChange={(e) => setAuthorizedUser(e.target.value)}
                placeholder="student_01"
              />
            </div>

            <div className="button-grid">
              <button className="btn" disabled={busy} onClick={() => runAction(() => authorizeAccess("nfc", authorizedUser))}>
                Authorize via NFC
              </button>

              <button className="btn" disabled={busy} onClick={() => runAction(() => authorizeAccess("face", authorizedUser))}>
                Authorize via Face
              </button>

              <button className="btn" disabled={busy} onClick={() => runAction(() => authorizeAccess("api", authorizedUser))}>
                Authorize via API
              </button>
            </div>
          </div>

          <div className="panel">
            <h3>Demonstration Event Log</h3>

            <div className="log-list">
              {logs.length === 0 && <div className="log-item">No logs available yet.</div>}

              {logs.map((log, index) => (
                <div className="log-item" key={log.id || index}>
                  <div className="log-head">
                    <strong>{(log.type || "log").toUpperCase()}</strong>
                    <span>{formatTime(log.time)}</span>
                  </div>

                  <div className="log-message">{log.message || "No message"}</div>

                  <div className="log-tags">
                    {log.reason && <span>reason: {log.reason}</span>}
                    {log.user && <span>user: {log.user}</span>}
                    {log.method && <span>method: {log.method}</span>}
                  </div>
                </div>
              ))}
            </div>
          </div>
        </section>
      </div>
    </div>
  );
}