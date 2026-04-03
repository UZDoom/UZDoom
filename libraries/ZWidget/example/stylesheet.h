#pragma once

static const char* stylesheet = R"xxxx(

.dark {
	--bgMain:   #2A2A2A; /* background */
	--fgMain:   #E2DFDB;
	--bgLight:  #212121; /* headers / inputs  */
	--fgLight:  #E2DFDB;
	--bgAction: #444444; /* interactive elements */
	--fgAction: #FFFFFF;
	--bgHover:  #003C88; /* hover / highlight */
	--fgHover:  #FFFFFF;
	--bgActive: #004FB4; /* click */
	--fgActive: #FFFFFF;
	--border:   #646464; /* around elements */
	--divider:  #555555; /* between elements */
}

.light, .aero {
	--bgMain:   #F0F0F0; /* background */
	--fgMain:   #191919;
	--bgLight:  #FAFAFA; /* headers / inputs  */
	--fgLight:  #191919;
	--bgAction: #C8C8C8; /* interactive elements */
	--fgAction: #000000;
	--bgHover:  #D2D2FF; /* hover / highlight */
	--fgHover:  #000000;
	--bgActive: #C7B4FF; /* click */
	--fgActive: #000000;
	--border:   #A0A0A0; /* around elements */
	--divider:  #B9B9B9; /* between elements */
}

widget {
	font-family: "system";
	font-size: 13;
	color: var(--fgMain);
	window-background: var(--bgMain);
	window-border: var(--bgMain);
	window-caption-color: var(--bgLight);
	window-caption-text-color: var(--fgLight);
}

textlabel {
}

pushbutton {
	noncontent-left: 10;
	noncontent-top: 5;
	noncontent-right: 10;
	noncontent-bottom: 5;
	color: var(--fgAction);
	background-color: var(--bgAction);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
}

pushbutton.aero {
	border-image-source: url("button_normal.png");
	border-top-width: 6;
	border-bottom-width: 6;
	border-left-width: 5;
	border-right-width: 5;
	border-top-image-slice: 6;
	border-bottom-image-slice: 6;
	border-left-image-slice: 5;
	border-right-image-slice: 5;
	border-top-image-width: 6;
	border-bottom-image-width: 6;
	border-left-image-width: 5;
	border-right-image-width: 5;
	border-center-image-slice: true;
}

pushbutton:hover {
	color: var(--fgHover);
	background-color: var(--bgHover);
}

pushbutton.aero:hover {
	border-image-source: url("button_hot.png");
}

pushbutton:down {
	color: var(--fgActive);
	background-color: var(--bgActive);
}

lineedit {
	noncontent-left: 5;
	noncontent-top: 3;
	noncontent-right: 5;
	noncontent-bottom: 3;
	color: var(--fgLight);
	background-color: var(--bgLight);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
	selection-color: var(--bgHover);
	no-focus-selection-color: var(--bgHover);
}

textedit {
	font-family: "monospace";
	noncontent-left: 8;
	noncontent-top: 8;
	noncontent-right: 8;
	noncontent-bottom: 8;
	color: var(--fgLight);
	background-color: var(--bgLight);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
	selection-color: var(--bgHover);
}

listview {
	noncontent-left: 1;
	noncontent-top: 1;
	noncontent-right: 1;
	noncontent-bottom: 1;
	background-color: var(--bgLight);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
}

listview-body {
	color: var(--fgLight);
	selection-color: var(--bgHover);
	noncontent-top: 2;
	noncontent-left: 8;
	noncontent-bottom: 9;
}

listview-header {
	background-color: var(--bgAction);
	color: var(--fgLight);
	noncontent-left: 10;
	noncontent-right: 2;
}

dropdown {
	noncontent-left: 5;
	noncontent-top: 5;
	noncontent-right: 5;
	noncontent-bottom: 5;
	color: var(--fgLight);
	background-color: var(--bgLight);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
	arrow-color: var(--border);
}

scrollbar {
	track-color: var(--divider);
	thumb-color: var(--border);
	noncontent-top: 5;
	noncontent-bottom: 5;
}

tabbar {
	spacer-left: 20;
	spacer-right: 20;
	background-color: var(--bgLight);
}

tabbar-tab {
	noncontent-left: 15;
	noncontent-right: 15;
	noncontent-top: 1;
	noncontent-bottom: 1;
	color: var(--bgMain);
	background-color: var(--bgMain);
	border-left-color: var(--divider);
	border-top-color: var(--divider);
	border-right-color: var(--divider);
	border-bottom-color: var(--border);
}

tabbar-tab:hover {
	color: var(--fgAction);
	background-color: var(--bgAction);
}

tabbar-tab:active {
	background-color: var(--bgMain);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: transparent;
}

tabbar-spacer {
	noncontent-bottom: 1;
	border-bottom-color: var(--border);
}

tabwidget-stack {
	noncontent-left: 20;
	noncontent-top: 5;
	noncontent-right: 20;
	noncontent-bottom: 5;
}

checkbox-label {
	checked-outer-border-color: var(--border);
	checked-inner-border-color: var(--bgMain);
	checked-color: var(--fgMain);
	unchecked-outer-border-color: var(--border);
	unchecked-inner-border-color: var(--bgMain);
}

checkbox-label.aero {
	checked-image: url("checkbox_checked_normal.png");
	checked-align: -3;
	unchecked-image: url("checkbox_unchecked_normal.png");
	unchecked-align: -3;
}

menubar {
	background-color: var(--bgLight);
}

menubaritem {
	color: var(--fgMain);
}

menubaritem:hover {
	color: var(--fgHover);
	background-color: var(--bgHover);
}

menubaritem:down {
	color: var(--fgActive);
	background-color: var(--bgActive);
}

menu {
	noncontent-left: 5;
	noncontent-top: 5;
	noncontent-right: 5;
	noncontent-bottom: 5;
	color: var(--fgMain);
	background-color: var(--bgMain);
	border-left-color: var(--border);
	border-top-color: var(--border);
	border-right-color: var(--border);
	border-bottom-color: var(--border);
}

menuitem {
}

menuitem:hover {
	color: var(--fgHover);
	background-color: var(--bgHover);
}

menuitem:down {
	color: var(--fgActive);
	background-color: var(--bgActive);
}

toolbar {
	background-color: var(--bgLight);
}

toolbarbutton {
}

toolbarbutton:hover {
	color: var(--fgHover);
	background-color: var(--bgHover);
}

toolbarbutton:down {
	color: var(--fgActive);
	background-color: var(--bgActive);
}

statusbar {
	background-color: var(--bgLight);
}

)xxxx";
