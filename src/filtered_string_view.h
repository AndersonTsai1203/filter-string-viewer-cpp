#ifndef COMP6771_ASS2_FSV_H
#define COMP6771_ASS2_FSV_H

#include <algorithm>
#include <compare>
#include <cstring>
#include <functional>
#include <iterator>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace fsv {
	using filter = std::function<bool(const char&)>;

	class filtered_string_view {
		class iter {
		 public:
			friend class filtered_string_view;

			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = char;
			using pointer = void;
			using reference = const char&;
			using difference_type = std::ptrdiff_t;

			iter() noexcept = default;
			iter(const char* data, filter predicate) noexcept;

			auto operator*() const noexcept -> reference;
			auto operator->() const noexcept -> pointer;

			auto operator++() noexcept -> iter&;
			auto operator++(int) noexcept -> iter;
			auto operator--() noexcept -> iter&;
			auto operator--(int) noexcept -> iter;

			friend auto operator==(const iter& lhs, const iter& rhs) noexcept -> bool {
				if (lhs.data_ != rhs.data_) {
					return false;
				}
				if (lhs.predicate_.target_type() != rhs.predicate_.target_type()) {
					return false;
				}
				return true;
			}
			friend auto operator!=(const iter& lhs, const iter& rhs) noexcept -> bool {
				return not(lhs == rhs);
			}

		 private:
			const char* data_;
			filter predicate_;
			void iterate_pre_increment() noexcept;
			void iterate_pre_decrement() noexcept;
		};

	 public:
		// Static Data Members
		static filter default_predicate;

		// Default Constructor
		filtered_string_view() noexcept;

		// Implicit String Constructor
		filtered_string_view(const std::string& str) noexcept;

		// String Constructor with Predicate
		filtered_string_view(const std::string& str, filter predicate) noexcept;

		// Implicit Null-Terminated String Constructor
		filtered_string_view(const char* str) noexcept;

		// Null-Terminated String with Predicate Constructor
		filtered_string_view(const char* str, filter predicate) noexcept;

		// Copy Constructor
		filtered_string_view(const filtered_string_view& other) noexcept;

		// Move Constructor
		filtered_string_view(filtered_string_view&& other) noexcept;

		// Destructor
		~filtered_string_view() noexcept = default;

		/**
		 * Member Operators Section
		 */
		// Copy Assignment
		auto operator=(const filtered_string_view& other) noexcept -> filtered_string_view&;

		// Move Assignment
		auto operator=(filtered_string_view&& other) noexcept -> filtered_string_view&;

		// Subscript
		auto operator[](int n) const noexcept -> const char&;

		// String Type Conversion
		explicit operator std::string() const noexcept;

		// Member Functions
		[[nodiscard]] auto at(int index) const -> const char&;
		[[nodiscard]] auto size() const noexcept -> std::size_t;
		[[nodiscard]] auto empty() const noexcept -> bool;
		[[nodiscard]] auto data() const noexcept -> const char*;
		[[nodiscard]] auto predicate() const noexcept -> const filter&;

		/**
		 * Iterators Section
		 */
		using iterator = iter;
		using const_iterator = iter;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		auto begin() const noexcept -> iterator;
		auto cbegin() const noexcept -> const_iterator;

		auto end() const noexcept -> iterator;
		auto cend() const noexcept -> const_iterator;

		auto rbegin() const noexcept -> reverse_iterator;
		auto crbegin() const noexcept -> const_reverse_iterator;

		auto rend() const noexcept -> reverse_iterator;
		auto crend() const noexcept -> const_reverse_iterator;

	 private:
		const char* data_;
		std::size_t size_;
		filter predicate_;
	};

	/**
	 * Non-Member Operators
	 */
	// Equality Comparison
	auto operator==(const filtered_string_view& lhs, const filtered_string_view& rhs) noexcept -> bool;

	// Relational Comparison
	auto operator<=>(const filtered_string_view& lhs, const filtered_string_view& rhs) noexcept -> std::strong_ordering;

	// Output Stream
	auto operator<<(std::ostream& os, const filtered_string_view& fsv) noexcept -> std::ostream&;

	/**
	 * Non-Member Utility Functions
	 */
	// Compose
	auto compose(const filtered_string_view& fsv, const std::vector<filter>& filts) noexcept -> filtered_string_view;

	// Split
	auto split(const filtered_string_view& fsv, const filtered_string_view& tok) noexcept
	    -> std::vector<filtered_string_view>;

	// SubStr
	auto substr(const filtered_string_view& fsv, int pos = 0, int count = 0) noexcept -> filtered_string_view;

} // namespace fsv

#endif // COMP6771_ASS2_FSV_H
