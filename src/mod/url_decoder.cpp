#include <bits/stdc++.h>
using namespace std;

string decode(string &s) {
  string ans;
  auto hexToAscii = [](char c) -> int {
    if (isdigit(c)) {
      return c - '0';
    } else
      return 10 + (toupper(c) - 'A');
  };
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == '%') {
      if (i + 2 >= s.size()) {
        return "Not Valid";
      }
      char val = 16 * hexToAscii(s[i + 1]) + hexToAscii(s[i + 2]);
      ans.push_back(val);
      i += 2;
    } else if (s[i] == '+') {
      ans.push_back(' ');
    } else {
      ans.push_back(s[i]);
    }
  }
  return ans;
}
pair<bool, map<string, string>> decodeURLCOMPONENT(string &s) {
#if 0

  string key = "", value = "";
  bool found = true;
  map<string, string> mpp;
  auto findKey = [&](int idx) -> int {
    for (int i = idx; i < s.size(); i++) {
      if (s[i] == '=')
        return i;
      else
        key.push_back(s[i]);
    }
    return -1;
  };
  auto findValue = [&](int idx) -> int {
    for (int i = idx; i < s.size(); i++) {
      if (s[i] == '&' || i == s.size() - 1)
        return i;
      else
        value.push_back(s[i]);
    }
    return -1;
  };
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == '?') {
      found = true;
    }
    if (found) {
      i = findKey(i + 1);
      if (i < 0) {
        return {false, mpp};
      }
      i = findValue(i + 1);
      if (i < 0) {
        return {false, mpp};
      }
      mpp[key] = value;
      key = "";
      value = "";
    }
  }
  if (!key.empty() && !value.empty())
    mpp[key] = value;
  return {true, mpp};
#endif

  string key = "", value = "";
  map<string, string> mpp;
  size_t idx = s.find('?');

  if (idx == std::string::npos) {
    return {true, {}};
  }
  auto findKey = [&](int i) -> int {
    for (int e = i; e < s.size(); e++) {
      if (s[e] == '=') {
        return e;
      } else {
        key.push_back(s[e]);
      }
    }
    return -1;
  };
  auto findValue = [&](int i) -> int {
    for (int e = i; e < s.size(); e++) {
      if (s[e] == '&' || e == s.size() - 1) {
        if (e == s.size() - 1)
          value.push_back(s[e]);
        return e;
      } else
        value.push_back(s[e]);
    }
    return -1;
  };
  size_t i = idx + 1;
  while (i < s.size()) {

    int keyEnd = findKey(i);
    if (keyEnd < 0) {
      return {false, {}};
    }
    int valueEnd = findValue(keyEnd + 1);
    if (valueEnd < 0)
      valueEnd = s.size();
    key = decode(key);
    value = decode(value);
    mpp[key] = value;
    key = "";
    value = "";

    i = valueEnd + 1;
  }
  return {true, mpp};
}
int main() {
  string s = "hello%20world%21";

  string str = decode(s);

  cout << str << endl;

  string p =
      "/path?name=John+Doe&msg=Hello%2C%20World%21&empty=&encoded=%E2%9C%93";
  pair<bool, map<string, string>> ans = decodeURLCOMPONENT(p);

  for (auto &it : ans.second) {
    cout << it.first << " " << it.second << endl;
  }
}
