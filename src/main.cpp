#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <httplib.h>
using namespace std;


static inline long long now_epoch() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}


static string json_escape(const string& s) {
    string o;
    for (char c : s) {
        switch (c) {
            case '\"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default: o += c; break;
        }
    }
    return o;
}

static string json_get(const string& body, const string& key) {
    string pat = "\"" + key + "\"";
    size_t p = body.find(pat);
    if (p == string::npos) return "";
    p = body.find(':', p);
    if (p == string::npos) return "";
    size_t s = body.find_first_not_of(" \t\r\n", p + 1);
    if (s == string::npos) return "";
    if (body[s] == '"') {
        size_t e = body.find('"', s + 1);
        if (e == string::npos) return "";
        return body.substr(s + 1, e - s - 1);
    } else {
        size_t e = body.find_first_of(",}", s);
        if (e == string::npos) e = body.size();
        return body.substr(s, e - s);
    }
}


struct Patient {
    int id{};
    string name;
    int age{};
    int severity{};
    int hr{};
    int sbp{};
    int spo2{};
    long long arrival{};
};

static double calculate_priority(const Patient& p, long long now_sec) {
    double base = 100.0 - (p.severity - 1) * 15.0; // severity: 1→100, 2→85, etc.
    if (p.spo2 < 90) base += 20;
    else if (p.spo2 <= 94) base += 10;
    if (p.hr >= 130) base += 10;
    if (p.sbp < 90) base += 15;
    if (p.age >= 65) base += 5;
    long long wait_sec = max(0LL, now_sec - p.arrival);
    double minutes = wait_sec / 60.0;
    base += min(30.0, minutes * 0.5);
    return base;
}

class TriageQueue {
    mutable std::mutex mtx;
    unordered_map<int, Patient> patients;
    int next_id = 1;

public:
    int add_patient(string name, int age, int severity, int hr, int sbp, int spo2) {
        lock_guard<mutex> lock(mtx);
        Patient p{next_id++, name, age, severity, hr, sbp, spo2, now_epoch()};
        patients[p.id] = p;
        return p.id;
    }

    bool update_patient(int id, int age, int severity, int hr, int sbp, int spo2) {
        lock_guard<mutex> lock(mtx);
        auto it = patients.find(id);
        if (it == patients.end()) return false;
        it->second = {id, it->second.name, age, severity, hr, sbp, spo2, it->second.arrival};
        return true;
    }

    bool treat_patient(Patient& out) {
        lock_guard<mutex> lock(mtx);
        if (patients.empty()) return false;
        long long now = now_epoch();
        int best_id = -1;
        double best_pr = -1e18;
        long long best_arrival = LLONG_MAX;
        for (auto& kv : patients) {
            double pr = calculate_priority(kv.second, now);
            if (pr > best_pr || (fabs(pr - best_pr) < 1e-9 && kv.second.arrival < best_arrival)) {
                best_pr = pr;
                best_arrival = kv.second.arrival;
                best_id = kv.first;
            }
        }
        out = patients[best_id];
        patients.erase(best_id);
        return true;
    }

    vector<Patient> list_queue() const {
        lock_guard<mutex> lock(mtx);
        vector<Patient> v;
        for (auto& kv : patients) v.push_back(kv.second);
        long long now = now_epoch();
        sort(v.begin(), v.end(), [now](const Patient& a, const Patient& b) {
            double pa = calculate_priority(a, now), pb = calculate_priority(b, now);
            if (pa != pb) return pa > pb;
            return a.arrival < b.arrival;
        });
        return v;
    }

    void clear_all() {
        lock_guard<mutex> lock(mtx);
        patients.clear();
        next_id = 1;
    }
};

static TriageQueue triage;

static string patient_to_json(const Patient& p) {
    long long now = now_epoch();
    double pr = calculate_priority(p, now);
    ostringstream os;
    os << "{"
       << "\"id\":" << p.id
       << ",\"name\":\"" << json_escape(p.name) << "\""
       << ",\"age\":" << p.age
       << ",\"severity\":" << p.severity
       << ",\"hr\":" << p.hr
       << ",\"sbp\":" << p.sbp
       << ",\"spo2\":" << p.spo2
       << ",\"priority\":" << pr
       << "}";
    return os.str();
}

static string patients_to_json_array(const vector<Patient>& v) {
    ostringstream os;
    os << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) os << ",";
        os << patient_to_json(v[i]);
    }
    os << "]";
    return os.str();
}

int main() {
    httplib::Server svr;

    svr.set_mount_point("/", "./public");

    // ADD patient
    svr.Post("/add", [](const httplib::Request& req, httplib::Response& res) {
        try {
            string name = json_get(req.body, "name");
            int age = stoi(json_get(req.body, "age"));
            int sev = stoi(json_get(req.body, "severity"));
            int hr = stoi(json_get(req.body, "hr"));
            int sbp = stoi(json_get(req.body, "sbp"));
            int spo2 = stoi(json_get(req.body, "spo2"));
            int id = triage.add_patient(name, age, sev, hr, sbp, spo2);
            res.set_content("{\"ok\":true,\"id\":" + to_string(id) + "}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"ok\":false}", "application/json");
        }
    });

    // UPDATE patient
    svr.Post("/update", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = stoi(json_get(req.body, "id"));
            int age = stoi(json_get(req.body, "age"));
            int sev = stoi(json_get(req.body, "severity"));
            int hr = stoi(json_get(req.body, "hr"));
            int sbp = stoi(json_get(req.body, "sbp"));
            int spo2 = stoi(json_get(req.body, "spo2"));
            bool ok = triage.update_patient(id, age, sev, hr, sbp, spo2);
            res.set_content(ok ? "{\"ok\":true}" : "{\"ok\":false}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"ok\":false}", "application/json");
        }
    });

    // TREAT patient
    svr.Post("/treat", [](const httplib::Request&, httplib::Response& res) {
        Patient p;
        if (triage.treat_patient(p))
            res.set_content("{\"ok\":true,\"treated\":" + patient_to_json(p) + "}", "application/json");
        else
            res.set_content("{\"ok\":true,\"treated\":null}", "application/json");
    });

    // LIST patients
    svr.Get("/list", [](const httplib::Request&, httplib::Response& res) {
        auto v = triage.list_queue();
        ostringstream os;
        os << "{\"ok\":true,\"count\":" << v.size() << ",\"items\":" << patients_to_json_array(v) << "}";
        res.set_content(os.str(), "application/json");
    });

    // ✅ RESET queue
    svr.Post("/reset", [](const httplib::Request&, httplib::Response& res) {
        triage.clear_all();
        res.set_content("{\"ok\":true,\"msg\":\"Queue cleared and IDs reset\"}", "application/json");
    });

    cout << "Triage backend running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}