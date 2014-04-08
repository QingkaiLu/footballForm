#ifndef EXTRACT_OD_VIDS_FEATURES_H_
#define EXTRACT_OD_VIDS_FEATURES_H_
/*
 * Extract feature vectors for offense and defense on the left/right of LOS
 */

#include "commonStructs.h"

void readOdLabels(std::string odLabelFilePath, std::vector<playId> &pIds, std::vector<char> &dir, std::vector<char> &odLabel);

void extracOdVidFeatsRts(int gameId);

void extracOdVidFeatSvm();

#endif
