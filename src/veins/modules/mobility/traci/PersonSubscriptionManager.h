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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_PERSONSUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_PERSONSUBSCRIPTIONMANAGER_H_

#include <set>
#include <string>
#include <list>

class PersonSubscriptionManager {
public:
    /**
     * Constructor.
     */
    PersonSubscriptionManager();

    /**
     * Default destructor.
     */
    virtual ~PersonSubscriptionManager() = default;

    /**
     * Update this PersonSubscriptionManager with the given
     * active person id list.
     *
     * Note: This is a list here because that is what is offered
     * by the traci command interface as return value.
     *
     * @param currentlyActivePersonIds A list containing the currently active persons
     * identified by their ids.
     */
    void update(std::list<std::string> currentlyActivePersonIds);

    /**
     * This gives you all the new persons that appeared when you
     * called update() the last time.
     *
     * @return std::set<std::string> a set of person ids.
     */
    std::set<std::string> getNewPersons();

    /**
     * This gives you all the disappeared persons when you
     * called update() the last time.
     *
     * @return std::set<std::string> a set of person ids.
     */
    std::set<std::string> getDisappearedPersons();

private:

    /**
     * This stores all the active person ids. No duplicates.
     */
    std::set<std::string> mActivePersons;

    /**
     * This is the difference between currentlyActivePersonIds
     * (provided with call to update()) and mActivePersons.
     *
     * This takes all the elements which are contained in
     * currentlyActivePersons but not in mActivePersons.
     */
    std::set<std::string> mNewPersons;

    /**
     * This is the difference between mActivPersons and
     * currentlyActivePersonIds (provided with call to update()).
     *
     * This takes all the elements which are contained in
     * mActivePersons but not in currentlyActivePersonIds.
     */
    std::set<std::string> mDisappearedPersons;

    /**
     * Get the active person id set.
     *
     * @return a set of string containing the active person ids.
     */
    std::set<std::string> getActivePersonIds();

    /**
     * Add active person.
     *
     * Note: Duplicates will be accepted but have no impact on the set.
     *
     * @param id of the new person.
     */
    void addActivePerson(std::string id);
};

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_PERSONSUBSCRIPTIONMANAGER_H_ */
