#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "httplib.h"
#include "EmailAlert.h"

struct LogEntry {
    std::string id;
    std::string time;
    std::string type;
    std::string message;
    std::string extra;
};

struct SystemState {
    bool systemArmed = true;
    bool alarmActive = false;
    bool doorOpen = false;
    std::string lockState = "locked";

    std::string lastAuthorizedMethod = "null";
    std::string lastAuthorizedUser = "null";
    std::string lastAuthorizationTime = "null";

    bool pendingVerification = false;
    bool intrusionDetected = false;
    bool buzzerOn = false;
    std::string ledStatus = "green";

    bool cameraOnline = true;
    bool nfcOnline = true;
    bool gpioOnline = true;
    bool apiOnline = true;

    std::chrono::system_clock::time_point startedAt = std::chrono::system_clock::now();
};

namespace {
    std::mutex g_mutex;
    SystemState g_state;
    std::vector<LogEntry> g_logs;

    constexpr int MAX_LOGS = 300;
    constexpr int VERIFICATION_WINDOW_SECONDS = 5;

    std::string getEnvOrEmpty(const char* key) {
        const char* value = std::getenv(key);
        return value ? std::string(value) : std::string();
    }

    int getEnvOrDefaultInt(const char* key, int defaultValue) {
        const char* value = std::getenv(key);
        if (!value) return defaultValue;

        try {
            return std::stoi(value);
        } catch (...) {
            return defaultValue;
        }
    }

    EmailAlert g_emailAlert(
        getEnvOrEmpty("ALERT_SMTP_SERVER"),
        getEnvOrDefaultInt("ALERT_SMTP_PORT", 587),
        getEnvOrEmpty("ALERT_EMAIL"),
        getEnvOrEmpty("ALERT_EMAIL_PASSWORD"),
        getEnvOrEmpty("ALERT_EMAIL_RECEIVER")
    );

    std::string nowIso8601() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
#ifdef _WIN32
        gmtime_s(&tm, &t);
#else
        gmtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        return oss.str();
    }

    std::string jsonEscape(const std::string& input) {
        std::ostringstream oss;

        for (char c : input) {
            switch (c) {
                case '\"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        oss << ' ';
                    } else {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    std::string boolToJson(bool value) {
        return value ? "true" : "false";
    }

    long long getUptimeSeconds() {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - g_state.startedAt
        );
        return duration.count();
    }

    void addLog(const std::string& type, const std::string& message, const std::string& extra = "") {
        LogEntry entry;
        entry.id = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        entry.time = nowIso8601();
        entry.type = type;
        entry.message = message;
        entry.extra = extra;

        g_logs.insert(g_logs.begin(), entry);

        if (static_cast<int>(g_logs.size()) > MAX_LOGS) {
            g_logs.pop_back();
        }

        std::cout << "[" << entry.time << "] [" << entry.type << "] " << entry.message;
        if (!entry.extra.empty()) {
            std::cout << " (" << entry.extra << ")";
        }
        std::cout << std::endl;
    }

    void updateLedStatus() {
        if (g_state.alarmActive) {
            g_state.ledStatus = "red";
        } else if (g_state.pendingVerification) {
            g_state.ledStatus = "yellow";
        } else {
            g_state.ledStatus = "green";
        }
    }

    void triggerAlarm(const std::string& reason) {
        g_state.alarmActive = true;
        g_state.intrusionDetected = true;
        g_state.buzzerOn = true;
        g_state.pendingVerification = false;
        updateLedStatus();
        addLog("alarm", "Alarm triggered", reason);

        std::thread([reason]() {
            std::string subject = "Door Intrusion Alarm Alert";

            std::ostringstream body;
            body << "Door Intrusion Alarm Triggered\n\n";
            body << "Time: " << nowIso8601() << "\n";
            body << "Reason: " << reason << "\n";

            {
                std::lock_guard<std::mutex> lock(g_mutex);
                body << "System Armed: " << (g_state.systemArmed ? "true" : "false") << "\n";
                body << "Door Open: " << (g_state.doorOpen ? "true" : "false") << "\n";
                body << "Intrusion Detected: " << (g_state.intrusionDetected ? "true" : "false") << "\n";
                body << "Lock State: " << g_state.lockState << "\n";
                body << "LED Status: " << g_state.ledStatus << "\n";
            }

            bool ok = g_emailAlert.sendIntrusionAlert(subject, body.str());

            std::lock_guard<std::mutex> lock(g_mutex);
            if (ok) {
                addLog("email", "Email alert sent");
            } else {
                addLog("email", "Email alert failed");
            }
        }).detach();
    }

    void clearAlarm() {
        g_state.alarmActive = false;
        g_state.intrusionDetected = false;
        g_state.buzzerOn = false;
        updateLedStatus();
        addLog("alarm", "Alarm cleared");
    }

    void authorizeAccess(const std::string& method, const std::string& user) {
        g_state.lastAuthorizedMethod = method;
        g_state.lastAuthorizedUser = user;
        g_state.lastAuthorizationTime = nowIso8601();
        g_state.pendingVerification = false;
        g_state.intrusionDetected = false;
        updateLedStatus();
        addLog("access", "Access authorized", method + ":" + user);
    }

    std::string buildStateJson() {
        std::ostringstream oss;

        oss << "{";
        oss << "\"systemArmed\":" << boolToJson(g_state.systemArmed) << ",";
        oss << "\"alarmActive\":" << boolToJson(g_state.alarmActive) << ",";
        oss << "\"doorOpen\":" << boolToJson(g_state.doorOpen) << ",";
        oss << "\"lockState\":\"" << jsonEscape(g_state.lockState) << "\",";
        oss << "\"lastAuthorizedMethod\":\"" << jsonEscape(g_state.lastAuthorizedMethod) << "\",";
        oss << "\"lastAuthorizedUser\":\"" << jsonEscape(g_state.lastAuthorizedUser) << "\",";
        oss << "\"lastAuthorizationTime\":\"" << jsonEscape(g_state.lastAuthorizationTime) << "\",";
        oss << "\"pendingVerification\":" << boolToJson(g_state.pendingVerification) << ",";
        oss << "\"intrusionDetected\":" << boolToJson(g_state.intrusionDetected) << ",";
        oss << "\"buzzerOn\":" << boolToJson(g_state.buzzerOn) << ",";
        oss << "\"ledStatus\":\"" << jsonEscape(g_state.ledStatus) << "\",";
        oss << "\"cameraOnline\":" << boolToJson(g_state.cameraOnline) << ",";
        oss << "\"nfcOnline\":" << boolToJson(g_state.nfcOnline) << ",";
        oss << "\"gpioOnline\":" << boolToJson(g_state.gpioOnline) << ",";
        oss << "\"apiOnline\":" << boolToJson(g_state.apiOnline) << ",";
        oss << "\"uptimeSeconds\":" << getUptimeSeconds();
        oss << "}";

        return oss.str();
    }

    std::string buildLogsJson(std::size_t limit) {
        std::ostringstream oss;

        oss << "{";
        oss << "\"ok\":true,";
        oss << "\"count\":" << std::min(limit, g_logs.size()) << ",";
        oss << "\"data\":[";

        for (std::size_t i = 0; i < g_logs.size() && i < limit; ++i) {
            if (i > 0) {
                oss << ",";
            }

            const auto& log = g_logs[i];

            oss << "{";
            oss << "\"id\":\"" << jsonEscape(log.id) << "\",";
            oss << "\"time\":\"" << jsonEscape(log.time) << "\",";
            oss << "\"type\":\"" << jsonEscape(log.type) << "\",";
            oss << "\"message\":\"" << jsonEscape(log.message) << "\",";
            oss << "\"extra\":\"" << jsonEscape(log.extra) << "\"";
            oss << "}";
        }

        oss << "]";
        oss << "}";

        return oss.str();
    }

    std::string getJsonStringField(const std::string& body, const std::string& key) {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPos = body.find(pattern);
        if (keyPos == std::string::npos) return "";

        std::size_t colonPos = body.find(':', keyPos + pattern.size());
        if (colonPos == std::string::npos) return "";

        std::size_t firstQuote = body.find('"', colonPos + 1);
        if (firstQuote == std::string::npos) return "";

        std::size_t secondQuote = body.find('"', firstQuote + 1);
        if (secondQuote == std::string::npos) return "";

        return body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
    }

    bool getJsonBoolField(const std::string& body, const std::string& key, bool& valueOut) {
        std::string pattern = "\"" + key + "\"";
        std::size_t keyPos = body.find(pattern);
        if (keyPos == std::string::npos) return false;

        std::size_t colonPos = body.find(':', keyPos + pattern.size());
        if (colonPos == std::string::npos) return false;

        std::size_t valuePos = body.find_first_not_of(" \t\r\n", colonPos + 1);
        if (valuePos == std::string::npos) return false;

        if (body.compare(valuePos, 4, "true") == 0) {
            valueOut = true;
            return true;
        }

        if (body.compare(valuePos, 5, "false") == 0) {
            valueOut = false;
            return true;
        }

        return false;
    }

    void setJsonResponse(httplib::Response& res, const std::string& body, int status = 200) {
        res.status = status;
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content(body, "application/json");
    }
}

int main() {
    httplib::Server server;

    server.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    });

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        setJsonResponse(
            res,
            "{"
            "\"ok\":true,"
            "\"service\":\"Door Intrusion Alarm API\","
            "\"version\":\"1.0.0\","
            "\"endpoints\":["
            "\"GET /api/health\","
            "\"GET /api/status\","
            "\"GET /api/logs\","
            "\"POST /api/system/arm\","
            "\"POST /api/system/disarm\","
            "\"POST /api/system/lock\","
            "\"POST /api/system/unlock\","
            "\"POST /api/access/authorize\","
            "\"POST /api/door/open\","
            "\"POST /api/door/close\","
            "\"POST /api/alarm/trigger\","
            "\"POST /api/alarm/clear\","
            "\"POST /api/devices/update\","
            "\"POST /api/email/test\""
            "]"
            "}"
        );
    });

    server.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::ostringstream oss;
        oss << "{";
        oss << "\"ok\":true,";
        oss << "\"service\":\"door-alarm-api-cpp\",";
        oss << "\"time\":\"" << nowIso8601() << "\",";
        oss << "\"uptimeSeconds\":" << getUptimeSeconds() << ",";
        oss << "\"devices\":{";
        oss << "\"gpioOnline\":" << boolToJson(g_state.gpioOnline) << ",";
        oss << "\"nfcOnline\":" << boolToJson(g_state.nfcOnline) << ",";
        oss << "\"cameraOnline\":" << boolToJson(g_state.cameraOnline) << ",";
        oss << "\"apiOnline\":" << boolToJson(g_state.apiOnline);
        oss << "}";
        oss << "}";

        setJsonResponse(res, oss.str());
    });

    server.Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);
        setJsonResponse(res, std::string("{\"ok\":true,\"data\":") + buildStateJson() + "}");
    });

    server.Get("/api/logs", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::size_t limit = 50;
        if (req.has_param("limit")) {
            limit = static_cast<std::size_t>(std::stoul(req.get_param_value("limit")));
            if (limit > 200) {
                limit = 200;
            }
        }

        setJsonResponse(res, buildLogsJson(limit));
    });

    server.Post("/api/system/arm", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_state.systemArmed = true;
        updateLedStatus();
        addLog("system", "System armed");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"System armed successfully\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/system/disarm", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_state.systemArmed = false;
        g_state.pendingVerification = false;
        clearAlarm();
        updateLedStatus();
        addLog("system", "System disarmed");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"System disarmed successfully\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/system/lock", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_state.lockState = "locked";
        addLog("system", "Door locked");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Door locked\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/system/unlock", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_state.lockState = "unlocked";
        addLog("system", "Door unlocked");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Door unlocked\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/access/authorize", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::string method = getJsonStringField(req.body, "method");
        std::string user = getJsonStringField(req.body, "user");

        if (method != "nfc" && method != "face" && method != "api") {
            setJsonResponse(
                res,
                "{\"ok\":false,\"message\":\"Invalid method. Use nfc, face, or api\"}",
                400
            );
            return;
        }

        if (user.empty()) {
            user = "unknown";
        }

        authorizeAccess(method, user);

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Access authorized\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/door/open", [](const httplib::Request&, httplib::Response& res) {
        {
            std::lock_guard<std::mutex> lock(g_mutex);

            g_state.doorOpen = true;
            addLog("door", "Door opened");

            if (g_state.systemArmed) {
                g_state.pendingVerification = true;
                updateLedStatus();
                addLog("system", "Verification window started", "5 seconds");
            }

            setJsonResponse(
                res,
                std::string("{\"ok\":true,\"message\":\"Door open event received\",\"data\":")
                    + buildStateJson() + "}"
            );
        }

        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(VERIFICATION_WINDOW_SECONDS));

            std::lock_guard<std::mutex> lock(g_mutex);

            if (g_state.doorOpen && g_state.systemArmed && g_state.pendingVerification) {
                triggerAlarm("Door opened without valid authorization in time window");
            }
        }).detach();
    });

    server.Post("/api/door/close", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_state.doorOpen = false;
        g_state.pendingVerification = false;
        updateLedStatus();
        addLog("door", "Door closed");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Door close event received\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/alarm/trigger", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::string reason = getJsonStringField(req.body, "reason");
        if (reason.empty()) {
            reason = "Manual alarm trigger";
        }

        triggerAlarm(reason);

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Alarm triggered\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/alarm/clear", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        clearAlarm();

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Alarm cleared\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/devices/update", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        bool value = false;
        if (getJsonBoolField(req.body, "gpioOnline", value)) {
            g_state.gpioOnline = value;
        }
        if (getJsonBoolField(req.body, "nfcOnline", value)) {
            g_state.nfcOnline = value;
        }
        if (getJsonBoolField(req.body, "cameraOnline", value)) {
            g_state.cameraOnline = value;
        }

        addLog("device", "Device status updated");

        setJsonResponse(
            res,
            std::string("{\"ok\":true,\"message\":\"Device status updated\",\"data\":")
                + buildStateJson() + "}"
        );
    });

    server.Post("/api/email/test", [](const httplib::Request&, httplib::Response& res) {
        std::thread([]() {
            bool ok = g_emailAlert.sendIntrusionAlert(
                "Test Email Alert",
                "This is a test email from Door Intrusion Alarm System."
            );

            std::lock_guard<std::mutex> lock(g_mutex);
            if (ok) {
                addLog("email", "Test email sent");
            } else {
                addLog("email", "Test email failed");
            }
        }).detach();

        setJsonResponse(res, "{\"ok\":true,\"message\":\"Test email request sent\"}");
    });

    server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        if (res.status == 404) {
            setJsonResponse(res, "{\"ok\":false,\"message\":\"Route not found\"}", 404);
        }
    });

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        addLog("system", "API server started on port 3000");
        if (g_emailAlert.isConfigured()) {
            addLog("email", "Email alert configured");
        } else {
            addLog("email", "Email alert not configured");
        }
    }

    std::cout << "Door Alarm C++ API running at http://localhost:3000" << std::endl;
    server.listen("0.0.0.0", 3000);

    return 0;
}