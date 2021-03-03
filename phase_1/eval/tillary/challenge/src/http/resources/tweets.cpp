#include "tweets.hpp"

#include "../../logger.hpp"
#include "../html_response.hpp"
#include "../redirect_response.hpp"
#include "../../rng.hpp"

#include <algorithm>
#include <fstream>

using namespace http;
using namespace http::resources;

const std::filesystem::path tweets_base = "/tweets/";
const std::filesystem::path tweets_root_dir = "/data/tweets/";

bool Tweets::handles(const std::filesystem::path& path) {
#ifdef PATCH_CHECK_FOR_DOTDOT
  if (path.lexically_relative(tweets_base).string().find("../") !=
      std::string::npos) {
    return false;
  }
#else
  if (0 != path.string().find(tweets_base.string())) {
    return false;
  }
#endif

  return true;
}

Tweets::Tweets(const Request& rq) : Resource(rq) {
  std::filesystem::path p = rq.path();
  if ("" != p.stem()) {
#ifdef PATCH_CHECK_FOR_DOTDOT
    status_id = p.stem().string();
#else
    status_id = p.string().substr(tweets_base.string().size());
#endif
    LLL.info() << status_id;
  } else {
    wanted_index = true;
  }
}

bool Tweets::allowed_method() {
  if (req.method_name() == Method::Name::GET) return true;
  if (req.method_name() == Method::Name::HEAD) return true;


  if (req.method_name() == Method::Name::POST) return true;

  if (req.method_name() == Method::Name::TRACE) return true;

  return false;
}


bool Tweets::exists() {
  if (is_index()) return true;
  try_load();

  if (did_load_status) return true;

  return false;
}

bool Tweets::acceptable_content_type() {
  auto accept_header = req.headers.find("Accept");
  if (req.headers.cend() == accept_header) return true;
  std::string types = accept_header->second;
  if (std::string::npos != types.find("*/*")) return true;
  if (std::string::npos != types.find("text/html")) return true;

  return false;
}

bool Tweets::allow_post() {
  if (is_index()) return true;

  return false;
}

void Tweets::process() {
  if (! is_index()) {
    throw http::RuntimeError("trying to process tweet not index");
  }

  if (Method::Name::POST != req.method_name()) {
    throw http::RuntimeError("trying to process tweet non-POST");
  }

  if (nullptr == req.body) {
    throw UnsupportedMediaType("Unsupported Media Type: need content");
  }

  if (req.body->size() == 0) {
    throw UnsupportedMediaType("Unsupported Media Type: need content");
  }

  if (req.body->size() >= 255) {
    throw PayloadTooLarge();
  }

  uint8_t len = req.body->size();

  std::time_t timestamp = std::time(nullptr);
  static_assert(std::is_same<std::time_t,
                std::int64_t>::value,
                "really needed std::time to be an int64_t");

  std::filesystem::path dest_file = tweets_root_dir /
    (std::to_string(timestamp) +
     "-" +
     Rng::get_printables(8));

  LLL.info() << "writing tweet `"
             << *(req.body)
             << "` to `"
             << dest_file
             << "`";

  std::ofstream writer(dest_file);
  if (!writer.is_open()) {
    throw http::RuntimeError("failed to save tweet");
  }
  if (!writer.good()) {
    throw http::RuntimeError("failed to save tweet");
  }
  writer.write(req.body->data(), len);
  if (!writer.good()) {
    throw http::RuntimeError("failed to save tweet");
  }
  writer.close();

  status_id = dest_file.stem().string();

  LLL.info() << "tweeted " << status_id;
}

bool Tweets::did_create() {
  if (Method::Name::POST != req.method_name()) return false;
  if (! is_index()) return false;
  if ("" == status_id) return false;

  return true;
}

bool Tweets::should_redirect() {
  if (! did_create()) return false;
  return true;
}

std::string Tweets::redirect_location() {
  return tweets_base / status_id;
}


ResponsePtr Tweets::get_response() {
  if (is_index()) return get_index();
  return get_status();
}

bool Tweets::is_loadable_status_id() const {
  if ("" != status_id) return true;

  return false;
}

bool Tweets::is_index() const {
  return wanted_index;
}

void Tweets::try_load() {
  if (did_load_status) return;
  if (failed_to_load_status) return;
  if (! is_loadable_status_id()) return;

#ifdef PATCH_CHECK_FOR_DOTDOT
  std::filesystem::path load_from = tweets_root_dir / status_id;
#else
  std::filesystem::path load_from =
    (tweets_root_dir / status_id).lexically_normal();
#endif

  LLL.info() << load_from;

  std::ifstream loader = {load_from};

  if (! loader.is_open()) {
    failed_to_load_status = true;
    return;
  }

  if (! loader.good()) {
    failed_to_load_status = true;
    return;
  }

  size_t starting_at = loader.tellg();
  loader.seekg(0, std::ios_base::end);

  if (! loader.good()) {
    failed_to_load_status = true;
    return;
  }

  size_t ending_at = loader.tellg();
  loader.seekg(0, std::ios_base::beg);

  if (! loader.good()) {
    failed_to_load_status = true;
    return;
  }

  text.resize(ending_at);
  loader.read(text.data(), ending_at);
  if (! loader.good()) {
    failed_to_load_status = true;
    return;
  }

  loader.seekg(starting_at, std::ios_base::beg);

  if (! loader.good()) {
    failed_to_load_status = true;
    return;
  }

  did_load_status = true;
}

struct Tweet {
  Tweet(std::filesystem::path p) : path(p) {
    std::ifstream loader = {p};
    if (! loader.is_open()) {
      throw http::RuntimeError("couldn't load tweet");
    }
    if (! loader.good()) {
      throw http::RuntimeError("couldn't load tweet");
    }

    loader.seekg(0, std::ios_base::end);

    if (! loader.good()) {
      throw http::RuntimeError("couldn't load tweet");
    }

    size_t ending_at = loader.tellg();
    loader.seekg(0, std::ios_base::beg);

    if (! loader.good()) {
      throw http::RuntimeError("couldn't load tweet");
    }

    body.resize(ending_at);
    loader.read(body.data(), ending_at);
    if (! loader.good()) {
      throw http::RuntimeError("couldn't load tweet");
    }
  }

  std::filesystem::path path;
  std::string body;
};

ResponsePtr Tweets::get_index() {
  std::vector<std::filesystem::path> tweet_paths;

  for (auto& entry : std::filesystem::directory_iterator(tweets_root_dir)) {
    if (! entry.is_regular_file()) continue;
    if (entry.path().extension().string() != "") continue;

    tweet_paths.push_back(entry.path());
  }

  std::sort(tweet_paths.begin(), tweet_paths.end());

  std::vector<Tweet> found_tweets;
  found_tweets.reserve(tweet_paths.size());

  for (std::filesystem::path p : tweet_paths) {
    found_tweets.push_back({p});
  }

  std::shared_ptr<HtmlResponse> resp =
    std::make_shared<HtmlResponse>();

  resp->raw("<!doctype html>");
  resp->open_tag("html");
  resp->open_tag("head");
  resp->open_tag("title");
  resp->text("tweets");
  resp->close_tag();
  resp->close_tag();
  resp->open_tag("body");
  resp->raw("\n");
  resp->open_tag("h1");
  resp->text("tweets");
  resp->close_tag();
  resp->raw("\n");
  resp->open_tag("ul");

  for (Tweet t : found_tweets) {
    resp->raw("\t");
    resp->open_tag("li");
    resp->open_tag("a", std::string("href=\"/tweets/") +
                   std::string(t.path.stem()) +
                   std::string("\""));
    resp->text(t.body);
    resp->close_tag();
    resp->close_tag();
    resp->raw("\n");
  }

  resp->close_tag();
  resp->close_tag();
  resp->close_tag();

  return resp;
}

ResponsePtr Tweets::get_status() {
  if (!did_load_status) {
    throw NotFound("tried to get_status but it wasn't loaded");
  }

  std::shared_ptr<HtmlResponse> resp =
    std::make_shared<HtmlResponse>();

  resp->raw("<!doctype html>");
  resp->open_tag("html");
  resp->open_tag("head");
  resp->open_tag("title");
  resp->text("tweet");
  resp->close_tag();
  resp->close_tag();
  resp->open_tag("body");
  resp->raw("\n");
  resp->open_tag("h1");
  resp->text("tweet");
  resp->close_tag();
  resp->raw("\n");
  resp->open_tag("p");
  resp->text(text);
  resp->close_tag();
  resp->close_tag();
  resp->close_tag();

  return resp;
}
