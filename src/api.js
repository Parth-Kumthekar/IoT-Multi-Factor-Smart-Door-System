const API_BASE = "http://192.168.1.22:3000";

async function rawGet(path) {
  const res = await fetch(`${API_BASE}${path}`);
  if (!res.ok) {
    throw new Error(`GET ${path} failed with ${res.status}`);
  }
  return res.json();
}

async function rawPost(path, body) {
  const res = await fetch(`${API_BASE}${path}`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: body ? JSON.stringify(body) : undefined
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || `POST ${path} failed with ${res.status}`);
  }

  return res.json();
}

export async function getStatus() {
  const raw = await rawGet("/api/status");
  const payload = raw.data ? raw.data : raw;

  return {
    systemArmed: payload.systemArmed ?? false,
    alarmActive: payload.alarmActive ?? false,
    doorOpen: payload.doorOpen ?? false,
    lockState: payload.lockState ?? "locked",
    lastAuthorizedMethod:
      payload.lastAuthorizedMethod && payload.lastAuthorizedMethod !== "null"
        ? payload.lastAuthorizedMethod
        : null,
    lastAuthorizedUser:
      payload.lastAuthorizedUser && payload.lastAuthorizedUser !== "null"
        ? payload.lastAuthorizedUser
        : null,
    lastAuthorizationTime:
      payload.lastAuthorizationTime && payload.lastAuthorizationTime !== "null"
        ? payload.lastAuthorizationTime
        : null,
    pendingVerification: payload.pendingVerification ?? false,
    intrusionDetected: payload.intrusionDetected ?? false,
    buzzerOn: payload.buzzerOn ?? false,
    ledStatus: payload.ledStatus ?? "green",
    cameraOnline: payload.cameraOnline ?? true,
    nfcOnline: payload.nfcOnline ?? true,
    gpioOnline: payload.gpioOnline ?? true,
    apiOnline: payload.apiOnline ?? true,
    uptimeSeconds: payload.uptimeSeconds ?? 0
  };
}

export async function getLogs(limit = 20) {
  const raw = await rawGet(`/api/logs?limit=${limit}`);

  const logs = Array.isArray(raw.data) ? raw.data : [];

  return logs.map((log) => {
    let reason = "";
    let user = "";
    let method = "";

    if (log.type === "access" && log.extra && log.extra.includes(":")) {
      const parts = log.extra.split(":");
      method = parts[0] || "";
      user = parts.slice(1).join(":") || "";
    } else if (log.extra) {
      reason = log.extra;
    }

    return {
      id: log.id,
      time: log.time,
      type: log.type,
      message: log.message,
      extra: log.extra,
      reason,
      user,
      method
    };
  });
}

export async function armSystem() {
  return rawPost("/api/system/arm");
}

export async function disarmSystem() {
  return rawPost("/api/system/disarm");
}

export async function lockDoor() {
  return rawPost("/api/system/lock");
}

export async function unlockDoor() {
  return rawPost("/api/system/unlock");
}

export async function triggerAlarm(reason) {
  return rawPost("/api/alarm/trigger", { reason });
}

export async function clearAlarm() {
  return rawPost("/api/alarm/clear");
}

export async function openDoor() {
  return rawPost("/api/door/open");
}

export async function closeDoor() {
  return rawPost("/api/door/close");
}

export async function authorizeAccess(method, user) {
  return rawPost("/api/access/authorize", { method, user });
}

export function formatTime(value) {
  if (!value) return "—";
  return new Date(value).toLocaleString();
}

export function formatUptime(totalSeconds) {
  if (typeof totalSeconds !== "number") return "—";
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;
  return `${hours}h ${minutes}m ${seconds}s`;
}