#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSuggestionPopup.h>

SAMPLE_BEGIN(AutoComplete)

auto container = std::make_unique<Wt::WContainerWidget>();

// Set options for email address suggestions:
Wt::WSuggestionPopup::Options contactOptions;
contactOptions.highlightBeginTag = "<span class=\"highlight\">";
contactOptions.highlightEndTag = "</span>";
contactOptions.listSeparator = ',';
contactOptions.whitespace = " \n";
contactOptions.wordSeparators = "-., \"@\n;";
contactOptions.appendReplacedText = ", ";

#ifndef WT_TARGET_JAVA
Wt::WSuggestionPopup *sp =
    container->addChild(
      std::make_unique<Wt::WSuggestionPopup>(
            Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
            Wt::WSuggestionPopup::generateReplacerJS(contactOptions)));
#else
Wt::WSuggestionPopup *sp =
    new Wt::WSuggestionPopup(
            Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
            Wt::WSuggestionPopup::generateReplacerJS(contactOptions));
#endif

Wt::WLineEdit *le = container->addNew<Wt::WLineEdit>();
le->setPlaceholderText("Enter a name starting with 'J'");
sp->forEdit(le);

// Populate the underlying model with suggestions:
sp->addSuggestion("John Tech <techie@mycompany.com>");
sp->addSuggestion("Johnny Cash <cash@mycompany.com>");
sp->addSuggestion("John Rambo <rambo@mycompany.com>");
sp->addSuggestion("Johanna Tree <johanna@mycompany.com>");

SAMPLE_END(return std::move(container))
