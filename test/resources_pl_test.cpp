#include <cstring>

#include "roo_windows_transceivers/model/switches/resources.h"
#include "roo_windows_transceivers/model/thermometers/resources.h"

// Verifies that selecting Polish includes both Polish model resource sets.
int main() {
  using namespace roo_windows_transceivers;
  if (std::strcmp(kStrSwitches, "Przełączniki") != 0) return 1;
  if (std::strcmp(kStrAssignSwitch, "Przypisz") != 0) return 1;
  if (std::strcmp(kStrThermometers, "Termometry") != 0) return 1;
  if (std::strcmp(kStrThermometerDetails, "Szczegóły termometru") != 0)
    return 1;
  return 0;
}
