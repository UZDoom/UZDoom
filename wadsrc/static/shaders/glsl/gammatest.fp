/*
** gammatest.fp
**
** Gamma Test pattern shown in image calibration screen
**
**---------------------------------------------------------------------------
**
** Copyright 2025 Marcus Minhorst
** Copyright 2025 Dileep V. Reddy
** Copyright 2025 UZDoom Maintainers and Contributors
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
**---------------------------------------------------------------------------
**
** https://phinet.ca/projects/gamma/description/
*/

void main()
{
	vec3 colour = texture(InputTexture, TexCoord).rgb;
	if (gl_FragCoord.x > uXmin && gl_FragCoord.x < uXmax && gl_FragCoord.y > uYmin && gl_FragCoord.y < uYmax)
	{
		float u_gamma = 1.0;
		float u_black_point = 0.0;
		float u_white_point = 1.0;

		float checker_size = 32.0;
		vec2 checker_coord = floor(gl_FragCoord.xy / checker_size);
		vec3 base_color;

		float col = floor(mod(checker_coord.x, 4.0));
		float row = floor(mod(checker_coord.y, 4.0));
		float dither_pattern = mod(gl_FragCoord.x + gl_FragCoord.y, 2.0);

		vec3 colorA = vec3(0.5);
		vec3 colorB = mix(vec3(0.0), vec3(1.0), dither_pattern);
		vec3 colorC = mix(vec3(uBlackPoint), vec3(1.0), dither_pattern);
		vec3 colorD = mix(vec3(0.0), vec3(uWhitePoint), dither_pattern);

		if (row < 1.0) { // Row 0: A B C D
			if (col < 1.0) { base_color = colorA; }
			else if (col < 2.0) { base_color = colorB; }
			else if (col < 3.0) { base_color = colorC; }
			else { base_color = colorD; }
		} else if (row < 2.0) { // Row 1: A B D C
			if (col < 1.0) { base_color = colorA; }
			else if (col < 2.0) { base_color = colorB; }
			else if (col < 3.0) { base_color = colorD; }
			else { base_color = colorC; }
		} else if (row < 3.0) { // Row 2: C D A B
			if (col < 1.0) { base_color = colorC; }
			else if (col < 2.0) { base_color = colorD; }
			else if (col < 3.0) { base_color = colorA; }
			else { base_color = colorB; }
		} else { // Row 3: D C A B
			if (col < 1.0) { base_color = colorD; }
			else if (col < 2.0) { base_color = colorC; }
			else if (col < 3.0) { base_color = colorA; }
			else { base_color = colorB; }
		}

		// Need to precompensate for ApplyGamma() here, if we can't disable it.
		vec3 final_color = pow(base_color, vec3(uInvGamma));
		FragColor = vec4(clamp(final_color, 0.0, 1.0), 1.0);
	}
	else
	{
		FragColor = vec4(colour, 1.0);
	}
}
