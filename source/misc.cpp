#include "misc.hpp"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

bool misc::resolve_tilde::would_resolve_tilde(const std::string_view str) {
  return (!(str.empty() || str.front() != '~'));
}

std::string misc::resolve_tilde::resolve_tilde(const std::string &str) {
  if (!would_resolve_tilde(str)) {
    return str;
  } else {
    std::string user{};
    std::string::size_type i{1};
    for (; i < str.size(); ++i) {
      if (str[i] == '/') {
        break;
      } else {
        user += str[i];
      }
    }

    if (user.empty()) {
      const char *env_home{std::getenv("HOME")};
      return ((env_home == nullptr) ? ("")
                                    : (std::string{env_home + str.substr(i)}));
    } else {
      struct passwd pwd;
      struct passwd *result;
      char *buf;
      long bufsize;

      bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
      if (bufsize == -1) {
        bufsize = 16384;
      }

      buf = static_cast<char *>(std::malloc(bufsize));
      if (buf == NULL) {
        return "";
      }

      getpwnam_r(user.c_str(), &pwd, buf, bufsize, &result);
      if (result == NULL) {
        return "";
      }

      return std::string{pwd.pw_dir + str.substr(i)};
    }
  }
}
