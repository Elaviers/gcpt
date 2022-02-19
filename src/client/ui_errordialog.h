#pragma once
#include "ui_container.h"
#include "ui_text_button.h"

class UI_ErrorDialog
{
	UIContainer _dlg;
	UIPanel _panel;
	UILabel _title;
	UILabel _text;
	UITextButton _btn;
	
public:
	UI_ErrorDialog(const Text& msgString);
	~UI_ErrorDialog() = default;

};

