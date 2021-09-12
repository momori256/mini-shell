#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int end(const char *const msg) {
  perror(msg);
  exit(1);
  return 0;
}

vector<string> split(const string &str, char d) {
  vector<string> v;
  const int size = (int)str.size();
  int prev = -1;
  for (int i = 0; i < size; ++i) {
    if (str[i] != d) continue;
    v.push_back(str.substr(prev + 1, i - prev - 1));
    prev = i;
  }
  v.push_back(str.substr(prev + 1));
  return v;
}

string trim(const string &s) {
  int l = 0, r = s.size() - 1;
  while (s[l] == ' ') ++l;
  while (s[r] == ' ') --r;
  string t;
  for (int i = l; i <= r; ++i) t += s[i];
  return t;
}

bool contain(const string &s, char c) {
  return s.find_first_of(c) != string::npos;
}

vector<char *> toArrayStyle(const vector<string> &v) {
  vector<char *> tmp;
  for (const string &s : v) tmp.push_back(const_cast<char *>(s.c_str()));
  return tmp;
}

void execCommand(const string &str) {
  auto extract = [](vector<string> &v, const string &target) {
    auto it = find(v.begin(), v.end(), target);
    if (it == v.end()) return string("");

    string s = *(it + 1);
    v.erase(it, it + 2);
    return s;
  };

  auto v = split(str, ' ');
  const string in = extract(v, "<");
  const string out = extract(v, ">");

  vector<char *> tmp = toArrayStyle(v);
  tmp.push_back(nullptr);

  if (!in.empty()) { // input redirect
    close(0);
    int fd = open(in.c_str(), O_RDONLY);
    (fd == 0) || end("open input");
    dup2(fd, STDIN_FILENO);
  }

  if (!out.empty()) { // output redirect
    close(1);
    int fd = open(out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    (fd != -1) || end("open output");
    dup2(fd, STDOUT_FILENO);
  }

  (execvp(tmp[0], tmp.data()) == 0) || end(tmp[0]);
}

int main() {
  while (1) {
    cout << "@ ";
    string s;
    getline(std::cin, s);
    if (s == "quit") return 0;
    if (!contain(s, '|')) {
      if (fork() == 0) {
        execCommand(s);
      } else {
        int status = 0;
        (wait(&status) != (pid_t)-1) || end("wait");
      }
      continue;
    }

    auto v = split(s, '|');
    for (auto &x : v) x = trim(x);

    if (fork() == 0) {
      int fd[2];
      pipe(fd) == 0 || end("pipe");
      if (fork() == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execCommand(v[0]);
      } else {
        int status = 0;
        (wait(&status) != (pid_t)-1) || end("wait");
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execCommand(v[1]);
      }
    } else {
      int status = 0;
      (wait(&status) != (pid_t)-1) || end("wait");
    }
  }
  return 0;
}