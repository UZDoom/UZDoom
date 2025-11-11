#!/usr/bin/env gawk -f

# why did I choose to use awk for this again?

# msgstr block to string
function clean_string(first_line, start_index, i, combined_str, line)
{
	combined_str = first_line

	# concat lines
	for (i = start_index + 1; i <= NF; i++) {
		line = $i

		if (line ~ /^"/) {
			combined_str = combined_str "\n" line
		} else {
			break
		}
	}

	# cleanup joins
	gsub(/^(msgid |msgstr |msgctxt )/, "", combined_str)
	gsub(/^"|"$/, "", combined_str)
	gsub(/"\n"/, "", combined_str)

	# unescape
	gsub(/\\"/, "\"", combined_str)
	gsub(/\\t/, "\t", combined_str)
	gsub(/\\\\/, "\\", combined_str)
	gsub(/\\n/, "\n", combined_str)

	# escape
	gsub(/"/, "\"\"", combined_str)

	# emit
	if (combined_str ~ /\n/ || combined_str ~ OFS) {
		return "\"" combined_str "\""
	} else {
		return combined_str
	}
}

BEGIN {
	RS = "(\r?\n){2,}"
	FS = "\r?\n"
	OFS = ","
	SUBSEP = "."
	IGNORECASE = 1
}

{
	# get lang from filename
	lang = FILENAME
	sub(/\.po$/, "", lang)
	sub(/.*\//, "", lang)

	# store lang
	if (!lang_seen[lang]) {
		langs[lang_count++] = lang
		lang_seen[lang] = 1
	}

	# check for header block
	is_header = 0
	for (i = 1; i <= NF; i++) {
		if ($i ~ /^msgid ""/) {
			is_header = 1
			break
		}
	}

	# process header
	if (is_header) {
		# find "HeaderCode:"
		for (i = 1; i <= NF; i++) {
			if (match($i, /"[[:space:]]*HeaderCode:[[:space:]]*(.+)[[:space:]]*\\n/, m)) {
				lang_headers[lang] = m[1] # store header code
				break
			}
		}
		next
	}

	current_msgid = ""
	current_msgstr = ""
	current_msgctxt = ""

	# find msgid/msgstr
	for (i = 1; i <= NF; i++) {
		if ($i ~ /^msgid /) {
			current_msgid = clean_string($i, i)
			# advance i
			for (j = i + 1; j <= NF; j++) {
				if ($j ~ /^"/) { i++ } else { break }
			}
		} else if ($i ~ /^msgctxt /) {
			current_msgctxt = clean_string($i, i)
			# advance i
			for (j = i + 1; j <= NF; j++) {
				if ($j ~ /^"/) { i++ } else { break }
			}
		} else if ($i ~ /^msgstr /) {
			current_msgstr = clean_string($i, i)
			# advance i
			for (j = i + 1; j <= NF; j++) {
				if ($j ~ /^"/) { i++ } else { break }
			}
		}
	}

	# store translation
	if (current_msgid != "" && current_msgid != "\"\"") {
		unique_key = current_msgid SUBSEP current_msgctxt

		if (!msgid_seen[unique_key]) {
			msgids[msgid_count++] = unique_key
			msgid_seen[unique_key] = 1
		}
		translations[unique_key SUBSEP lang] = current_msgstr
	}
}

# END block
END {
	# header row
	printf "default%sIdentifier%sRemarks%sFilter", OFS, OFS, OFS
	for (i = 0; i < lang_count; i++) {
		lang_id = langs[i]
		if (lang_id == "en_US") {
			continue
		}

		header_val = lang_headers[lang_id]
		if (header_val == "") {
			header_val = lang_id # fallback to lang id
		}
		printf "%s%s", OFS, header_val
	}
	printf "\n"

	# empty row
	if (msgid_count > 0) {
		printf ""
		for (j = 0; j < (lang_count + 2); j++) {
			if (j == 2) printf "---"
			printf "%s", OFS
		}
		printf "\n"
	}

	# data rows
	for (i = 0; i < msgid_count; i++) {
		unique_key = msgids[i]
		split(unique_key, key_parts, SUBSEP)
		msgctxt = key_parts[1]
		msgid = key_parts[2]

		if (msgid == "") {
			msgid = msgctxt
			msgctxt = ""
		}

		# en_US col
		en_str = translations[unique_key SUBSEP "en_US"]
		if (en_str == "") {
			en_str = ""
		}
		printf "%s", en_str

		printf "%s%s%s%s%s", OFS, msgid, OFS, OFS, msgctxt

		# print lang cols
		for (j = 0; j < lang_count; j++) {
			lang = langs[j]
			if (lang == "en_US") {
				continue
			}

			msgstr = translations[unique_key SUBSEP lang]

			# handle missing translation
			if (msgstr == "") {
				msgstr = ""
			}
			printf "%s%s", OFS, msgstr
		}
		printf "\n"
	}
}
