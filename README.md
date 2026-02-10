

# 🏥 Hospital Emergency Queue System
## Priority-Based Emergency Triage Web Application (DSA Project)

> A web-based hospital triage system that prioritizes patients based on medical urgency using Data Structures and Algorithms. Developed as part of a Data Structures academic project.

---

## 📌 Overview

This project simulates a real-world **Hospital Emergency Room Queue** where patients are treated based on their medical condition rather than simple arrival order.

Patient priority is calculated using:
- Severity level
- Vital signs (Heart Rate, Blood Pressure, SpO₂)
- Age
- Waiting time

The system provides a **web interface** connected to a **C++ backend** that manages the triage logic.

**Tech Stack**
- C++ (Backend)
- HTTP Server using `cpp-httplib`
- HTML, CSS, JavaScript (Frontend)
- CMake (Build System)

---

## ✨ Features

- ➕ Add new patient
- 🔄 Update patient medical details
- 💊 Treat highest-priority patient first
- 📋 Live queue display sorted by urgency
- 🧹 Reset queue (clears all data and restarts IDs)
- 🔒 Thread-safe backend using mutex
- 🌐 Simple web interface

---

## 🧠 Data Structures Used

| Structure | Purpose |
|-----------|---------|
| `unordered_map<int, Patient>` | Stores patient records with O(1) access |
| `vector<Patient>` | Used for sorting patients by priority |
| Custom sorting (lambda) | Orders patients by priority and arrival time |
| `mutex` | Ensures thread-safe operations |

---

## ⚙️ Priority Calculation

Priority score is calculated as:

```
Base Severity Score
+ SpO₂ condition bonus
+ Heart Rate condition bonus
+ Blood Pressure condition bonus
+ Age factor (≥ 65)
+ Waiting time factor
```

### Severity Base Score

| Severity | Priority Base |
|----------|---------------|
| 1 (Immediate) | 100 |
| 2 | 85 |
| 3 | 70 |
| 4 | 55 |
| 5 (Non-Urgent) | 40 |

Higher priority score → Treated first.

If two patients have equal priority, the one who **arrived earlier** is treated first.

---

## 🏗️ Project Structure

```
hospital-queue-web/
│
├── src/
│   └── main.cpp          # C++ backend logic
│
├── public/
│   └── index.html        # Web interface
│
├── third_party/
│   └── httplib.h         # HTTP library
│
├── build/                # Generated build files
├── CMakeLists.txt
└── README.md
```

---

## 🚀 How to Run

### 1. Clone the repository
```
git clone https://github.com/your-username/hospital-queue-web.git
cd hospital-queue-web
```

### 2. Build the project
```
mkdir build
cd build
cmake ..
cmake --build .
```

### 3. Run the server
From project root:
```
./build/server
```

You should see:
```
Triage backend running at http://localhost:8080
```

### 4. Open in browser
```
http://localhost:8080
```

---

## 🌐 API Endpoints

| Method | Endpoint | Description |
|--------|---------|-------------|
| POST | `/add` | Add new patient |
| POST | `/update` | Update patient |
| POST | `/treat` | Treat highest-priority patient |
| GET | `/list` | Get sorted queue |
| POST | `/reset` | Clear queue and reset IDs |

---

## 📊 Time Complexity

| Operation | Complexity |
|-----------|------------|
| Add Patient | O(1) |
| Update Patient | O(1) |
| Treat Patient | O(n) |
| List Queue (sorting) | O(n log n) |

---

## 🎯 Learning Outcomes

- Implementation of priority-based scheduling
- Use of C++ STL in real-world applications
- REST API development in C++
- Integration of frontend with backend
- Thread-safe programming using mutex

---

## 👨‍💻 Author

**Mohammad Amir**  
CSE (AI & ML)  
DSA Project

---

## 📜 License

This project is for academic and educational purposes.
