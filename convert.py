import pandas as pd
import re
import polib
import os

# this file converts gzdoom's language table to gnu gettext files
# there is really no reason to use this other than to sync up repos

def to_gettext(sheet, page, output_dir=None):
	print(f"read {sheet}/{page}")

	data = pd.read_excel(sheet, sheet_name=page)

	if (output_dir == None):
		output_dir=re.sub("[^a-z0-9]+", "_", page.lower())

	source_text_col = 'default'
	key_col = 'Identifier'
	comment_col = 'Remarks'
	context_col = 'Filter'
	source_lang_code = 'en_US'

	mapping = {
		# ba: # !!! Bashkir
		# bg: # !!! Bulgarian
		"by": "be_Latn", # Belarusian
		# cs: # !!! Czech
		# da: # Danish
		# de: # German
		"default": "en_US", # American English
		# el: # Greek
		"eng enc ena enz eni ens enj enb enl ent enw": "en_GB", # British English
		# eo: # Esperanto
		# es: # Spanish
		"esm esn esg esc esa esd esv eso esr ess esf esl esy esz esb ese esh esi esu": "es_MX", # Mexican Spanish
		# fi: # Finnish
		# fr: # French
		# hr: # Croatian
		# hu: # Hungarian
		# it: # Italian
		"ja jp": "ja", # Japanese
		"jp": "ja", # Japanese
		# ko: # Korean
		# nl: # Dutch
		"no nb": "nb_NO", # Norwegian
		# pl: # Polish
		"pt": "pt_BR", # Brazilian Portuguese
		"ptg": "pt", # Portuguese
		# ro: # Romanian
		# ru: # Russian
		# sr: # Serbian
		# sv: # Swedish
		# tr: # Turkish
		# uk: # Ukrainian
	}

	ignored_cols = [key_col, comment_col, context_col]

	lang_cols = [col for col in data.columns if col not in ignored_cols]

	if not os.path.exists(output_dir):
		os.makedirs(output_dir)
		print(f"mkdir {output_dir}")

	def to_gettext_file(in_name, out_name):

		print(f"  in: {out_name or 'Template'}")

		po_trans = polib.POFile()
		po_trans.header = "UZDoom translation file"
		po_trans.metadata = {
			'Project-Id-Version': '1.0',
			'Content-Type': 'text/plain; charset=utf-8',
			'MIME-Version': '1.0',
		}

		if out_name:
			po_trans.metadata['Language'] = out_name
			po_trans.metadata['HeaderCode'] = in_name

		for _, row in data.iterrows():
			if pd.notna(row[key_col]):
				entry = polib.POEntry(
					msgid=str(row[key_col]),
					msgstr=str(row[in_name]) if (out_name and pd.notna(row[in_name])) else '',
					tcomment=str(row[comment_col]) if pd.notna(row[comment_col]) else '',
					msgctxt=str(row[context_col]) if pd.notna(row[context_col]) else None
				)
				po_trans.append(entry)

		out_name = f"{out_name}.po" if out_name else "template.pot"

		po_trans.save(os.path.join(output_dir, out_name))
		print(f"  out: {out_name}")

	to_gettext_file('default', None)

	for lang_col_name in lang_cols:
		output_lang_code = mapping[lang_col_name] if lang_col_name in mapping else lang_col_name
		to_gettext_file(lang_col_name, output_lang_code)

def to_csv(sheet, page):
	print(f"read {sheet}/{page}")

	data = pd.read_excel(sheet, sheet_name=page)

	output=re.sub("[^a-z0-9]+", "_", page.lower())

	data.to_csv(f"{output}.csv", index=False)

	print(f"out {output}.csv")

if __name__ == "__main__":
	sheet="./GZDoom and Raze Strings.xlsx"

	to_gettext(sheet, "Common")
	to_gettext(sheet, "GZDoom Engine Strings", "zdoom_engine_strings")
	to_gettext(sheet, "GZDoom Game Strings", "zdoom_game_strings")
	to_gettext(sheet, "Chex Quest 3")
	to_gettext(sheet, "Harmony")
	to_gettext(sheet, "Hacx")
	to_csv(sheet, "Macros")
	# to_gettext(sheet, "Raze",  "zdoom_engine_strings")
	# to_gettext(sheet, "Unused content")
