

# BlockMySeat: A MINECRAFT Themed C++ Movie Booking System

BlockMySeat is a web-based movie ticket booking application featuring a C++ backend and a vanilla JavaScript frontend. It allows users to sign up, log in, browse movies and venues, and select seats for a showtime.

## Core Technologies
*   **Backend:** C++
    *   **Web Framework:** CrowCpp
    *   **Database:** SQLite3
*   **Frontend:** HTML, CSS, Vanilla JavaScript

---

## Prerequisites

Before you begin, you will need to install a C++ compiler and the Git version control system.

### For Windows

1.  **MSYS2 with MinGW C++ Compiler (Required):** This provides the `g++` compiler and the necessary environment to build the project.
    *   Go to the [MSYS2 website](https://www.msys2.org/) and download the installer.
    *   Follow the installation instructions.
    *   After installation, open the **MSYS2 UCRT64** terminal (search for it in the Start Menu).
    *   Update the package database by running these commands:
        ```bash
        pacman -Syu
        pacman -Su
        ```
    *   Install the C++ toolchain (which includes `g++` and other build tools):
        ```bash
        pacman -S mingw-w64-ucrt-x86_64-toolchain
        ```

2.  **Git for Windows:**
    *   Download and install [Git for Windows](https://git-scm.com/download/win). This will allow you to clone the repository.

### For macOS

1.  **Xcode Command Line Tools (Required):** This provides the `g++` compiler (Clang) and Git.
    *   Open the Terminal application.
    *   Run the following command. A pop-up will appear asking you to install the tools.
        ```bash
        xcode-select --install
        ```

2.  **Homebrew:** This is the standard package manager for macOS, used here to install the SQLite library.
    *   Open the Terminal and install Homebrew by running the command found on the [Homebrew website](https://brew.sh/).
    *   Once Homebrew is installed, use it to install the SQLite3 development library:
        ```bash
        brew install sqlite3
        ```

---

## Setup and Installation Guide

Follow these steps carefully to get the project running on your local machine.

### Step 1: Clone the Repository

Open your terminal (Git Bash on Windows, or the standard Terminal on macOS) and navigate to the directory where you want to store the project. Then, run the following command:

```bash
git clone https://github.com/kingskod/BlockMySeat.git
cd BlockMySeat
```
### Step 1.5: Delete the .db file in the backend folder  

The repository by default contains a database file that is uploaded as it is a hassle to keep deleting it every push so for a fresh database, delete the backend file that comes with the repository

### Step 2: Compile the Backend

The backend must be compiled before it can be run. The necessary libraries (`Crow`, `nlohmann/json`) and the SQLite source code are already included in the repository.

#### On Windows:

**Important:** You **must** use the **MSYS2 UCRT64** terminal for this step.

1.  Navigate to the backend directory:
    ```bash
    cd backend
    ```
2.  Compile the C code for SQLite into an object file:
    ```bash
    gcc -c sqlite3.c -o sqlite3.o
    ```
3.  Compile the C++ application and link it with all necessary libraries:
    ```bash
    g++ main.cpp sqlite3.o -o server -I include -lws2_32 -lmswsock
    ```

#### On macOS:

1.  Navigate to the backend directory in your terminal:
    ```bash
    cd backend
    ```
2.  Compile the C code for SQLite:
    ```bash
    gcc -c sqlite3.c -o sqlite3.o
    ```
3.  Compile the C++ application and link it with the SQLite library:
    ```bash
    g++ main.cpp sqlite3.o -o server -I include -lsqlite3
    ```

After these commands finish, you will have a new executable file named `server` (or `server.exe`) in your `backend` folder.

### Step 3: Run the Application

The project consists of two separate parts that must be running at the same time: the backend server and the frontend client.

1.  **Start the Backend Server:**
    *   In your terminal, while still in the `backend` directory, run the executable:
        ```bash
        ./server
        ```
    *   You should see the message `Server starting on port 18080...`.
    *   **Keep this terminal window open.** The server must be running for the website to work.

2.  **Open the Frontend:**
    *   Navigate to the `frontend` folder in your file explorer.
    *   Open the `index.html` file in your preferred web browser (e.g., Chrome, Firefox).

You can now use the website!

---

## How to Use

1.  The `index.html` page is the main login/sign-up page.
2.  You can create an account or use the "Movies" / "Venues" links in the header to enter as a guest.
3.  After logging in, you will be redirected to the `movies.html` page.
4.  From there, you can navigate the site, select a movie, choose a showtime, and proceed to the seat selection page.

## Troubleshooting

*   **`g++: command not found` (Windows):** You are not using the **MSYS2 UCRT64** terminal. Close your current terminal and open the correct one from the Start Menu.
*   **Movies/Venues don't load (CORS or Connection Error):**
    1.  Make sure your C++ `./server` is running in the terminal.
    2.  **Firewall (Windows):** The first time you run the server, Windows Defender Firewall may ask for permission. Ensure you check **both Private and Public networks** and click **"Allow access"**.
*   **Data seems old or tables are missing:** If you make changes to the database structure in `main.cpp`, you **must delete the `blockmyseat.db` file** in the `backend` folder and restart the server to force it to create a new, correct database.