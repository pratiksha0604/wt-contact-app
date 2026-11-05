# Wt C++ Web App – Home, About, Projects, Contact

Author: Pratiksha Bhende

A C++ web application built with [Wt](https://www.webtoolkit.eu/wt). The Contact
form posts JSON with `fetch` to a C++ endpoint (`/api/contact`) which stores the
submissions in an in-memory `std::vector` and logs them on the server console.

## Project structure

```
wt-contact-app/
├── CMakeLists.txt
├── README.md
├── test_post.sh            (curl test)
├── src/main.cpp            (routes, pages, /api/contact handler)
└── docroot/static/
    ├── style.css
    └── contact.js          (fetch POST + confirmation message)
```

## Routing logic

| URL | What serves it |
|-----|----------------|
| `/`, `/about`, `/projects`, `/contact` | One Wt `WApplication` (`SiteApp`). Each nav link is a `WAnchor` with an *internal path*; `internalPathChanged()` calls `route()` which switches the page in a `WStackedWidget` and highlights the active link. |
| `/api/contact` | A separate `Wt::WResource` (`ContactResource`) registered with `server.addResource(...)`. `POST` reads the JSON body, validates it, pushes it into the vector and returns JSON. `GET` lists stored submissions (for verification). |

## How the 5 steps are implemented

1. **Form + fetch** – `static/contact.js` listens for `submit`, calls `preventDefault()` and sends `fetch('/api/contact', {method:'POST', ...})`.
2. **Route `/api/contact`** – `ContactResource::handleRequest` parses the JSON body (`Wt::Json::parse`) and returns a JSON success response.
3. **In-memory storage** – `std::vector<Submission>` guarded by a `std::mutex`.
4. **Confirmation** – the JS shows the server's success message under the form.
5. **Logging** – every submission is printed on the server console as `[CONTACT #n] ...`.

## Build and run

Requires Wt 4.x, CMake 3.16+ and a C++17 compiler (Linux / WSL Ubuntu recommended).

```bash
sudo apt update
sudo apt install build-essential cmake libwt-dev libwthttp-dev

cmake -S . -B build
cmake --build build

./build/wt_app --docroot "docroot;/static" --http-address 0.0.0.0 --http-port 8080
```

Open <http://localhost:8080/>, go to **Contact**, submit the form and watch the terminal.
Verify stored data at <http://localhost:8080/api/contact> or run `./test_post.sh`.

## Push to GitHub

```bash
git init
git add .
git commit -m "Wt app with contact POST endpoint"
git branch -M main
git remote add origin https://github.com/pratiksha0604/wt-contact-app.git
git push -u origin main
```
