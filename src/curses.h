#pragma once

#include "client.h"
#include "entry.h"
#include "log.h"

#include <curses.h>

#include <map>
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

enum class DirectionKey : char {
  LEFT = 'h',
  DOWN = 'j',
  UP = 'k',
  RIGHT = 'l',
};

class Window {
 protected:
  int x_, y_;
  int width_, height_;

 private:
  WINDOW* win_;

  bool has_border = false;

 public:
  Window(int x, int y, int width, int height) : x_(x), y_(y), width_(width), height_(height),
      win_(newwin(height, width, y, x)) {
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

  void clear() { wclear(win_); }

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

  virtual void sync_display() = 0;
  virtual bool update(Client& client, Log& logfile) = 0;

  // TODO(jsvana): handle resizes
};

class InputWindow : public Window {
 private:
  std::string buf_;

 public:
  InputWindow(int x, int y, int width, int height) : Window(x, y, width, height) {
    add_border();
    write_string(1, 1, "> ");
    disable_delay();
  }

  void sync_display() {
    move_cursor(3 + buf_.length(), 1);
  }

  bool update(Client& client, Log& logfile) {
    char c = get_char();
    if (c < 0) {
      return true;
    } else if (c == 27) { // Escape
      return false;
    } else if (c == 13) { // Enter
      if (buf_.empty()) {
        return true;
      }
      const auto line = client.next_id() + " " + buf_;
      client.write(line);
      logfile.info("> " + line);
      buf_.clear();
      clear_line(1);
      write_string(1, 1, "> ");
    } else if (c == 8 || c == 127) { // Backspace
      buf_ = buf_.substr(0, buf_.length() - 1);
      delete_char(3 + buf_.length(), 1);
    } else {
      buf_ += c;
      // "|> "
      write_char(2 + buf_.length(), 1, c);
    }

    return true;
  }
};

class ListWindow : public Window {
 private:
  std::map<int, Entry> entries_;

  int entry_id = 0;
  int selected = entry_id;
  int offset = 0;

  int next_id() {
    return entry_id++;
  }

 public:
  ListWindow(int x, int y, int width, int height) : Window(x, y, width, height) {}

  int add_entry(const Entry& entry) {
    const auto next = next_id();
    entries_.emplace(next, entry);
    entries_.find(selected)->second.selected = true;
    return next;
  }

  void sync_display() {
    clear();
    for (auto& p : entries_) {
      if (p.first < offset || p.first >= offset + height_) {
        continue;
      }
      add_line(p.second.str());
    }
  }

  bool update(Client&, Log& logfile) {
    char c = get_char();
    int prev_selected = selected;
    switch (static_cast<DirectionKey>(c)) {
    case DirectionKey::UP:
      --selected;
      if (selected < 0) {
        selected = entries_.size() - 1;
      }
      if (selected < offset) {
        --offset;
      }
      if (selected == static_cast<int>(entries_.size()) - 1) {
        offset = entries_.size() - height_;
      }
      break;
    case DirectionKey::DOWN:
      selected = (selected + 1) % entries_.size();
      if (selected >= offset + height_) {
        ++offset;
      }
      if (selected == 0) {
        offset = 0;
      }
      break;
    default:
      break;
    }
    if (prev_selected != selected) {
      entries_.find(prev_selected)->second.selected = false;
      entries_.find(selected)->second.selected = true;
    }
    return true;
  }
};

} // namespace curses
