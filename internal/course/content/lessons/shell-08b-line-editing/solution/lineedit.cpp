#include "lineedit.hpp"

#include "keys.hpp"

static bool is_space(char c) {
    return c == ' ' || c == '\t';
}

LineEditor::Result LineEditor::feed(int key) {
    switch (key) {
    case kEnter:
    case '\n':
        return Done;
    case ctrl_key('c'):
        return Cancelled;
    case ctrl_key('d'):
        if (text_.empty()) {
            return EndOfInput;
        }
        [[fallthrough]];
    case kDelete:
        if (cursor_ < text_.size()) {
            text_.erase(cursor_, 1);
        }
        return Editing;
    case kBackspace:
    case ctrl_key('h'):
        if (cursor_ > 0) {
            text_.erase(--cursor_, 1);
        }
        return Editing;
    case kArrowLeft:
        if (cursor_ > 0) {
            --cursor_;
        }
        return Editing;
    case kArrowRight:
        if (cursor_ < text_.size()) {
            ++cursor_;
        }
        return Editing;
    case kHome:
    case ctrl_key('a'):
        cursor_ = 0;
        return Editing;
    case kEnd:
    case ctrl_key('e'):
        cursor_ = text_.size();
        return Editing;
    case ctrl_key('u'):
        text_.erase(0, cursor_);
        cursor_ = 0;
        return Editing;
    case ctrl_key('k'):
        text_.erase(cursor_);
        return Editing;
    case ctrl_key('w'): {
        size_t start = cursor_;
        while (start > 0 && is_space(text_[start - 1])) {
            --start;
        }
        while (start > 0 && !is_space(text_[start - 1])) {
            --start;
        }
        text_.erase(start, cursor_ - start);
        cursor_ = start;
        return Editing;
    }
    case kArrowUp:
        if (history_->empty() || browsing_ == 0) {
            return Editing;
        }
        if (browsing_ == SIZE_MAX) {
            draft_ = text_;
            browsing_ = history_->size();
        }
        text_ = (*history_)[--browsing_];
        cursor_ = text_.size();
        return Editing;
    case kArrowDown:
        if (browsing_ == SIZE_MAX) {
            return Editing;
        }
        if (++browsing_ == history_->size()) {
            browsing_ = SIZE_MAX;
            text_ = draft_;
        } else {
            text_ = (*history_)[browsing_];
        }
        cursor_ = text_.size();
        return Editing;
    default:
        if (key >= 32 && key < 256 && key != 127) {
            text_.insert(cursor_++, 1, static_cast<char>(key));
        }
        return Editing;
    }
}

std::string LineEditor::render(const std::string& prompt) const {
    std::string out = "\r" + prompt + text_ + "\x1b[K";
    if (cursor_ < text_.size()) {
        out += "\x1b[" + std::to_string(text_.size() - cursor_) + "D";
    }
    return out;
}
