#include "./filtered_string_view.h"

// Static Data Members
fsv::filter fsv::filtered_string_view::default_predicate = [](const char&) { return true; };

// Default Constructor
fsv::filtered_string_view::filtered_string_view() noexcept
: data_{nullptr}
, size_{0}
, predicate_{default_predicate} {};

// Implicit String Constructor
fsv::filtered_string_view::filtered_string_view(const std::string& str) noexcept
: data_{str.data()}
, size_{str.size()}
, predicate_{default_predicate} {};

// String Constructor with Predicate
fsv::filtered_string_view::filtered_string_view(const std::string& str, filter predicate) noexcept
: data_{str.data()}
, size_{str.size()}
, predicate_{std::move(predicate)} {};

// Implicit Null-Terminated String Constructor
fsv::filtered_string_view::filtered_string_view(const char* str) noexcept
: data_{str}
, size_{std::strlen(str)}
, predicate_{default_predicate} {};

// Null-Terminated String with Predicate Constructor
fsv::filtered_string_view::filtered_string_view(const char* str, filter predicate) noexcept
: data_{str}
, size_{std::strlen(str)}
, predicate_{predicate} {};

// Copy Constructor
fsv::filtered_string_view::filtered_string_view(const filtered_string_view& other) noexcept
: data_{other.data_}
, size_{other.size_}
, predicate_{other.predicate_} {};

// Move Constructor
fsv::filtered_string_view::filtered_string_view(filtered_string_view&& other) noexcept
: data_{std::exchange(other.data_, nullptr)}
, size_{std::exchange(other.size_, 0)}
, predicate_{std::exchange(other.predicate_, default_predicate)} {};

// Member Operator - Copy Assignment
auto fsv::filtered_string_view::operator=(const filtered_string_view& other) noexcept -> filtered_string_view& {
	if (this != &other) {
		this->data_ = other.data_;
		this->size_ = other.size_;
		this->predicate_ = other.predicate_;
	}
	return *this;
}

// Member Operator - Move Assignment
auto fsv::filtered_string_view::operator=(filtered_string_view&& other) noexcept -> filtered_string_view& {
	if (this != &other) {
		this->data_ = std::exchange(other.data_, nullptr);
		this->size_ = std::exchange(other.size_, 0);
		this->predicate_ = std::exchange(other.predicate_, default_predicate);
	}
	return *this;
}

// Member Operator - Subscript
auto fsv::filtered_string_view::operator[](int n) const noexcept -> const char& {
	auto index = 0;
	for (auto i = size_t{0}; i < size_; i++) {
		if (predicate_(data_[i])) {
			if (index == n) {
				return data_[i];
			}
			++index;
		}
	}
	return data_[size_];
}

// Member Operator - String Type Conversion
fsv::filtered_string_view::operator std::string() const noexcept {
	auto filtered_string = std::string{};
	for (auto i = size_t{0}; i < size_; ++i) {
		if (predicate_(data_[i])) {
			filtered_string.push_back(data_[i]);
		}
	}
	return filtered_string;
}

// Member Function - at
auto fsv::filtered_string_view::at(int index) const -> const char& {
	auto position = index;
	for (auto i = size_t{0}; i < size_; ++i) {
		if (predicate_(data_[i])) {
			if (position == 0) {
				return data_[i];
			}
			--position;
		}
	}
	throw std::domain_error{"filtered_string_view::at(" + std::to_string(index) + "): invalid index"};
}

// Member Function - size
auto fsv::filtered_string_view::size() const noexcept -> std::size_t {
	auto size_count = size_t{0};
	for (auto i = size_t{0}; i < size_; ++i) {
		if (predicate_(data_[i])) {
			++size_count;
		}
	}
	return size_count;
}

// Member Function - empty
auto fsv::filtered_string_view::empty() const noexcept -> bool {
	auto size_count = size_t{0};
	for (auto i = size_t{0}; i < size_; ++i) {
		if (predicate_(data_[i])) {
			++size_count;
		}
	}
	if (size_count == 0) {
		return true;
	}
	return false;
}

// Member Function - data
auto fsv::filtered_string_view::data() const noexcept -> const char* {
	return data_;
}

// Member Function - predicate
auto fsv::filtered_string_view::predicate() const noexcept -> const filter& {
	return predicate_;
}

// Non-Member Operator - Equality Comparison
auto fsv::operator==(const filtered_string_view& lhs, const filtered_string_view& rhs) noexcept -> bool {
	return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

// Non-Member Operator - Relational Comparison
auto fsv::operator<=>(const filtered_string_view& lhs, const filtered_string_view& rhs) noexcept -> std::strong_ordering {
	return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

// Non-Member Operator - Output Stream
auto fsv::operator<<(std::ostream& os, const filtered_string_view& fsv) noexcept -> std::ostream& {
	for (auto char_ = fsv.cbegin(); char_ != fsv.cend(); ++char_) {
		os << *char_;
	}
	return os;
}

// Non-Member Utility Function - Compose
auto fsv::compose(const filtered_string_view& fsv, const std::vector<filter>& filts) noexcept -> filtered_string_view {
	if (filts.empty()) {
		return filtered_string_view(fsv.data(), fsv.default_predicate);
	}

	auto new_predicate = [filts](const char& c) {
		for (const auto& filt : filts) {
			if (not(filt(c))) {
				return false;
			}
		}
		return true;
	};
	return filtered_string_view{fsv.data(), new_predicate};
};

namespace {
	/**
	 * The lambda function works as follows:
	 *
	 * 1.It keeps track of a "current_index" which keeps track of the current index
	 *   of the character being evaluated.
	 *
	 * 2.The function checks if the character satisfies the given filter
	 *   and if the current index is within the specified range [start_pos, end_pos].
	 *   If both conditions are true, the function returns true.
	 *
	 * 3.After evaluating the character, the function increments the
	 *   "current_index" and checks if it has reached the end of the string. If
	 *   it has, the "current_index" is reset to 0.
	 *
	 * 4.The function is mutable, which means that it can modify its captured variables.
	 *
	 * @param filt The filter function to be applied to each character.
	 * @param size_fsv_data The size of the underlying string data.
	 * @param start_index The starting position of the substring.
	 * @param end_index The ending position of the substring.
	 * @return A predicate to filter the substring.
	 */
	auto filter_split(const fsv::filter filt,
	                  const std::size_t size_fsv_data,
	                  const std::size_t start_index,
	                  const std::size_t end_index) noexcept -> fsv::filter {
		auto current_index = std::size_t{0};
		return [filt, size_fsv_data, start_index, end_index, current_index](const char& c) mutable {
			auto result = false;
			if (end_index != std::string::npos) {
				if (current_index >= start_index and current_index <= end_index) {
					if (filt(c)) {
						result = true;
					}
				}
			}
			++current_index;
			if (current_index == size_fsv_data) {
				current_index = 0;
			}
			return result;
		};
	}
	/**
	 * The lambda function works as follows:
	 *
	 * 1."fsv_index" keeps track of the current index in the original filtered_string_view.
	 *
	 * 2."rcount_index" keeps track of the current index in the substring.
	 *
	 * 3.The function checks if the current index in the original filtered_string_view is within
	 *   the range of the substring. If true, then the function checks if the character satisfies
	 *   the predicate of the original filtered_string_view. If true, then the function checks if
	 *   the current index in the substring is less than the specified count. If true, then the
	 *   function returns "true", indicating that the character should be included in the substring.
	 *   The function increments "rcount_index" in this case.
	 *
	 * 4.After checking if the character should be included in the substring, the function increments "fsv_index".
	 *
	 * 5.If "fsv_index" reaches the end of the original filtered_string_view, then it is
	 *   reset to 0 and "rcount_index" is reset to 0.
	 *
	 * 6.The function is mutable, which means that it can modify its captured variables.
	 *
	 * @param fsv_predicate The predicate function of the original filtered_string_view.
	 * @param substr_start The starting index of the substring.
	 * @param size_fsv_data The size of the original filtered_string_view.
	 * @param rcount The count of characters in the substring.
	 * @return A predicate to filter a substring of the original filtered_string_view.
	 */
	auto filter_substr(const fsv::filter fsv_predicate,
	                   const std::size_t substr_start,
	                   const std::size_t size_fsv_data,
	                   const std::size_t rcount) noexcept -> fsv::filter {
		auto fsv_index = std::size_t{0};
		auto rcount_index = std::size_t{0};
		return [fsv_predicate, fsv_index, substr_start, size_fsv_data, rcount, rcount_index](const char& c) mutable {
			auto result = false;
			if (fsv_index >= substr_start and fsv_index <= size_fsv_data) {
				if (fsv_predicate(c)) {
					if (rcount_index < rcount) {
						result = true;
						++rcount_index;
					}
				}
			}
			++fsv_index;
			if (fsv_index == size_fsv_data) {
				fsv_index = 0;
				rcount_index = 0;
			}
			return result;
		};
	}
} // namespace

// Non-Member Utility Function - Split
auto fsv::split(const filtered_string_view& fsv, const filtered_string_view& tok) noexcept
    -> std::vector<filtered_string_view> {
	auto result = std::vector<filtered_string_view>{};
	if (fsv.empty() or tok.empty()) {
		result.emplace_back(fsv);
		return result;
	}

	auto fsv_data = std::string{fsv.data()};
	auto tok_data = std::string{tok.data()};
	auto size_fsv_data = fsv_data.size();
	auto fsv_index = std::size_t{0};

	// Find the first occurrence of tok in fsv. The return value is the index of the
	// first character of tok, or std::string::npos if not found.
	auto end_split_index = fsv_data.find(tok_data);

	// Split fsv into substrings based on tok until tok is not found.
	while (end_split_index != std::string::npos) {
		// Add the substring from fsv_index to split_index - 1 to the result vector.
		// The filter_split function is used to calculate a predicate that filters the substring.
		result.emplace_back(fsv.data(), filter_split(fsv.predicate(), size_fsv_data, fsv_index, end_split_index - 1));

		// Move fsv_index to the position after tok.
		fsv_index = end_split_index + tok_data.size();

		// Find the next occurrence of tok in fsv. The return value is the index of the
		// first character of tok, or std::string::npos if not found.
		end_split_index = fsv_data.find(tok_data, fsv_index);
	}

	// Add the substring from fsv_index to the end of fsv to the result vector.
	// The filter_split function is used to calculate a predicate that filters the substring.
	result.emplace_back(fsv.data(), filter_split(fsv.predicate(), size_fsv_data, fsv_index, fsv_data.size()));
	return result;
}

// Non-Member Utility Function - Substr
auto fsv::substr(const filtered_string_view& fsv, int pos, int count) noexcept -> filtered_string_view {
	auto fsv_string = std::string{fsv.data()};
	auto size_fsv_data = fsv_string.size();

	// If "count" is less than or equal to 0, then the substring will have a length equal
	// to the length of the original filtered_string_view minus "pos".
	auto rcount = count <= 0 ? static_cast<int>(size_fsv_data) - pos : count;
	auto fsv_predicate = fsv.predicate();

	// Initialize a variable to keep track of the number of characters that should be
	// skipped at the beginning of the substring.
	auto fsv_offset = 0;

	// Iterate over the characters of the underlying string of fsv. If a character
	// satisfies the predicate function, then break out of the loop.
	for (auto i = size_t{0}; i < size_fsv_data; ++i) {
		if (fsv_predicate(fsv_string[i])) {
			break;
		}
		++fsv_offset;
	}

	// Calculate the starting index of the substring based on the value of "pos" and
	// the number of characters to skip at the beginning of the substring.
	auto substr_start = static_cast<std::size_t>(pos + fsv_offset);

	// Calculate a new predicate function that filters the substring based on the
	// original predicate function and the starting index and length of the substring.
	auto new_predicate = filter_substr(fsv_predicate, substr_start, size_fsv_data, static_cast<std::size_t>(rcount));

	// Create a new filtered_string_view that represents the substring of the original
	// filtered_string_view. The substring is created by using the underlying string
	// of the original filtered_string_view and the new predicate function.
	return filtered_string_view{fsv.data(), new_predicate};
}

// Iterator
fsv::filtered_string_view::iter::iter(const char* data, filter predicate) noexcept
: data_{data}
, predicate_{predicate} {
	while (not(predicate_(*data_)) and *data_ != '\0') {
		++data_;
	}
};

// helper function - iterate_pre_increment
auto fsv::filtered_string_view::iter::iterate_pre_increment() noexcept -> void {
	do {
		++data_;
	} while (not(predicate_(*data_)) and *data_ != '\0');
}
// helper function - iterate_pre_decrement
auto fsv::filtered_string_view::iter::iterate_pre_decrement() noexcept -> void {
	do {
		--data_;
	} while (not(predicate_(*data_)) and *data_ != '\0');
}

// Member Operator - Dereference
auto fsv::filtered_string_view::iter::operator*() const noexcept -> filtered_string_view::iter::reference {
	return *data_;
}

// Member Operator - Arrow
auto fsv::filtered_string_view::iter::operator->() const noexcept -> filtered_string_view::iter::pointer {}

// Member Operator - Pre-Increment
auto fsv::filtered_string_view::iter::operator++() noexcept -> filtered_string_view::iter& {
	fsv::filtered_string_view::iter::iterate_pre_increment();
	return *this;
}

// Member Operator - Post-Increment
auto fsv::filtered_string_view::iter::operator++(int) noexcept -> filtered_string_view::iter {
	auto old_this = *this;
	fsv::filtered_string_view::iter::iterate_pre_increment();
	return old_this;
}

// Member Operator - Pre-Decrement
auto fsv::filtered_string_view::iter::operator--() noexcept -> filtered_string_view::iter& {
	fsv::filtered_string_view::iter::iterate_pre_decrement();
	return *this;
}

// Member Operator - Post-Decrement
auto fsv::filtered_string_view::iter::operator--(int) noexcept -> filtered_string_view::iter {
	auto old_this = *this;
	fsv::filtered_string_view::iter::iterate_pre_decrement();
	return old_this;
}

// Range - Normal Begin
auto fsv::filtered_string_view::begin() const noexcept -> filtered_string_view::iterator {
	return iterator{data_, predicate_};
}

// Range - Constant Begin
auto fsv::filtered_string_view::cbegin() const noexcept -> filtered_string_view::const_iterator {
	return begin();
}

// Range - Normal End
auto fsv::filtered_string_view::end() const noexcept -> filtered_string_view::iterator {
	return iterator{data_ + size_, predicate_};
}

// Range - Constant End
auto fsv::filtered_string_view::cend() const noexcept -> filtered_string_view::const_iterator {
	return end();
}

// Range - Reverse Begin
auto fsv::filtered_string_view::rbegin() const noexcept -> filtered_string_view::reverse_iterator {
	auto forward_rbegin = reverse_iterator{end()};
	return forward_rbegin++;
}

// Range - Constant Reverse Begin
auto fsv::filtered_string_view::crbegin() const noexcept -> filtered_string_view::const_reverse_iterator {
	return rbegin();
}

// Range - Reverse End
auto fsv::filtered_string_view::rend() const noexcept -> filtered_string_view::reverse_iterator {
	// refer to https://en.cppreference.com/w/cpp/iterator/reverse_iterator
	auto forward_rend = reverse_iterator{begin()};
	return --forward_rend;
}

// Range - Constant Reverse End
auto fsv::filtered_string_view::crend() const noexcept -> filtered_string_view::const_reverse_iterator {
	return rend();
}
