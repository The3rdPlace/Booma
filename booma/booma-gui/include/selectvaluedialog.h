#ifndef BOOMA_SELECTVALUEDIALOG_H
#define BOOMA_SELECTVALUEDIALOG_H

#include <iostream>
#include <vector>
#include <map>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>

class SelectValueDialog {

    private:

        static SelectValueDialog* _instance;

        bool _changed;
        std::string _title;
        std::string _text;
        std::string _description;
        std::map<int, std::string> _values;

        std::string _selectedValue;
        int _selectedId;

        Fl_Window* _win;
        Fl_Output* _descriptionField;
        Fl_Choice* _valuesField;
        Fl_Button* _okButton;
        Fl_Button* _cancelButton;

    public:

        SelectValueDialog(std::string title, std::string text, std::string description, std::vector<std::string> values, std::string selectedValue = "");
        SelectValueDialog(std::string title, std::string text, std::string description, std::map<int, std::string> values, int selectedId = -1);
        ~SelectValueDialog();

        static SelectValueDialog* GetInstance() {
            return _instance;
        }

        bool Show();

        void Ok();
        void Cancel();

        std::string SelectedValue() {
            return _selectedValue;
        }

        int SelectedId() {
            return _selectedId;
        }
};

#endif