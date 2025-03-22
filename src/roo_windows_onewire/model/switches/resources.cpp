#include "roo_windows_onewire/model/thermometers/resources.h"

#include "roo_windows/config.h"

namespace roo_windows_onewire {

#if (ROO_WINDOWS_LANG == ROO_WINDOWS_LANG_pl)

const char* kStrSwitches = "Przełączniki";
const char* kStrSwitchDetails = "Szczegóły przełącznika";

const char* kStrAssignSwitch = "Przypisz";
const char* kStrUnassignSwitch = "Rozłącz";

const char* kStrUnassignSwitchQuestion = "Usunąć przypisanie?";

const char* kStrUnassignSwitchSupportingText =
    "Przełącznik nie będzie już\n"
    "aktywny, i będzie wymagał\n"
    "ponownego przypisania.";

const char* kStrSwitchNotAssigned = "(nieprzypisany)";
const char* kStrSelectSwitch = "Przypisz przełącznik";

#else

const char* kStrSwitches = "Switches";
const char* kStrSwitchDetails = "Switch details";

const char* kStrAssignSwitch = "Assign";
const char* kStrUnassignSwitch = "Unassign";

const char* kStrSwitchNotAssigned = "(not assigned)";
const char* kStrSelectSwitch = "Assign switch";

const char* kStrUnassignSwitchQuestion = "Remove assignment?";

const char* kStrUnassignSwitchSupportingText =
    "The switch will be no longer\n"
    "actie, and it will require \n"
    "re-assignment";

#endif

}  // namespace roo_windows_onewire
