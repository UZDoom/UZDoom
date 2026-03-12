#pragma once

#include <string>
#include <vector>
#include <functional>

enum class ThemeStyleTokenType
{
	null,                 /// No more tokens
	ident,                /// Identifier
	atkeyword,            /// @ keyword
	string,               /// String
	invalid,              /// Syntax error
	hash,                 /// # Hash
	number,               /// Number
	percentage,           /// Percentage number
	dimension,            /// Number with dimension
	uri,                  /// Url function
	unicode_range,        /// Unicode range
	cdo,                  /// HTLM comment begin <!--
	cdc,                  /// HTML comment end -->
	colon,                /// :
	semi_colon,           /// ;
	curly_brace_begin,    /// {
	curly_brace_end,      /// }
	bracket_begin,        /// (
	bracket_end,          /// )
	square_bracket_begin, /// [
	square_bracket_end,   /// ]
	whitespace,           /// Whitespace
	comment,              /// C style comment block
	function,             /// Function name
	includes,             /// ~=
	dashmatch,            /// |=
	delim                 /// Any other character
};

class ThemeStyleToken
{
public:
	ThemeStyleTokenType type = ThemeStyleTokenType::null;
	std::string value;
	std::string dimension;
	size_t offset = 0; // Offset where token began
};

class ThemeStyleTokenizer
{
public:
	static void parse(const std::string& properties, std::function<void(std::string name, std::vector<ThemeStyleToken> tokens, bool important_flag)> parse_property);

	ThemeStyleTokenizer(const std::string& text);

	void read(ThemeStyleToken& out_token, bool eat_whitespace, bool eat_comments = true);
	std::vector<ThemeStyleToken> read_property_value(ThemeStyleToken& token, bool& out_important_flag);

	static bool compare_case_insensitive(const std::string& a, const std::string& b);

private:
	void read(ThemeStyleToken& out_token);
	void peek(ThemeStyleToken& out_token);

	void read_atkeyword(ThemeStyleToken& out_token);
	void read_hash(ThemeStyleToken& out_token);
	void read_cdo(ThemeStyleToken& out_token);
	void read_cdc(ThemeStyleToken& out_token);
	void read_comment(ThemeStyleToken& out_token);
	void read_uri(ThemeStyleToken& out_token);
	void read_function(ThemeStyleToken& out_token);
	void read_whitespace(ThemeStyleToken& out_token);
	void read_includes(ThemeStyleToken& out_token);
	void read_dashmatch(ThemeStyleToken& out_token);
	void read_number_type(ThemeStyleToken& out_token);
	size_t read_ident(size_t p, std::string& out_ident);
	size_t read_name(size_t p, std::string& out_ident);
	inline size_t read_nmstart(size_t p, std::string::value_type& out_c);
	inline size_t read_nmchar(size_t p, std::string::value_type& out_c);
	size_t read_string(size_t p, std::string& out_str, std::string::value_type str_char) const;
	size_t read_invalid(size_t p) const;
	size_t read_uri_nonquoted_string(size_t p, std::string& out_str) const;
	inline static bool is_whitespace(std::string::value_type c);

	std::string doc;
	size_t pos = 0;
};
