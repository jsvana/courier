#pragma once

#include <curses.h>

#include <string>

namespace curses {

void init() {
  initscr();

  if (has_colors()) {
    start_color();

    // TODO(jsvana): init color pairs
  }

  // Disable echoing characters
  noecho();

  // Disable newline on enter
  nonl();

  halfdelay(1);

  // Enable special character input
  keypad(stdscr, TRUE);

  refresh();
}

void cleanup() {
  endwin();
}

const std::tuple<int, int> dimensions() {
  int x, y;
  getmaxyx(stdscr, y, x);
  return std::make_tuple(x, y);
}

char get_char() {
  return getch();
}

class Window {
 private:
  WINDOW* win_;

  bool has_border = false;

 public:
  Window(int x, int y, int width, int height) : win_(newwin(height, width, y, x)) {
    nodelay(win_, TRUE);
  }

  ~Window() { delwin(win_); }

  void refresh() { wrefresh(win_); }

  void enable_scroll() { scrollok(win_, TRUE); }
  void disable_scroll() { scrollok(win_, FALSE); }

  void disable_delay() { nodelay(win_, TRUE); }
  void enable_delay() { nodelay(win_, FALSE); }

  void add_border() {
    wborder(win_, 0, 0, 0, 0, 0, 0, 0, 0);
    has_border = true;
  }

  void move_cursor(int x, int y) { wmove(win_, y, x); }

  void delete_char(int x, int y) {
    mvwdelch(win_, y, x);
  }

  char get_char() { return wgetch(win_); }

  void clear_line(int y) {
    move_cursor(0, y);
    wclrtoeol(win_);
    if (has_border) {
      add_border();
    }
  }

  void write_char(int x, int y, char c) { mvwaddch(win_, y, x, c); }

  void write_string(int x, int y, const std::string& str) {
    for (std::size_t i = 0; i < str.length(); i++) {
      write_char(x + i, y, str[i]);
    }
    refresh();
  }

  void add_line(const std::string& str) {
    wprintw(win_, str.c_str());
    wprintw(win_, "\n");
    refresh();
  }

  // TODO(jsvana): handle resizes
};

} // namespace curses
