#include "getvaluedialog.h"

GetValueDialog* GetValueDialog::_instance;

void HandleValueOkButtonCallback(Fl_Widget* w, void* data) {
    GetValueDialog::GetInstance()->Ok();
}

void HandleValueCancelButtonCallback(Fl_Widget* w, void* data) {
    GetValueDialog::GetInstance()->Cancel();
}

GetValueDialog::GetValueDialog(std::string title, std::string text, std::string description, std::string defaultValue):
    _title(title),
    _text(text),
    _description(description),
    _value(defaultValue),
    _changed(false) {
    _instance = this;
}

GetValueDialog::~GetValueDialog() {

}
#include <FL/fl_draw.H>

bool GetValueDialog::Show() {

    // Length of the description string. This sets the dialog width
    int w = fl_width(_description.c_str()) + 10;

    // Create window
    _win = new Fl_Window(w + 20, 160);
    _win->set_modal();

    // Set dialog title
    char title[256];
    strcpy(title, _title.c_str());
    _win->label(title);

    // Add fields
    _descriptionField = new Fl_Output(10, 10, w, 30);
    _descriptionField->value(_description.c_str());
    _descriptionField->box(FL_NO_BOX);
    _descriptionField->visible_focus(0);
    int lw = fl_width(_text.c_str());
    _valueField = new Fl_Input(lw + 10, 50, _win->w() - lw - 20, 30, _text.c_str());
    _valueField->value(_value.c_str());
    _okButton = new Fl_Button(w - 130 + 20, 120, 50, 30, "&Ok");
    _okButton->callback(HandleValueOkButtonCallback);
    _cancelButton = new Fl_Button(w - 70 + 20, 120, 60, 30, "&Cancel");
    _cancelButton->callback(HandleValueCancelButtonCallback);

    // Show the windows
    _win->end();
    _win->show();
    _win->focus(_valueField);

    // Wait for the dialog to be closed
    while (_win->shown()) Fl::wait();

    // Signal if we pressed or cancel
    return _changed;
}

void GetValueDialog::Ok() {
    _changed = true;
    _value = _valueField->value();
    _win->hide();
}

void GetValueDialog::Cancel() {
    _win->hide();
}