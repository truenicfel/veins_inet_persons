//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include <algorithm>

#include "veins/modules/mobility/traci/PersonSubscriptionManager.h"

PersonSubscriptionManager::PersonSubscriptionManager()
    : mActivePersons()
    , mNewPersons()
    , mDisappearedPersons()
{
}

void PersonSubscriptionManager::update(std::list<std::string> currentlyActivePersonIds) {

    // convert list to set
    std::set<std::string> currentlyActivePersons;
    for (auto personID: currentlyActivePersonIds) {
        currentlyActivePersons.insert(personID);
    }

    // check for new persons
    std::set<std::string> newPersons;
    // basically: currentlyActivePersons - mActivePersons
    // --> result is a list of persons that are new
    std::set_difference(currentlyActivePersons.begin(), currentlyActivePersons.end(),
            mActivePersons.begin(), mActivePersons.end(),
            std::inserter(newPersons, newPersons.begin()));
    // replace
    mNewPersons = newPersons;


    // check for disappeared persons
    std::set<std::string> disappearedPersons;
    // basically: mActivePersons - currentlyActivePersons
    // --> result is a list of persons that disappeared
    std::set_difference(mActivePersons.begin(), mActivePersons.end(),
            currentlyActivePersons.begin(), currentlyActivePersons.end(),
            std::inserter(disappearedPersons, disappearedPersons.begin()));
    // replace
    mDisappearedPersons = disappearedPersons;

    // replace
    mActivePersons = currentlyActivePersons;
}

std::set<std::string> PersonSubscriptionManager::getNewPersons() {
    return mNewPersons;
}

std::set<std::string> PersonSubscriptionManager::getDisappearedPersons() {
    return mDisappearedPersons;
}

std::set<std::string> PersonSubscriptionManager::getActivePersonIds() {
    return mActivePersons;
}

void PersonSubscriptionManager::addActivePerson(std::string id) {
    mActivePersons.insert(id);
}

