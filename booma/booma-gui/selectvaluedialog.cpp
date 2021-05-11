#include "selectvaluedialog.h"

SelectValueDialog* SelectValueDialog::_instance;

void HandleValueSelectedOkButtonCallback(Fl_Widget* w, void* data) {
    SelectValueDialog::GetInstance()->Ok();
}

void HandleValueSelectedCancelButtonCallback(Fl_Widget* w, void* data) {
    SelectValueDialog::GetInstance()->Cancel();
}

SelectValueDialog::SelectValueDialog(std::string title, std::string text, std::string description, std::vector<std::string> values, std::string selectedValue):
    _title(title),
    _text(text),
    _description(description),
    _changed(false) {
    _instance = this;

    int id = 0;
    for( std::vector<std::string>::iterator it = values.begin(); it != values.end(); it++ ) {
        _values.insert(std::pair<int, std::string>(id, (*it)));
        if( selectedValue == (*it) ) {
            _selectedId = id;
        }
        id++;
    }
}

SelectValueDialog::SelectValueDialog(std::string title, std::string text, std::string description, std::map<int, std::string> values, int selectedId):
    _title(title),
    _text(text),
    _description(description),
    _changed(false),
    _selectedId(selectedId) {
    _instance = this;
    _values = values;
}

SelectValueDialog::~SelectValueDialog() {

}

bool SelectValueDialog::Show() {

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
    _valuesField = new Fl_Choice(lw + 10, 50, _win->w() - lw - 20, 30, _text.c_str());
    for( std::map<int, std::string>::iterator it = _values.begin(); it != _values.end(); it++ ) {
        _valuesField->add((*it).second.c_str());
    }
    _valuesField->value(_selectedId);
    _okButton = new Fl_Button(w - 130 + 20, 120, 50, 30, "&Ok");
    _okButton->callback(HandleValueSelectedOkButtonCallback);
    _cancelButton = new Fl_Button(w - 70 + 20, 120, 60, 30, "&Cancel");
    _cancelButton->callback(HandleValueSelectedCancelButtonCallback);

    // Show the windows
    _win->end();
    _win->show();
    _win->focus(_valuesField);

    // Wait for the dialog to be closed
    while (_win->shown()) Fl::wait();

    // Signal if we pressed or cancel
    return _changed;
}

void SelectValueDialog::Ok() {
    _selectedId = -1;
    _selectedValue = "";
    if( _valuesField->value() != -1 ) {
        int index = 0;
        for( std::map<int, std::string>::iterator it = _values.begin(); it != _values.end(); it++ ) {
            if (index++ == _valuesField->value()) {
                _selectedId = (*it).first;
                _selectedValue = (*it).second;
                _changed = true;
            }
        }
    }
    _win->hide();
}

void SelectValueDialog::Cancel() {
    _win->hide();
}