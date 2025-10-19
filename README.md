# Simple HTTP Server for GPS data collection

A **lightweight and custom-built HTTP server** designed to receive, parse, and store GPS location data. This server is implemented directly atop a **TCP socket**, providing a simple, foundational platform for handling basic HTTP requests.

---

## Key Features

* **Custom HTTP Implementation:** Built from the ground up on a **TCP socket** (written in C) for educational purposes and minimal overhead.
* **Concurrency:** Utilizes the **`fork()`** system call to handle each client connection in a separate process.
* **SIM7600 Integration:** Specifically engineered to **receive and parse** binary GPS data payloads sent by the **SIM7600** cellular and GPS module (or similar devices) via **HTTP POST** requests.
* **Data Persistence:** Efficiently stores the received and parsed GPS coordinates (latitude, longitude, timestamp, etc.) into a **local SQLite database** for easy retrieval and analysis.
* **Visualization:** Includes a Python script using **Plotly** to generate interactive maps of the collected GPS tracks.

---

## Technology and Architecture 💻

This project is a complete pipeline utilizing a mix of foundational and modern tools:

* **Backend Server:** **C** (POSIX Sockets) for a low-level, high-performance TCP server.
* **Concurrency:** Uses the **`fork()`** system call for parallel client handling.
* **Data Storage:** **SQLite 3** for reliable, lightweight, file-based data persistence (`tracker_board.db`).
* **Visualization:** **Python** with **Pandas** (data manipulation) and **Plotly Express** (interactive mapping).
* **Network Exposure:** **ngrok** is recommended for safely creating a public, routable endpoint to receive data from the SIM7600 device.

---

## How It Works (The Complete Pipeline) 🗺️

This project implements the full data flow from the physical device to an interactive map:

1.  The **SIM7600 module** acquires GPS data and packs it into a binary format.
2.  It sends this data via an **HTTP POST** request to the public **ngrok URL**.
3.  **ngrok** securely tunnels the request to the local server running on port **8080**.
4.  The custom **C server** receives, parses, and unpacks the binary data.
5.  The server writes the structured GPS data into the **SQLite database**.
6.  The **Python visualization script** reads all entries from the SQLite database.
7.  The script uses **Plotly Express** to plot the coordinates on an **interactive map**, displaying the route line and individual points.

---

## Getting Started (Server) 🛠️

To build and run the server, you'll need a C compiler and the SQLite development libraries.

### Prerequisites (Server):

* **GCC** or **Clang**
* **Make** utility
* **SQLite 3** Development Headers/Libraries (`sqlite3.h`)
* **ngrok** (installed and authenticated)

### Installation and Run:

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/AndreasCnaus/simple_http_server
    cd simple_http_server 
    ```

2.  **Build the Server:**
    ```bash
    make
    ```

3.  **Start the Server Locally:**
    ```bash
    ./http_server
    # Server will start listening on [http://127.0.0.1:8080](http://127.0.0.1:8080)
    ```

4.  **Create a Public Tunnel (Secure Exposure):**
    Open a **second terminal window** and use ngrok to expose your local port `8080` to the internet. **Use this public URL** when configuring the SIM7600 module's HTTP POST endpoint.
    ```bash
    ngrok http 8080
    ```

---

## Visualization Tool (`plot_data.py`) 📊

The provided Python script uses **Plotly Express** to visualize the `gps_data` table contents on an interactive OpenStreetMap.

### Key Visualization Features:

* **Interactive Map:** Displays the GPS track using `px.scatter_mapbox`.
* **Detailed Hover Info:** Custom tooltips display the exact **Latitude**, **Longitude**, **Date/Time**, **Altitude**, and **Velocity** for each recorded point.

### Prerequisites & Setup (Visualization):

* **Python 3**
* The following Python packages: `pandas`, `plotly`, and `sqlite3`.

It is **highly recommended** to use a Python **virtual environment** to install these dependencies:

1.  **Create and Activate Virtual Environment:**
    ```bash
    # Create the environment 
    python -m venv venv 
    
    # Activate the environment (Linux/macOS)
    source venv/bin/activate 
    ```

2.  **Install Required Packages:**
    ```bash
    # Install pandas and plotly, which includes plotly.express
    pip install pandas plotly
    ```

### Running the Visualization:

1.  Ensure your virtual environment is active.
2.  Run the script (assuming the SQLite file is in the `build/` directory, as per your C code):
    ```bash
    python plot_data.py
    ```
    This will open the interactive map in your default web browser.