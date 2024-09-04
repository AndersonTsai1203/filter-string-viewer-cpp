#include "./filtered_string_view.h"

#include <catch2/catch.hpp>
#include <set>
#include <sstream>

TEST_CASE("Default Constructor") {
	const auto sv = fsv::filtered_string_view{};
	CHECK(sv.empty());
	CHECK(sv.predicate()('a'));
}

TEST_CASE("Implicit String Constructor") {
	const auto s = std::string{"cat"};
	const auto sv = fsv::filtered_string_view{s};
	CHECK(sv.size() == 3);
}

TEST_CASE("String Constructor with Predicate") {
	const auto s = std::string{"cat"};
	const auto pred = [](const char& c) { return c == 'a'; };
	const auto sv = fsv::filtered_string_view{s, pred};
	CHECK(sv.size() == 1);
}

TEST_CASE("Implicit Null-Terminated String Constructor") {
	const auto sv = fsv::filtered_string_view{"cat"};
	CHECK(sv.size() == 3);
}

TEST_CASE("Null-Terminated String with Predicate Constructor") {
	const auto pred = [](const char& c) { return c == 'a'; };
	const auto sv = fsv::filtered_string_view{"cat", pred};
	CHECK(sv.size() == 1);
}

TEST_CASE("Copy Constructor") {
	auto sv1 = fsv::filtered_string_view{"bulldog"};
	const auto copy = sv1;
	CHECK(copy.data() == sv1.data());
}

TEST_CASE("Move Constructor") {
	auto sv1 = fsv::filtered_string_view{"bulldog"};
	const auto move = std::move(sv1);
	CHECK(sv1.data() == nullptr);
}

TEST_CASE("Copy Assignment") {
	const auto pred = [](const char& c) { return c == '4' || c == '2'; };
	const auto fsv1 = fsv::filtered_string_view{"42 bro", pred};
	auto fsv2 = fsv::filtered_string_view{};
	fsv2 = fsv1;
	CHECK(fsv1.data() == fsv2.data());
}

TEST_CASE("Move Assignment") {
	const auto pred = [](const char& c) { return c == '8' || c == '9'; };
	auto fsv1 = fsv::filtered_string_view{"'89 baby", pred};
	auto fsv2 = fsv::filtered_string_view{};
	const auto expected = "'89 baby";
	fsv2 = std::move(fsv1);
	CHECK(fsv1.empty());
	CHECK(fsv1.data() == nullptr);
	CHECK(fsv2.size() == 2);
	CHECK(fsv2.data() == expected);
}

TEST_CASE("Subscript - number") {
	const auto pred = [](const char& c) { return c == '9' || c == '0' || c == ' '; };
	const auto fsv1 = fsv::filtered_string_view{"1234567890123456", pred};
	CHECK(fsv1[1] == '0');
}

TEST_CASE("Subscript - string") {
	const auto pred = [](const char& c) { return c == '9' || c == '0' || c == ' '; };
	const auto fsv1 = fsv::filtered_string_view{"only 90s kids understand", pred};
	CHECK(fsv1[2] == '0');
}

TEST_CASE("String Type Conversion") {
	const auto fsv1 = fsv::filtered_string_view("vizsla");
	const auto str = static_cast<std::string>(fsv1);
	CHECK((fsv1.data() == str.data()) == false);
}

TEST_CASE("at") {
	const auto vowels = std::set<char>{'a', 'A', 'e', 'E', 'i', 'I', 'o', 'O', 'u', 'U'};
	const auto is_vowel = [&vowels](const char& c) { return vowels.contains(c); };
	const auto sv = fsv::filtered_string_view{"Malamute", is_vowel};
	CHECK(sv.size() == 4);
	CHECK(sv.at(0) == 'a');
	CHECK(sv.at(1) == 'a');
	CHECK(sv.at(2) == 'u');
	CHECK(sv.at(3) == 'e');
}

TEST_CASE("at - invalid index") {
	const auto sv = fsv::filtered_string_view{""};
	CHECK_THROWS_MATCHES(sv.at(0),
	                     std::domain_error,
	                     Catch::Matchers::Message("filtered_string_view::at(0): invalid index"));
	CHECK_THROWS_MATCHES(sv.at(-2),
	                     std::domain_error,
	                     Catch::Matchers::Message("filtered_string_view::at(-2): invalid index"));
	CHECK_THROWS_MATCHES(sv.at(2),
	                     std::domain_error,
	                     Catch::Matchers::Message("filtered_string_view::at(2): invalid index"));
}

TEST_CASE("size") {
	const auto sv = fsv::filtered_string_view{"Maltese"};
	CHECK(sv.size() == 7);
}

TEST_CASE("size - with predicate") {
	const auto sv = fsv::filtered_string_view{"Toy Poodle", [](const char& c) { return c == 'o'; }};
	CHECK(sv.size() == 3);
}

TEST_CASE("empty") {
	const auto sv = fsv::filtered_string_view{"Australian Shephard"};
	const auto empty_sv = fsv::filtered_string_view{};
	CHECK(!sv.empty());
	CHECK(empty_sv.empty());
}

TEST_CASE("empty - with predicate") {
	const auto sv = fsv::filtered_string_view{"Border Collie", [](const char& c) { return c == 'z'; }};
	CHECK(sv.empty());
}

TEST_CASE("data") {
	const auto s = "Sum 42";
	const auto sv = fsv::filtered_string_view{s};
	CHECK(sv.data() == s);
	CHECK(std::strcmp(sv.data(), s) == 0);
}

TEST_CASE("data - with predicate") {
	const auto s = "Sum 42";
	const auto sv = fsv::filtered_string_view{s, [](const char&) { return false; }};
	CHECK(sv.data() == s);
	CHECK(std::strcmp(sv.data(), s) == 0);
}

TEST_CASE("predicate") {
	const auto print_and_return_true = [](const char& c) { return c == 'g'; };
	const auto s1 = fsv::filtered_string_view{"doggo", print_and_return_true};
	const auto& predicate = s1.predicate();
	const auto expected_s1 = fsv::filtered_string_view{"gg"};
	CHECK(s1.size() == 2);
	CHECK(s1 == expected_s1);
	predicate('g');
}

TEST_CASE("Equality Comparison") {
	const auto lo = fsv::filtered_string_view{"aaa"};
	const auto hi = fsv::filtered_string_view{"aaa"};
	CHECK(lo == hi);
}

TEST_CASE("Equality Comparison - not equal") {
	const auto lo = fsv::filtered_string_view{"aaa"};
	const auto hi = fsv::filtered_string_view{"zzz"};
	CHECK(lo != hi);
}

TEST_CASE("Equality Comparison -  with predicate") {
	const auto lo = fsv::filtered_string_view{"aaa", [](const char& c) { return c == 'a'; }};
	const auto hi = fsv::filtered_string_view{"zzz", [](const char& c) { return c == 'z'; }};
	CHECK(lo != hi);
}

TEST_CASE("Relational Comparison") {
	const auto lo = fsv::filtered_string_view{"aaa"};
	const auto hi = fsv::filtered_string_view{"zzz"};
	CHECK((lo < hi) == true);
	CHECK((lo <= hi) == true);
	CHECK((lo > hi) == false);
	CHECK((lo >= hi) == false);
	CHECK(((lo <=> hi) == std::strong_ordering::less) == true);
}

TEST_CASE("Output Stream") {
	const auto fsv = fsv::filtered_string_view{"c++ > rust > java", [](const char& c) { return c == 'c' || c == '+'; }};
	std::ostringstream out_str_stream;
	out_str_stream << fsv;
	std::string str = out_str_stream.str();
	const auto expected_str = "c++";
	CHECK(str == expected_str);
}

TEST_CASE("Compose") {
	const auto best_languages = fsv::filtered_string_view{"c / c++"};
	const auto vf = std::vector<fsv::filter>{[](const char& c) { return c == 'c' || c == '+' || c == '/'; },
	                                         [](const char& c) { return c > ' '; },
	                                         [](const char&) { return true; }};
	const auto sv = fsv::compose(best_languages, vf);
	const auto expected_sv = fsv::filtered_string_view{"c/c++"};
	CHECK(sv.size() == 5);
	CHECK(sv == expected_sv);
}

TEST_CASE("Compose - empty vector of filters") {
	const auto best_languages = fsv::filtered_string_view{"c / c++"};
	const auto vf = std::vector<fsv::filter>{};
	const auto sv = fsv::compose(best_languages, vf);
	const auto expected_sv = "c / c++";
	CHECK(sv.size() == 7);
	CHECK(sv == expected_sv);
}

TEST_CASE("Split - tok is underlying string") {
	const auto interest = std::set<char>{'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F', ' ', '/'};
	const auto sv = fsv::filtered_string_view{"0xDEADBEEF / 0xdeadbeef",
	                                          [&interest](const char& c) { return interest.contains(c); }};
	const auto expected_sv = fsv::filtered_string_view{"DEADBEEF / deadbeef"};
	CHECK(sv == expected_sv);
	const auto tok = fsv::filtered_string_view{" / "};
	const auto v = fsv::split(sv, tok);
	CHECK(v.size() == 2);
	CHECK(v[0] == "DEADBEEF");
	CHECK(v[1] == "deadbeef");
}

TEST_CASE("Split - empty on both side") {
	const auto sv = fsv::filtered_string_view{"xax"};
	const auto tok = fsv::filtered_string_view{"x"};
	const auto v = fsv::split(sv, tok);
	const auto expected_v = std::vector<fsv::filtered_string_view>{"", "a", ""};
	CHECK(v == expected_v);
}

TEST_CASE("Split - empty all") {
	const auto sv = fsv::filtered_string_view{"xx"};
	const auto tok = fsv::filtered_string_view{"x"};
	const auto v = fsv::split(sv, tok);
	const auto expected_v = std::vector<fsv::filtered_string_view>{"", "", ""};
	CHECK(v == expected_v);
}

TEST_CASE("Split - tok empty") {
	const auto sv = fsv::filtered_string_view{"abcde"};
	const auto tok = fsv::filtered_string_view{""};
	const auto v = fsv::split(sv, tok);
	CHECK(v.size() == 1);
	CHECK(v[0] == "abcde");
}

TEST_CASE("Split - tok not underlying string") {
	const auto sv = fsv::filtered_string_view{"abcde"};
	const auto tok = fsv::filtered_string_view{"z"};
	const auto v = fsv::split(sv, tok);
	CHECK(v.size() == 1);
	CHECK(v[0] == "abcde");
}

TEST_CASE("Split - challenge") {
	const auto sv = fsv::filtered_string_view{"//a/aa/aaa//a//aaa///"};
	const auto tok = fsv::filtered_string_view{"/"};
	const auto v = fsv::split(sv, tok);
	const auto expected_v =
	    std::vector<fsv::filtered_string_view>{"", "", "a", "aa", "aaa", "", "a", "", "aaa", "", "", ""};
	CHECK(v.size() == expected_v.size());
	CHECK(v == expected_v);
}

TEST_CASE("Substr - without length") {
	const auto sv = fsv::filtered_string_view{"Siberian Husky"};
	const auto sub_sv = fsv::substr(sv, 9);
	const auto expected = "Husky";
	CHECK(sub_sv == expected);
	CHECK(sub_sv.size() == 5);
}

TEST_CASE("Substr - without position and length") {
	const auto sv = fsv::filtered_string_view{"Sled Dog", [](const char& c) { return c == 'a'; }};
	const auto sub_sv = fsv::substr(sv);
	const auto expected = "";
	CHECK(sub_sv == expected);
	CHECK(sub_sv.size() == 0);
}

TEST_CASE("Substr - with predicate") {
	const auto is_upper = [](const char& c) { return std::isupper(static_cast<unsigned char>(c)); };
	const auto sv = fsv::filtered_string_view{"Sled Dog", is_upper};
	const auto sub_sv = fsv::substr(sv, 0, 2);
	const auto expected = "SD";
	CHECK(sub_sv == expected);
	CHECK(sub_sv.size() == 2);
}

TEST_CASE("Substr") {
	const auto sv = fsv::filtered_string_view{"Sled Dog", [](const char& c) { return !(c == 'S' || c == 'D'); }};
	const auto expected_sv = fsv::filtered_string_view{"led og"};
	CHECK(sv == expected_sv);
	const auto sub_sv = fsv::substr(sv, 2, 4);
	const auto expected_sub_sv = "d og";
	CHECK(sub_sv.size() == 4);
	CHECK(sub_sv == expected_sub_sv);
}

TEST_CASE("Iterator - With default predicate") {
	const auto expect = std::vector<char>{'c', 'o', 'r', 'g', 'i'};
	auto result = std::vector<char>{};
	auto print_via_iterator = [&result](fsv::filtered_string_view const& sv) {
		std::copy(sv.begin(), sv.end(), std::back_inserter(result));
	};
	auto fsv1 = fsv::filtered_string_view{"corgi"};
	print_via_iterator(fsv1);
	CHECK(result == expect);
}

TEST_CASE("Iterator - With predicate which removes lowercase vowels") {
	const auto fsv = fsv::filtered_string_view{"samoyed", [](const char& c) {
		                                           return !(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
	                                           }};
	CHECK(fsv.size() == 4);
	const auto expected_fsv = "smyd";
	CHECK(fsv == expected_fsv);
	auto it = fsv.begin();
	CHECK(*it == 's');
	CHECK(*++it == 'm');
	CHECK(*++it == 'y');
	CHECK(*++it == 'd');
}

TEST_CASE("Iterator - without predicate") {
	const auto str = std::string("tosa");
	const auto s = fsv::filtered_string_view{str};
	auto sv = s.cend();
	CHECK(*std::prev(sv) == 'a');
	CHECK(*std::prev(sv, 2) == 's');
}

TEST_CASE("Iterator - pre increment") {
	const auto fsv = fsv::filtered_string_view{"abcd"};
	auto sv = fsv.begin();
	CHECK(*sv == 'a');
	CHECK(*++sv == 'b');
	CHECK(*++sv == 'c');
	CHECK(*++sv == 'd');
}

TEST_CASE("Iterator - post increment") {
	const auto fsv = fsv::filtered_string_view{"abcd"};
	auto sv = fsv.begin();
	CHECK(*sv++ == 'a');
	CHECK(*sv++ == 'b');
	CHECK(*sv++ == 'c');
	CHECK(*sv == 'd');
}

TEST_CASE("Iterator - pre decrement") {
	const auto fsv = fsv::filtered_string_view{"abcd"};
	auto it = fsv.end();
	CHECK(*--it == 'd');
	CHECK(*--it == 'c');
	CHECK(*--it == 'b');
	CHECK(*--it == 'a');
}

TEST_CASE("Iterator - post decrement") {
	const auto fsv = fsv::filtered_string_view{"abcd"};
	auto it = fsv.end();
	*it--;
	CHECK(*it-- == 'd');
	CHECK(*it-- == 'c');
	CHECK(*it-- == 'b');
	CHECK(*it == 'a');
}

TEST_CASE("Iterator - begin()") {
	const auto sv = fsv::filtered_string_view{"toast", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 3);
	const auto expected_sv = fsv::filtered_string_view{"oas"};
	CHECK(sv == expected_sv);
	auto it = sv.begin();
	CHECK(*it == 'o');
}

TEST_CASE("Iterator - end()") {
	const auto sv = fsv::filtered_string_view{"toast", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 3);
	const auto expected_sv = fsv::filtered_string_view{"oas"};
	CHECK(sv == expected_sv);
	auto it = sv.end();
	CHECK(*std::prev(it) == 's');
}

TEST_CASE("Iterator - cbegin()") {
	const auto sv = fsv::filtered_string_view{"toast", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 3);
	const auto expected_sv = fsv::filtered_string_view{"oas"};
	CHECK(sv == expected_sv);
	auto it = sv.cbegin();
	CHECK(*it == 'o');
}

TEST_CASE("Iterator - cend()") {
	const auto sv = fsv::filtered_string_view{"toast", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 3);
	const auto expected_sv = fsv::filtered_string_view{"oas"};
	CHECK(sv == expected_sv);
	auto it = sv.cend();
	CHECK(*std::prev(it) == 's');
}

TEST_CASE("Iterator - rbegin()") {
	const auto sv = fsv::filtered_string_view{"table", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 4);
	const auto expected_sv = fsv::filtered_string_view{"able"};
	CHECK(sv == expected_sv);
	auto it = sv.rbegin();
	CHECK(*it == 'e');
}

TEST_CASE("Iterator - rend()") {
	const auto sv = fsv::filtered_string_view{"table", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 4);
	const auto expected_sv = fsv::filtered_string_view{"able"};
	CHECK(sv == expected_sv);
	auto it = sv.rend();
	CHECK(*it == 'a');
}

TEST_CASE("Iterator - crbegin()") {
	const auto sv = fsv::filtered_string_view{"table", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 4);
	const auto expected_sv = fsv::filtered_string_view{"able"};
	CHECK(sv == expected_sv);
	auto it = sv.crbegin();
	CHECK(*it == 'e');
}

TEST_CASE("Iterator - crend()") {
	const auto sv = fsv::filtered_string_view{"table", [](const char& c) { return !(c == 't'); }};
	CHECK(sv.size() == 4);
	const auto expected_sv = fsv::filtered_string_view{"able"};
	CHECK(sv == expected_sv);
	auto it = sv.crend();
	CHECK(*it == 'a');
}

TEST_CASE("Iterator - Equality Comparison - not equal") {
	const auto str1 = std::string{"aaa"};
	const auto str2 = std::string{"bbb"};
	const auto lo = fsv::filtered_string_view{str1, [](const char& c) { return c == 'a'; }};
	const auto hi = fsv::filtered_string_view{str2, [](const char& c) { return c == 'b'; }};
	CHECK(lo.begin() != hi.begin());
	CHECK(lo.end() != hi.end());
}

TEST_CASE("Iterator - Equality Comparison") {
	const auto str = std::string{"aaa"};
	const auto f = [](const char& c) { return c == 'a'; };
	const auto lo = fsv::filtered_string_view{str, f};
	const auto hi = fsv::filtered_string_view{str, f};
	CHECK(lo.begin() == hi.begin());
	CHECK(lo.end() == hi.end());
	CHECK(*lo.crbegin() == 'a');
	CHECK(*hi.crbegin() == 'a');
}

TEST_CASE("Iterator - normal loop") {
	auto sv = fsv::filtered_string_view{"abcde"};
	CHECK(sv.size() == 5);
	CHECK(sv == "abcde");
	auto i = std::size_t{0};
	const auto sv_string = std::string{sv};
	for (auto it = sv.begin(); it != sv.end(); ++it, ++i) {
		CHECK(*it == sv_string[i]);
	}
}

TEST_CASE("Iterator - normal loop with predicate") {
	auto sv = fsv::filtered_string_view{"abcadef", [](const char& c) { return !(c == 'a'); }};
	auto expected_sv = fsv::filtered_string_view{"bcdef"};
	CHECK(sv.size() == 5);
	CHECK(sv == expected_sv);
	auto i = std::size_t{0};
	const auto sv_string = std::string{sv};
	for (auto it = sv.begin(); it != sv.end(); ++it, ++i) {
		CHECK(*it == sv_string[i]);
	}
}

TEST_CASE("Iterator - reverse loop") {
	auto sv = fsv::filtered_string_view{"vwxyz"};
	CHECK(sv.size() == 5);
	CHECK(sv == "vwxyz");
	auto i = std::size_t{0};
	const auto sv_string = "zyxwv";
	for (auto it = sv.rbegin(); it != sv.rend(); ++it, ++i) {
		CHECK(*it == sv_string[i]);
	}
}

TEST_CASE("Iterator - reverse loop with predicate") {
	auto sv = fsv::filtered_string_view{"uvwuxyz", [](const char& c) { return !(c == 'u'); }};
	auto expected_sv = fsv::filtered_string_view{"vwxyz"};
	CHECK(sv.size() == 5);
	CHECK(sv == expected_sv);
	auto i = std::size_t{0};
	const auto sv_string = "zyxwv";
	for (auto it = sv.rbegin(); it != sv.rend(); ++it, ++i) {
		CHECK(*it == sv_string[i]);
	}
}
