// Pratiksha Bhende - Wt (C++) web app: Home, About, Projects, Contact
// Contact form -> fetch POST -> /api/contact (Wt resource) -> in-memory vector

#include <Wt/WApplication.h>
#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLink.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Value.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Step 3: simple in-memory storage
// ---------------------------------------------------------------------------
struct Submission {
  std::string name;
  std::string email;
  std::string message;
  std::string receivedAt;
};

static std::string jsonEscape(const std::string& s) {
  std::ostringstream o;
  for (unsigned char c : s) {
    switch (c) {
      case '"':  o << "\\\""; break;
      case '\\': o << "\\\\"; break;
      case '\n': o << "\\n";  break;
      case '\r': o << "\\r";  break;
      case '\t': o << "\\t";  break;
      default:
        if (c < 0x20) o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c << std::dec;
        else o << c;
    }
  }
  return o.str();
}

static std::string jsonField(const Wt::Json::Object& obj, const std::string& key) {
  auto it = obj.find(key);
  if (it == obj.end()) return "";
  const Wt::Json::Value& v = it->second;
  if (v.type() != Wt::Json::Type::String) return "";
  Wt::WString s = v;
  return s.toUTF8();
}

static std::string trim(const std::string& s) {
  const char* ws = " \t\r\n";
  auto b = s.find_first_not_of(ws);
  if (b == std::string::npos) return "";
  auto e = s.find_last_not_of(ws);
  return s.substr(b, e - b + 1);
}

// ---------------------------------------------------------------------------
// Step 2: Wt route '/api/contact'  (reads JSON body, returns JSON response)
// ---------------------------------------------------------------------------
class ContactResource : public Wt::WResource {
public:
  ~ContactResource() override { beingDeleted(); }

protected:
  void handleRequest(const Wt::Http::Request& request,
                     Wt::Http::Response& response) override {
    response.setMimeType("application/json");
    const std::string method = request.method();

    if (method == "GET") {           // handy for verification in the browser
      listSubmissions(response);
      return;
    }
    if (method != "POST") {
      response.setStatus(405);
      response.out() << "{\"success\":false,\"error\":\"Use POST\"}";
      return;
    }

    std::string body((std::istreambuf_iterator<char>(request.in())),
                     std::istreambuf_iterator<char>());

    Wt::Json::Object obj;
    try {
      Wt::Json::parse(body, obj);
    } catch (const std::exception&) {
      response.setStatus(400);
      response.out() << "{\"success\":false,\"error\":\"Invalid JSON body\"}";
      return;
    }

    Submission s;
    s.name    = trim(jsonField(obj, "name"));
    s.email   = trim(jsonField(obj, "email"));
    s.message = trim(jsonField(obj, "message"));

    if (s.name.empty() || s.message.empty() ||
        s.email.find('@') == std::string::npos ||
        s.name.size() > 100 || s.email.size() > 200 || s.message.size() > 2000) {
      response.setStatus(400);
      response.out() << "{\"success\":false,\"error\":\"Please provide a valid name, email and message\"}";
      return;
    }

    std::size_t total;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::ostringstream ts;
      ts << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
      s.receivedAt = ts.str();
      submissions_.push_back(s);
      total = submissions_.size();

      // Step 5: log on the server console
      std::cout << "[CONTACT #" << total << "] " << s.receivedAt
                << " | name=" << s.name
                << " | email=" << s.email
                << " | message=" << s.message << std::endl;
    }

    response.setStatus(201);
    response.out() << "{\"success\":true,\"message\":\"Thank you, " << jsonEscape(s.name)
                   << "! Your message has been received.\",\"total\":" << total << "}";
  }

private:
  void listSubmissions(Wt::Http::Response& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    response.out() << "{\"count\":" << submissions_.size() << ",\"submissions\":[";
    for (std::size_t i = 0; i < submissions_.size(); ++i) {
      const Submission& s = submissions_[i];
      if (i) response.out() << ",";
      response.out() << "{\"name\":\"" << jsonEscape(s.name)
                     << "\",\"email\":\"" << jsonEscape(s.email)
                     << "\",\"message\":\"" << jsonEscape(s.message)
                     << "\",\"receivedAt\":\"" << s.receivedAt << "\"}";
    }
    response.out() << "]}";
  }

  std::mutex mutex_;
  std::vector<Submission> submissions_;
};

// ---------------------------------------------------------------------------
// Pages: '/', '/about', '/projects', '/contact' with a nav bar
// ---------------------------------------------------------------------------
class SiteApp : public Wt::WApplication {
public:
  explicit SiteApp(const Wt::WEnvironment& env) : Wt::WApplication(env) {
    setTitle("Pratiksha Bhende | Wt Portfolio");
    useStyleSheet("static/style.css");
    require("static/contact.js");

    auto nav = root()->addNew<Wt::WContainerWidget>();
    nav->setStyleClass("navbar");
    nav->addNew<Wt::WText>("<span class=\"brand\">Pratiksha</span>");

    const char* labels[] = {"Home", "About", "Projects", "Contact"};
    const char* paths[]  = {"/", "/about", "/projects", "/contact"};
    for (int i = 0; i < 4; ++i)
      links_.push_back(nav->addNew<Wt::WAnchor>(
          Wt::WLink(Wt::LinkType::InternalPath, paths[i]), labels[i]));

    pages_ = root()->addNew<Wt::WStackedWidget>();
    pages_->setStyleClass("page");

    addPage("<h1>Welcome</h1><p>This is a three-page C++ web application built with "
            "<b>Wt</b>. Use the navigation bar to move between pages.</p>");
    addPage("<h1>About</h1><p>I am Pratiksha Bhende, learning full-stack and C++ web "
            "development. This site shows Wt routing and a C++ REST-style endpoint.</p>");
    addPage("<h1>Projects</h1><ul><li>Portfolio website (HTML, CSS, JS)</li>"
            "<li>React to-do list</li><li>Simple REST API with Node.js</li>"
            "<li>Wt C++ web app with contact endpoint</li></ul>");

    // Step 1: contact form (default submit is prevented in static/contact.js,
    // which sends the data with fetch + POST to /api/contact)
    auto contact = pages_->addNew<Wt::WContainerWidget>();
    contact->addNew<Wt::WText>(
        "<h1>Contact</h1>"
        "<form id=\"contact-form\" novalidate=\"novalidate\">"
        "<label for=\"cf-name\">Name</label>"
        "<input type=\"text\" id=\"cf-name\" />"
        "<label for=\"cf-email\">Email</label>"
        "<input type=\"email\" id=\"cf-email\" />"
        "<label for=\"cf-message\">Message</label>"
        "<textarea id=\"cf-message\" rows=\"5\"></textarea>"
        "<button type=\"submit\">Send</button>"
        "</form>"
        "<p id=\"form-status\" class=\"status\"></p>",
        Wt::TextFormat::UnsafeXHTML);

    internalPathChanged().connect([this](const std::string& p) { route(p); });
    route(internalPath());
  }

private:
  void addPage(const std::string& html) {
    auto c = pages_->addNew<Wt::WContainerWidget>();
    c->addNew<Wt::WText>(html);
  }

  void route(const std::string& path) {
    int idx = 0;
    if (path == "/about") idx = 1;
    else if (path == "/projects") idx = 2;
    else if (path == "/contact") idx = 3;
    pages_->setCurrentIndex(idx);
    for (int i = 0; i < (int)links_.size(); ++i)
      links_[i]->toggleStyleClass("active", i == idx);
  }

  Wt::WStackedWidget* pages_ = nullptr;
  std::vector<Wt::WAnchor*> links_;
};

int main(int argc, char** argv) {
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(
        Wt::EntryPointType::Application,
        [](const Wt::WEnvironment& env) { return std::make_unique<SiteApp>(env); },
        "/");

    server.addResource(std::make_shared<ContactResource>(), "/api/contact");

    if (server.start()) {
      std::cout << "Server running. Open http://localhost:8080/" << std::endl;
      Wt::WServer::waitForShutdown();
      server.stop();
    }
  } catch (const Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
