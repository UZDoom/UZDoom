header_install_dir = get_option('includedir') / 'wayland-protocols'
foreach protocol_file : protocol_files
	header_name = fs.name(protocol_file).replace('.xml', '-enum.h')
	headers += custom_target(
		header_name,
		output: header_name,
		input: '../..' / protocol_file,
		command: [
			prog_scanner,
			'--strict',
			'enum-header',
			'@INPUT@',
			'@OUTPUT@',
		],
		install: true,
		install_dir: header_install_dir,
	)
endforeach
