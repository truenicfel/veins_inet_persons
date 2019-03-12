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

#include <veins/modules/mobility/traci/subscriptionManagement/PersonSubscriptionManager.h>
#include <algorithm>

#include "veins/modules/mobility/traci/TraCIConstants.h"

namespace Veins {
namespace TraCISubscriptionManagement {

PersonSubscriptionManager::PersonSubscriptionManager()
    : SubscriptionManagerBase()
    , mUpdatedPersons()
    , mDisappearedPersons()
{
}

bool PersonSubscriptionManager::updateWithList(std::list<std::string>& currentlyActivePersonIds) {
    processPersonIDList(currentlyActivePersonIds);
    return true;
}

bool PersonSubscriptionManager::update(TraCIBuffer& buffer) {

    bool idListUpdateReceived = false;

    // this is the object id that this subscription result contains
    // content about. for example a person id.
    std::string responseObjectID;
    buffer >> responseObjectID;

    // the number of response variables that are contained in this buffer
    uint8_t numberOfResponseVariables;
    buffer >> numberOfResponseVariables;
    // this should either be one or five
    ASSERT(numberOfResponseVariables == 1 || numberOfResponseVariables == 5);

    // these need to be filled (total of 5 variables --> x, y count as one)
    double x;
    double y;
    double speed;
    std::string edge;
    double angle;
    std::string typeID;

    for (int counter = 0; counter < numberOfResponseVariables; ++counter) {

        // extract a couple of values:
        // - identifies the variable (position, list etc.)
        uint8_t responseVariableID;
        buffer >> responseVariableID;
        // - status of the variable
        uint8_t variableStatus;
        buffer >> variableStatus;
        // - type of the variable
        uint8_t variableType;
        buffer >> variableType;

        if (variableStatus == TraCIConstants::RTYPE_OK) {
            // the status of the variable is ok

            // now check which variable id we got
            // it is either an id_list or a subscription for a specific vehicle
            // never both
            if (responseVariableID == TraCIConstants::ID_LIST) {
                ASSERT(variableType == TraCIConstants::TYPE_STRINGLIST);
                idListUpdateReceived = true;

                uint32_t numberOfActivePersons;
                buffer >> numberOfActivePersons;
                EV_DEBUG << "TraCI reports " << numberOfActivePersons << " active persons."
                                << endl;

                // add all id strings of reported active persons to a set
                std::list<std::string> traciActivePersons;
                for (uint32_t counter = 0; counter < numberOfActivePersons; ++counter) {
                    std::string idstring;
                    buffer >> idstring;
                    traciActivePersons.push_back(idstring);
                }

                // helper method takes care of this
                processPersonIDList(traciActivePersons);

            } else {
                // subscription for specific person
                if (responseVariableID == TraCIConstants::VAR_POSITION) {
                    ASSERT(variableType == TraCIConstants::POSITION_2D);
                    buffer >> x;
                    buffer >> y;
                } else if (responseVariableID == TraCIConstants::VAR_ROAD_ID) {
                    ASSERT(variableType == TraCIConstants::TYPE_STRING);
                    buffer >> edge;
                } else if (responseVariableID == TraCIConstants::VAR_SPEED) {
                    ASSERT(variableType == TraCIConstants::TYPE_DOUBLE);
                    buffer >> speed;
                } else if (responseVariableID == TraCIConstants::VAR_ANGLE) {
                    ASSERT(variableType == TraCIConstants::TYPE_DOUBLE);
                    buffer >> angle;
                } else if (responseVariableID == TraCIConstants::VAR_TYPE) {
                    ASSERT(variableType == TraCIConstants::TYPE_STRING);
                    buffer >> typeID;
                }else {
                    throw cRuntimeError("Received unknown person subscription result");
                }
            }

        } else {
            // the status of the variable is not ok
            ASSERT(variableType == TraCIConstants::TYPE_STRING);
            std::string errormsg;
            buffer >> errormsg;
            if (isSubscribed(responseObjectID)) {
                if (variableStatus == TraCIConstants::RTYPE_NOTIMPLEMENTED) {
                    throw cRuntimeError(
                            "TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.",
                            responseVariableID, errormsg.c_str());
                }

                throw cRuntimeError(
                        "TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").",
                        responseVariableID, errormsg.c_str());
            }
        }

    }

    // make sure we are only entering this section if we got a person that we already subscribed to
    if (isSubscribed(responseObjectID)) {
        // we want to deliver an update for this person for the next call to getUpdated()
        TraCIPerson person(x, y, edge, speed, angle, responseObjectID, typeID);
        mUpdatedPersons.push_back(person);
    }

    return idListUpdateReceived;
}

std::list<TraCIPerson> PersonSubscriptionManager::getUpdated() {
    std::list<TraCIPerson> temp = mUpdatedPersons;
    mUpdatedPersons.clear();
    return temp;
}

std::set<std::string> PersonSubscriptionManager::getDisappeared() {
    std::set<std::string> temp = mDisappearedPersons;
    mDisappearedPersons.clear();
    return temp;
}

void PersonSubscriptionManager::initialize(
        std::shared_ptr<TraCIConnection> connection,
        std::shared_ptr<TraCICommandInterface> commandInterface) {

    // call base class
    SubscriptionManagerBase::initialize(connection, commandInterface);

    // subscribe to list of person ids
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = "";
    uint8_t variableNumber = 1;
    uint8_t variable1 = TraCIConstants::ID_LIST;
    TraCIBuffer buf = getConnection()->query(TraCIConstants::CMD_SUBSCRIBE_PERSON_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buf >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buf >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buf >> responseCommandID;
    ASSERT(responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_PERSON_VARIABLE);

    update(buf);
    ASSERT(buf.eof());

}

void PersonSubscriptionManager::processPersonIDList(std::list<std::string>& idList) {

    std::set<std::string> subscribed = getSubscribed();

    // check for persons that need subscribing to
    std::set<std::string> needSubscribe;
    // basically: idList - getSubscribed()
    // --> result is a list of persons that need to be subscribed
    std::set_difference(idList.begin(), idList.end(),
            subscribed.begin(), subscribed.end(),
            std::inserter(needSubscribe, needSubscribe.begin()));
    for (auto id : needSubscribe) {
        addToSubscribed(id);
        // the person will be automatically added to update
        // after executing the following method
        subscribeToPersonVariables(id);
    }

    // check for persons that disappeared
    // basically: getSubscribed - idList
    // --> result is a list of persons that need to be unsubscribed
    std::set_difference(subscribed.begin(), subscribed.end(),
            idList.begin(), idList.end(),
            std::inserter(mDisappearedPersons, mDisappearedPersons.begin()));
    for (auto id : mDisappearedPersons) {
        if (isSubscribed(id)) {
            removeFromSubscribed(id);
        }
        // there is no need to unsubscribe at TraCI: if a person disappears
        // the subscription will not be updated anyways
    }

}

void PersonSubscriptionManager::subscribeToPersonVariables(std::string id) {
    // subscribe to some attributes of the person
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = id;
    uint8_t variableNumber = 5;
    uint8_t variable1 = TraCIConstants::VAR_POSITION;
    uint8_t variable2 = TraCIConstants::VAR_ROAD_ID;
    uint8_t variable3 = TraCIConstants::VAR_SPEED;
    uint8_t variable4 = TraCIConstants::VAR_ANGLE;
    uint8_t variable5 = TraCIConstants::VAR_TYPE;

    TraCIBuffer buffer = getConnection()->query(TraCIConstants::CMD_SUBSCRIBE_PERSON_VARIABLE,
            TraCIBuffer() << beginTime << endTime << objectId << variableNumber
                    << variable1 << variable2 << variable3 << variable4 << variable5);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buffer >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buffer >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buffer >> responseCommandID;
    ASSERT(responseCommandID == TraCIConstants::RESPONSE_SUBSCRIBE_PERSON_VARIABLE);

    update(buffer);
    ASSERT(buffer.eof());
}

} // end namespace TraCISubscriptionManagement
} // end namespace Veins

