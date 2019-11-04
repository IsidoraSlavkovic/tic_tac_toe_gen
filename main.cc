#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <unordered_set>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"

DEFINE_string(output_dir, "./",
              "The directory to output files to. Should end with a '/'.");

using namespace std;

static const string HTML_TEMPLATE = R"(
<!DOCTYPE html>
<html>
  <head>
      <title>Tic-Tac-Toe</title>
      <style>
        table {
          font-family: arial, sans-serif;
          border-collapse: collapse;
          width: 65%;
        }
        td {
          border: 3px solid black;
          text-align: center;
          padding: 100px;
        }
        a:link{
            text-decoration: none;
            color: black;
        }
      </style>
  </head>
<body>
  <table>
    <tr>
      <td>$0</td>
      <td>$1</td> 
      <td>$2</td>
    </tr>
    <tr>
      <td>$3</td>
      <td>$4</td> 
      <td>$5</td>
    </tr>
    <tr>
      <td>$6</td>
      <td>$7</td> 
      <td>$8</td>
    </tr>
  </table>
</body>
</html>
)";

static const string HTML_END_TEMPLATE = R"(
<!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
</head>
<body>
  <h1>$0 WON!</h1>
</body>
</html>
)";

// State: 010202101 means:
// _X_
// O_O
// X_X
bool is_ending_state(const string& state) {
  set<char> s;
  for (int i = 0; i < 3; i++) {
    s.clear();
    s.insert(state[i * 3 + 0]);
    s.insert(state[i * 3 + 1]);
    s.insert(state[i * 3 + 2]);
    if (s.size() == 1 && *s.begin() != '0') return true;

    s.clear();
    s.insert(state[0 * 3 + i]);
    s.insert(state[1 * 3 + i]);
    s.insert(state[2 * 3 + i]);
    if (s.size() == 1 && *s.begin() != '0') return true;
  }

  s.clear();
  s.insert(state[0]);
  s.insert(state[4]);
  s.insert(state[8]);
  if (s.size() == 1 && *s.begin() != '0') return true;

  s.clear();
  s.insert(state[2]);
  s.insert(state[4]);
  s.insert(state[6]);
  if (s.size() == 1 && *s.begin() != '0') return true;

  return false;
}

unordered_set<string> traversed;

void generate(const string& state, bool x_turn) {
  auto merge_state = absl::StrCat(state, x_turn);
  if (traversed.count(merge_state) == 1) {
    return;
  }
  traversed.insert(merge_state);

  vector<string> subst(9);

  for (int i = 0; i < state.size(); i++) {
    switch (state[i]) {
      case '0': {
        string next_state = state;
        next_state[i] = x_turn ? '1' : '2';
        string next_merge_state = absl::StrCat(next_state, !x_turn);
        string link_next_state = "";
        if (is_ending_state(next_state)) {
          link_next_state = absl::StrCat(x_turn, "_won.html");
        } else {
          generate(next_state, !x_turn);
          link_next_state = absl::StrCat(next_merge_state, "_state.html");
        }
        subst[i] = absl::Substitute("<a href=\"$0\">*</a>", link_next_state);
        break;
      }
      case '1': {
        subst[i] = 'X';
        break;
      }
      case '2': {
        subst[i] = 'O';
        break;
      }
    }
  }

  string page =
      absl::Substitute(HTML_TEMPLATE, subst[0], subst[1], subst[2], subst[3],
                       subst[4], subst[5], subst[6], subst[7], subst[8]);
  ofstream fout(absl::StrCat(FLAGS_output_dir, merge_state, "_state.html"));
  fout << page;
  fout.close();
}

void generate_end_states() {
  for (int i = 0; i <= 1; i++) {
    ofstream fout(absl::Substitute(
        absl::StrCat(FLAGS_output_dir, "$0_won.html"), i));
    fout << absl::Substitute(HTML_END_TEMPLATE, i == 0 ? 'O' : 'X');
    fout.close();
  }
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cout << absl::Substitute("Starting generation to $0\n", FLAGS_output_dir);
  auto start = absl::Now();
  generate("000000000", true);
  generate_end_states();
  auto end = absl::Now();
  cout << absl::Substitute("Finised generation in $0\n",
                           absl::FormatDuration(end - start));
  return 0;
}
