#ifndef BOOMA_GETVALUEDIALOG_H
#define BOOMA_GETVALUEDIALOG_H

#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>

class GetValueDialog {

    private:

        static GetValueDialog* _instance;

        bool _changed;
        std::string _title;
        std::string _text;
        std::string _description;
        std::string _value;

        Fl_Window* _win;
        Fl_Output* _descriptionField;
        Fl_Input* _valueField;
        Fl_Button* _okButton;
        Fl_Button* _cancelButton;

    public:

        GetValueDialog(std::string title, std::string text, std::string description, std::string defaultValue = "");
        ~GetValueDialog();

        static GetValueDialog* GetInstance() {
            return _instance;
        }

        bool Show();

        void Ok();
        void Cancel();

        std::string GetValue() {
            return _value;
        }
};

#endif